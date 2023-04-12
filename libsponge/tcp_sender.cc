#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {
        

        RTO=retx_timeout;
    }

uint64_t TCPSender::bytes_in_flight() const {

     //TCPSegment seg1= outstandings.back();
     //TCPSegment seg2= outstandings.front();
    
     //size_t s=seg1.length_in_sequence_space();
     
     
     return in_flights;

 }

void TCPSender::fill_window() {


    while(next_seqno_absolute()<=window_right){


        
        TCPSegment seg{};

        TCPHeader& hd=seg.header();
        hd.seqno=next_seqno();
        
        int syn=0;
        //int fin=0;
        if(next_seqno_absolute()==0){

            hd.syn=true;
            syn=1;//syn占一位
        }
        //Buffer load=seg.payload();不能这么用因为load此时相当于一个新的copy后的对象
        //Buffer& payload_data = seg.payload();这样表达是可以的

        //要减去syn,避免因为syn占位使segment超过了对面window的限制
        std::string data=stream_in().read(window_right-syn-next_seqno_absolute()+1);
        seg.payload()=std::string(data);//seg.payload()返回payload的reference。
                                        //其实就是seg种的payload



        _next_seqno=seg.length_in_sequence_space()+_next_seqno;

        if(stream_in().eof()&&_next_seqno<=window_right){

            hd.fin=true;
            _next_seqno++;
        }
        
        //seg.length_in_sequence_space=data.size();

        //_next_seqno=_next_seqno+data.size()+syn+fin;
        
        //if(hd.syn)_next_seqno++;
        //if(hd.fin)_next_seqno++;
        
        in_flights=in_flights+seg.length_in_sequence_space();
        outstandings.push(seg);

        segments_out().push(seg);
    }

}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    
    
    DUMMY_CODE(ackno, window_size); 
    
    
    //ackno可能只acknowledge某个segment的一部分,但本lab不考虑这种。
    //如果只ack某个segment的一部分,该segment视为尚未被ack,不将其截断
    
    uint64_t ackabs= unwrap(ackno,_isn,stream_in().bytes_read());
    
    window=window_size;
    if(ackabs+window_size-1>window_right)window_right=ackabs+window_size-1;
    
    //if(outstandings.size()==0||)
    
    //是否有任何的outstanding segment被ack掉
    bool rm=false;
    while(outstandings.size()!=0){
        
        TCPSegment seg=outstandings.front();

        uint64_t last=seg.length_in_sequence_space()+unwrap(seg.header().seqno,_isn,stream_in().bytes_read())-1;

        if(last<ackabs){

            rm=true;
            //这个地方可能有坑,因为ack可能只acknowledge了一部分
            //要保持和outstanding segments的一致性
            in_flights=in_flights-outstandings.front().length_in_sequence_space();
            outstandings.pop();
        }else break;

    }

    if(rm){

        RTO=_initial_retransmission_timeout;
        if(outstandings.size()>0)timePassed=0;
        recount=0;
    }

}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    
    
    DUMMY_CODE(ms_since_last_tick);

    if(outstandings.size()==0)return;
    
    if(ms_since_last_tick+timePassed>=RTO){

        segments_out().push(outstandings.front());
    }

    if(window>0){

        recount++;
        RTO=RTO*2;
    }

    timePassed=0;
    
 }

unsigned int TCPSender::consecutive_retransmissions() const { 
    

    return recount;
 }

void TCPSender::send_empty_segment() {


   
    TCPSegment seg{};


   TCPHeader& hd=seg.header();

   hd.seqno=next_seqno();

   segments_out().push(seg);


}
