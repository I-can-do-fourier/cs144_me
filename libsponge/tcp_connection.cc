#include "tcp_connection.hh"

#include <iostream>

#include "tcp_state.hh"



// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { 
    

    return _sender.stream_in().remaining_capacity();
}

size_t TCPConnection::bytes_in_flight() const { 
    

    return _sender.bytes_in_flight();
 }

size_t TCPConnection::unassembled_bytes() const { 
    
    return _receiver.unassembled_bytes();
}

size_t TCPConnection::time_since_last_segment_received() const { 
    
    return _time_since_last_segment_received;
}

void TCPConnection::segment_received(const TCPSegment &seg) { 
    
    
    DUMMY_CODE(seg); 
    
    const TCPHeader& hd=seg.header();

    _time_since_last_segment_received=0;

    //std::string state=TCPState::state_summary(_sender);
    //当local处于listen状态下(未发送syn)，收到ack是不合理的
    if(_sender.next_seqno_absolute() == 0&&seg.header().ack)return;


    if(hd.rst){
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _active=false;
        _linger_after_streams_finish=false;

        return;
    }
    /**
     * 
     * 具体参考pdf。
     * 当local还没有发出fin时，首先收到了remote的fin。
     * 那么之后local可以不进行time_wait。
    */
    if(hd.fin&&(!_sender.stream_in().eof()||_sender.next_seqno_absolute()<_sender.stream_in().bytes_written() + 2)){

        _linger_after_streams_finish=false;
    }
    
    //不能使用下面这句代码。会导致重叠包不能正常接收。
    //if(!(_receiver.ackno().has_value()&&seg.header().seqno == _receiver.ackno().value() - 1))
    _receiver.segment_received(seg);
    if(hd.ack){
        //调整对方window,
        //以及根据ackno来确认那些被ack的包，将他们剔除outstanding队列
        _sender.ack_received(hd.ackno,hd.win);

        //调整对方window后,立刻将能发送的data都装上
        _sender.fill_window();
    }
    //当local率先收到对面的syn时，要及时回应，发出自己的(syn,ack)包
    if(_sender.next_seqno_absolute()==0&&_receiver.ackno().has_value()){

        _sender.fill_window();
    }
    //如果收到的seg包含任何的data(syn,fin,payload)，都需要回应ack。对面需要这个ack
    if(seg.length_in_sequence_space()>0){

        //seg_2_seg();    
        //_sender.fill_window();
        if(_sender.segments_out().empty())_sender.send_empty_segment();
        //seg_2_seg();
    }
     //详见pdf。对面发出了一个keep-alive包，local需要回应。
     //根据tcp Rfc, keep-alive包的sequence number为_receiver.ackno().value() - 1，即
     //receiver需要的ack-1
     if (_receiver.ackno().has_value()&&seg.length_in_sequence_space() == 0&&
         seg.header().seqno == _receiver.ackno().value() - 1) {
         if(_sender.segments_out().empty())_sender.send_empty_segment();
         //seg_2_seg();
    }  

    seg_2_seg();

}

bool TCPConnection::active() const { 
    

    return _active;
 }

