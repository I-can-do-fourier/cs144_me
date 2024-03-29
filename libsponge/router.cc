#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    DUMMY_CODE(route_prefix, prefix_length, next_hop, interface_num);
    // Your code here.

    len_map[route_prefix]=prefix_length;
    itf_map[route_prefix]=interface_num;

    if(next_hop.has_value())gw_map[route_prefix]=(*next_hop).ipv4_numeric();
    
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    DUMMY_CODE(dgram);
    // Your code here.

    uint8_t m_length=0;
    size_t itf=0;
    uint32_t next_hop=2873314305;//没有匹配到时，发到default_router

    auto it=len_map.begin();

    while(it!=len_map.end()){

        uint32_t prefix=(it->first)>>(32-it->second);
        uint32_t addr=dgram.header().dst;
        uint32_t addr_mask=addr>>(32-it->second);

        if(addr_mask==prefix&&it->second>m_length){

            m_length=it->second;
            itf=itf_map[it->first];
            
            if(gw_map.find(it->first)==gw_map.end())next_hop=addr;
            else next_hop=gw_map[it->first];
        }
        
        it++;
    }

    //if(m_length==0)next_hop=;

    if(dgram.header().ttl-1<=0)return;

    dgram.header().ttl=dgram.header().ttl-1;

    interface(itf).send_datagram(dgram,Address::from_ipv4_numeric(next_hop));
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
