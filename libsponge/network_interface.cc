#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();


    EthernetFrame frame{};
    if(cache.find(next_hop_ip)!=cache.end()){

        //缓存中有next_hop_ip与其mac地址的映射关系

        frame.payload()=dgram.serialize();

        //EthernetHeader最关键的事local和next_hop的mac地址
        EthernetHeader& hd=frame.header();
        hd.src=_ethernet_address;
        hd.dst=cache.find(next_hop_ip)->second;
        hd.type=hd.TYPE_IPv4;


        frames_out().push(frame);

    }else if(since_last_sent.find(next_hop_ip)==since_last_sent.end()||since_last_sent.find(next_hop_ip)->second>=5000){
        
        //缓存中没有映射信息，要发送arp广播

        ARPMessage arp{};
        EthernetHeader& hd=frame.header();
        
        hd.src=_ethernet_address;
        hd.dst=ETHERNET_BROADCAST;//广播
        hd.type=hd.TYPE_ARP;
        
        
        since_last_sent[next_hop_ip]=0;

        arp.opcode=arp.OPCODE_REQUEST;
        
        arp.sender_ip_address=_ip_address.ipv4_numeric();
        arp.sender_ethernet_address=_ethernet_address;
        arp.target_ip_address=next_hop_ip;

        frame.payload()=arp.serialize();

        frames_out().push(frame);

        if(datagrams.find(next_hop_ip)==datagrams.end()){

            std::list<InternetDatagram> ls{};
            datagrams[next_hop_ip]=ls;
 
            
        }
            //将待发送datagram放到相应队列中。等ip-mac 映射准备好后，再统一发送
            datagrams.find(next_hop_ip)->second.push_front(dgram);
        
    }
    DUMMY_CODE(dgram, next_hop, next_hop_ip);
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    DUMMY_CODE(frame);

    const EthernetHeader& hd=frame.header();

    if(!(hd.dst==ETHERNET_BROADCAST||hd.dst==_ethernet_address))return {};
    if(hd.type==hd.TYPE_IPv4){
        //返回收到的正常IPv4 datagram
        IPv4Datagram dgram{};
        ParseResult pr=dgram.parse(frame.payload());

        if(pr==ParseResult::NoError)return dgram;

    }else{
        //收到的是arp（可能是request/reply）
        ARPMessage am{};
        ParseResult pr=am.parse(frame.payload());

        if(pr!=ParseResult::NoError)return{};
        
        //不论是何种arp，先记录下sender的映射关系。
        cache[am.sender_ip_address]=am.sender_ethernet_address;
        cache_time[am.sender_ip_address]=0;

        //看看datagram等待队列中有没有需要这个映射关系的。如果有，就立刻将它们发送出去。
        if(datagrams.find(am.sender_ip_address)!=datagrams.end()){

            list<InternetDatagram> wait=datagrams.find(am.sender_ip_address)->second;
            
            while(wait.size()>0){

                InternetDatagram id=wait.front();
                wait.pop_front();
                EthernetFrame ff{};

                ff.header().dst=am.sender_ethernet_address;
                ff.header().src=_ethernet_address;
                ff.header().type=ff.header().TYPE_IPv4;

                ff.payload()=id.serialize();

                frames_out().push(ff);

            }

           datagrams.erase(datagrams.find(am.sender_ip_address));
        }
        //如果arp是一个request，而且请求的是自己ip-mac映射，就回复。
        if(am.opcode==am.OPCODE_REQUEST&&am.target_ip_address==_ip_address.ipv4_numeric()){

            ARPMessage reply{};

            reply.opcode=reply.OPCODE_REPLY;

            reply.sender_ip_address=_ip_address.ipv4_numeric();
            reply.sender_ethernet_address=_ethernet_address;

            reply.target_ip_address=am.sender_ip_address;
            reply.target_ethernet_address=am.sender_ethernet_address;

            EthernetFrame f{};

            EthernetHeader& fhd=f.header();

            //arp用在内网(相对)，要么直连，要么通过交换机。所以这里直接将ethernet的dst设置为arp发送方的mac地址
            //对于arp发送方，arp中的src mac地址应该和frame中的src是相同的。
            fhd.dst=am.sender_ethernet_address;
            fhd.src=_ethernet_address;
            fhd.type=fhd.TYPE_ARP;

            f.payload()=reply.serialize();

            frames_out().push(f);

        }
    }
    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) { 
    
    //清理缓存
    
    DUMMY_CODE(ms_since_last_tick); 
    
    
    auto it=cache_time.begin();

    while(it!=cache_time.end()){

        uint32_t ip=it->first;
        size_t t=it->second;

        t=t+ms_since_last_tick;

        if(t>=30000){

            cache.erase(cache.find(ip));
            it=cache_time.erase(it);
            
        }else{
            
            it->second=t;
            it++;
        } 
    }

    it=since_last_sent.begin();

    while(it!=since_last_sent.end()){

        
        //uint32_t ip=it->first;
        size_t t=it->second;

        t=t+ms_since_last_tick;

        if(t>=5000){

            it=since_last_sent.erase(it);
            
            
        }else{
            
            it->second=t;
            it++;
        } 
    }


}