size_t TCPConnection::write(const string &data) {
    DUMMY_CODE(data);

    size_t res= _sender.stream_in().write(data);

    //每次向stream写入数据后,要尽可能地向remote peer发送数据
    _sender.fill_window();
    seg_2_seg();

    return res;

}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
/**
 * 
 * I̶n̶ t̶h̶e̶o̶r̶y̶,̶ i̶t̶ i̶s̶ n̶e̶c̶e̶s̶s̶a̶r̶y̶ t̶o̶ s̶e̶n̶d̶ s̶o̶m̶e̶ p̶r̶o̶b̶e̶ p̶a̶c̶k̶e̶t̶s̶ a̶p̶p̶r̶o̶p̶r̶i̶a̶t̶e̶l̶y̶ e̶a̶c̶h̶ t̶i̶m̶e̶ a̶ t̶i̶c̶k̶ o̶c̶c̶u̶r̶s̶.̶
 *
*/
void TCPConnection::tick(const size_t ms_since_last_tick) { 
    
    DUMMY_CODE(ms_since_last_tick); 
    

    _sender.tick(ms_since_last_tick);//告诉sender时间
    _time_since_last_segment_received=_time_since_last_segment_received+ms_since_last_tick;//近似时间

    TCPState st_tw(TCPState::State::TIME_WAIT);//st_tw为TIME_WAIT所对应的TcpState
    if(state()==st_tw){

        _since_time_wait=_since_time_wait+ms_since_last_tick;
        
        if(_since_time_wait>=10*_cfg.rt_timeout){//这个地方是>=,是个test中的小坑
            //超过time_wait时间后,彻底关闭tcp连接
            _linger_after_streams_finish=false;
            _active=false;
        }

        //return;

    }else if(_active&&_sender.consecutive_retransmissions()>TCPConfig::MAX_RETX_ATTEMPTS){

        //连续重发过多的segment,将彻底关闭local peer

        _active=false;
        
        //这里需要先清空_sender和tcp_connection的segments_out队列。保证RST包能够直接发出
        while(!_sender.segments_out().empty())_sender.segments_out().pop();
        while(!segments_out().empty())segments_out().pop();

        _sender.send_empty_segment_RST();//要先发出rst包，再关闭sender和receiver的stream
        

        seg_2_seg();
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _linger_after_streams_finish=false;

        //return；
    }else{

        //_sender.fill_window();
        seg_2_seg();

        /**
         * 具体参见lab4的pdf。此处如果满足了clean shutdown所需要的四个条件，就会讲local close掉
         * 如果_linger_after_streams_finish==false,说明remote peer已经率先发送fin了。条件1被满足
         * 同时，当自己所有的data都被ack后，2,3,4会同时被满足。此时可以将自己close掉
        */
        if(!_linger_after_streams_finish&&_receiver.stream_out().input_ended()
            &&_sender.stream_in().eof()
            &&_sender.next_seqno_absolute() ==_sender.stream_in().bytes_written() + 2&&_sender.bytes_in_flight() == 0){

             _active=false;
        }
    }



    

}

void TCPConnection::end_input_stream() {


    _sender.stream_in().end_input();
    /**
     * 
     * 当input end后，要及时地发送fin。
     * 
     * 这里有一个细节要特别的注意。如果window不够，fin可能发不出去
     * 
     * 但事实上，即使某一时刻window不够，后续的包也能发出去。原因就是我们会发送send_one_byte_segment()。
     * 由于该segment包含1byte，local一定保证让它被发送成功，而对于对面的receiver，也不会忽略该segment。
     * 因此只要该segment被正式ack掉，就代表了win可能的变化，segment_received就可以"尝试"继续fill_window()。继续发送后续的正常segment(或
     * 者由于window限制继续发送send_one_byte_segment)。不管怎样一定能发送fin
     * 
     */ 
    _sender.fill_window();
    seg_2_seg();
}

void TCPConnection::connect() {

    //fill_window用来发送所有非空segment(空segment: syn,fin均置为0，payload为空)
    //可以用来发送包含syn/fin
    _sender.fill_window();

    seg_2_seg();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            

            _active=false;
            _linger_after_streams_finish=false;

            _sender.send_empty_segment_RST();
            seg_2_seg();
            _sender.stream_in().set_error();
            _receiver.stream_out().set_error();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

//
void TCPConnection::seg_2_seg(){


    while(!_sender.segments_out().empty()){

        TCPSegment& seg=_sender.segments_out().front();
        _sender.segments_out().pop();

        if(_receiver.ackno().has_value()){

            seg.header().ack=true;
            seg.header().ackno=*_receiver.ackno();
            seg.header().win=_receiver.window_size();
        }
        segments_out().push(seg);
        

    }

}
