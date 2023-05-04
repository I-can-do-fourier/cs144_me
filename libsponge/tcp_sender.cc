#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

#include "tcp_state.hh"

#include <iostream>

#include <sstream>

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

    //cout<<next_seqno_absolute()<<" "<<window_right<<endl;


    

    unsigned long tcpMax=TCPConfig::MAX_PAYLOAD_SIZE;

    bool ReadyToFin=stream_in().eof()&&next_seqno_absolute() < stream_in().bytes_written() + 2;

    /**
     * 
     * sender因为window的size不能发出时,有两种情况
     * 1. window==0
     * 2. window!=0 _next_seqno=window_right+1
     * 
     * 只有当window==0时才会probing。第二种情况不会触发probing。第二种情况情况相当于sender
       发送了一些segment，直到填满window。但这些segment并没有全部被ack。此时不能发送额外的probing
     * 
     * 第一种情况要不第二种情况更极端。
    */
    if(window==0&&!probing){

        if(!stream_in().buffer_empty()){

            probing=true;
            send_one_byte_segment(false);
            return;
        }else if(ReadyToFin){


            probing=true;
            send_one_byte_segment(true);
            return;
        }

    }


    
    // //暂时不能考虑发送空包的情况
    // if(window==0&&stream_in().buffer_empty()){

    //     send_empty_segment();
    //     return;
    // }

    //if(stream_in().buffer_empty())return;
    //cout<<next_seqno_absolute()<<endl;
    while(next_seqno_absolute()<=window_right){

        //if(stream_in().eof()&&next_seqno_absolute() == stream_in().bytes_written() + 2&&in_flights>0)break;
        // if(stream_in().buffer_empty()){


        //     if(next_seqno_absolute()>0)break;

        // }

        //
        ReadyToFin=stream_in().eof()&&next_seqno_absolute() < stream_in().bytes_written() + 2;

        
        // if(window==0){

        //     if(!probing){
                
        //         if(ReadyToFin){

        //             probing=true;
        //             send_one_byte_segment(true);
        //             return;
        //         }else if(!stream_in().buffer_empty()){

        //             probing=true;
        //             send_one_byte_segment(false);
        //             return;

        //         }

                
        //     }else return;o

        //     if(probing||(!ReadyToFin))return;

        // }

        //根据测试用例，似乎fin_ack之后就不能进来了
        //bool Fin_Acked=stream_in().eof()&&next_seqno_absolute() == stream_in().bytes_written() + 2&&in_flights == 0;

        //只有当处于CLOSED、ReadyToFin（就差一个fin）时，才允许payload为空
        if(next_seqno_absolute()>0&&stream_in().buffer_empty()&&!(ReadyToFin))break;
        TCPSegment seg{};

        //TCPHeader  &hd=seg.header();
        seg.header().seqno=next_seqno();
        
        int syn=0;
        //int fin=0;


        if(next_seqno_absolute()==0){

            //cout<<"syn"<<endl;
            seg.header().syn=1;
            syn=1;//syn占一位
        }
        //Buffer load=seg.payload();不能这么用因为load此时相当于一个新的copy后的对象
        //Buffer& payload_data = seg.payload();这样表达是可以的

        //要减去syn,避免因为syn占位使segment超过了对面window的限制


        size_t dlen=min(window_right-syn-next_seqno_absolute()+1,tcpMax);
        std::string data=stream_in().read(dlen);
        seg.payload()=std::string(data);//seg.payload()返回payload的reference。
                                        //其实就是seg种的payload



        _next_seqno=seg.length_in_sequence_space()+_next_seqno;

        //!fin_sent这个判断应该是可以不要了
        if(stream_in().eof()&&_next_seqno<=window_right&&!fin_sent){

            seg.header().fin=1;
            _next_seqno++;
            fin_sent=true;
        }
        
        //seg.length_in_sequence_space=data.size();

        //_next_seqno=_next_seqno+data.size()+syn+fin;
        
        //if(hd.syn)_next_seqno++;
        //if(hd.fin)_next_seqno++;
        
        in_flights=in_flights+seg.length_in_sequence_space();
        outstandings.push(seg);
        
        
        //cout<<seg.header().summary()<<endl;
        segments_out().push(seg);

        //std::ostringstream ss;
        //cout<<next_seqno_absolute()<<" "<<in_flights<<" "<<seg.length_in_sequence_space()<<endl;

        //if(stream_in().buffer_empty()&&!(stream_in().eof()&&next_seqno_absolute() < stream_in().bytes_written() + 2))break;

        //if(Fin_Acked)break;

    }

    //TCPState state{};

    //cout<<segments_out().front().length_in_sequence_space()<<endl;
    //cout<<state.state_summary(*this)<<endl;
    

}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    
    
    DUMMY_CODE(ackno, window_size); 
    
    
    //TCPState state{};

    //cout<<segments_out().front().length_in_sequence_space()<<endl;

    //ackno可能只acknowledge某个segment的一部分,但本lab不考虑这种。
    //如果只ack某个segment的一部分,该segment视为尚未被ack,不将其截断
    
    uint64_t ackabs= unwrap(ackno,_isn,stream_in().bytes_read());

    if(ackabs>next_seqno_absolute())return;
    
    //cout<<"ack:"<<ackabs<<" "<<window_size <<endl;
    window=window_size;
    //if(window>0)probing=false;
    bool window_move=false;
    if(ackabs+window_size-1>window_right){

        window_move=true;
        window_right=ackabs+window_size-1;
    }
    
    //if(outstandings.size()==0||)
    
    //是否有任何的outstanding segment被ack掉
    bool rm=false;
    while(outstandings.size()!=0){
        
        TCPSegment& seg=outstandings.front();

        uint64_t last=seg.length_in_sequence_space()+unwrap(seg.header().seqno,_isn,stream_in().bytes_read())-1;
        //cout<<"last"<<last<<endl;
        if(last<ackabs){

            rm=true;
            //这个地方可能有坑,因为ack可能只acknowledge了一部分
            //要保持和outstanding segments的一致性
            in_flights=in_flights-outstandings.front().length_in_sequence_space();
            outstandings.pop();
        }else break;

    }

    //只要window 的右侧移动了,必定说明probing完成(当然probing也许本来就没开始,不影响)
    if(window_move)probing=false;

    if(rm){

        //probing=false;//只要被ack了任何一个seg,就说明probing结束了
        RTO=_initial_retransmission_timeout;
        if(outstandings.size()>0)timePassed=0;
        recount=0;
    }

    //有空位就补上
    //test中会自动调用fill_window().所以此处不能调
    //fill_window();

}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    
    //将tick和retransmission合在一起。没有单独设置RTO timer
    
    DUMMY_CODE(ms_since_last_tick);

    //只有当目前有outstandings有segment，或者说有一些segment没有被ack时才会启动定时器。
    if(outstandings.size()>0) timePassed=ms_since_last_tick+timePassed;

    if(outstandings.size()==0)return;
    


    if(timePassed>=RTO){

        segments_out().push(outstandings.front());

        if(window>0){

            recount++;
            RTO=RTO*2;
        }

        timePassed=0;    
    }


    
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

void TCPSender::send_empty_segment_RST() {


   
    TCPSegment seg{};


   TCPHeader& hd=seg.header();

   hd.seqno=next_seqno();
   
   hd.rst=true;

   segments_out().push(seg);


}


/*
    支持发送单个finsegment
*/
void TCPSender::send_one_byte_segment(bool f){

   TCPSegment seg{};

   
   TCPHeader& hd=seg.header();

   hd.seqno=next_seqno();

   if(f)hd.fin=true;
   else seg.payload()=std::string(stream_in().read(1));
   
   
   
    _next_seqno++;
   in_flights++;
   outstandings.push(seg);


   segments_out().push(seg);

}
