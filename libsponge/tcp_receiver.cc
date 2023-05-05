#include "tcp_receiver.hh"
#include <iostream>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    DUMMY_CODE(seg);


    Buffer bf{};
    NetParser np{bf};

    TCPSegment s=seg;
    //ParseResult pr=s.header().parse(np);

    //as_string(pr);
    
    TCPHeader h=s.header();
    

    /**
     * 
     * 这一步有待讨论。涉及到tcp线程是如何监听请求的。
     * 1. 在三次握手开始时，tcp是不是就生成了相应的内核空间，且保存了对方的信息(ip,port)。这样就能被找到
     * 2. 三次握手开始时每个tcp线程是怎么和对方客户绑定起来的
     * 3. syn标志位的判断是否是在tcp线程中也就是这里判断。创建线程之前就判断了。要不然对方恶意发一些不带syn的包怎么办。
     *     此时各种数据结构和空间都已经分配了
    */
    if(!syned&&!h.syn)return;
    
    if(h.syn){
        
        //cout<<"flag:"<<h.syn<<endl;
        isn=h.seqno;
        syned=true;
        //不能return，因为可能出现syn和fin同时set的情况,也可能有data
        //return;
    }

    bool f=h.fin;

    //std::string pl=s.payload().copy();

    uint64_t idx=0;
    if(h.syn)idx=0;//这是避免syn被set了，同时segment中还有payload的情况。此时对于stream来说，index位0
    else if(unwrap(h.seqno,isn,stream_out().bytes_written())==0)return;//侵占了syn的位置，直接返回。
    else idx=unwrap(h.seqno,isn,stream_out().bytes_written())-1;

    //if(idx==0)
    _reassembler.push_substring(s.payload().copy(),idx,f);

    //_reassembler.push_substring()
}

//void 

optional<WrappingInt32> TCPReceiver::ackno() const { 
    

    if(!syned) return {};

    //要将stream的index换成absolute index
    //syn和fin都会在absoluteindex中占用一个位置

    uint64_t ack=_reassembler.stream_out().bytes_written()+1;

    if(_reassembler.stream_out().input_ended())ack=ack+1;
    return WrappingInt32{wrap(ack,isn)};
}

size_t TCPReceiver::window_size() const { 


    return _reassembler.stream_out().remaining_capacity();

}
