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

    if(_sender.next_seqno_absolute() == 0&&seg.header().ack)return;


    if(hd.rst){
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _active=false;
        _linger_after_streams_finish=false;

        return;
    }

    if(hd.fin&&(!_sender.stream_in().eof()||_sender.next_seqno_absolute()<_sender.stream_in().bytes_written() + 2)){

        _linger_after_streams_finish=false;
    }
    
    //待定
    //if(!(_receiver.ackno().has_value()&&seg.header().seqno == _receiver.ackno().value() - 1))
    _receiver.segment_received(seg);
    if(hd.ack){

        _sender.ack_received(hd.ackno,hd.win);
        
        _sender.fill_window();
    }

    if(_sender.next_seqno_absolute()==0&&_receiver.ackno().has_value()){

        _sender.fill_window();
    }

    if(seg.length_in_sequence_space()>0){

        //seg_2_seg();    
        //_sender.fill_window();
        if(_sender.segments_out().empty())_sender.send_empty_segment();
        //seg_2_seg();
    }

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

    _sender.fill_window();

    seg_2_seg();
    return res;

}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { 
    
    DUMMY_CODE(ms_since_last_tick); 
    
    _sender.tick(ms_since_last_tick);
    _time_since_last_segment_received=_time_since_last_segment_received+ms_since_last_tick;

    TCPState st_tw(TCPState::State::TIME_WAIT);
    if(state()==st_tw){

        _since_time_wait=_since_time_wait+ms_since_last_tick;
        
        if(_since_time_wait>=10*_cfg.rt_timeout){

            _linger_after_streams_finish=false;
            _active=false;
        }

        //return;

    }else if(_active&&_sender.consecutive_retransmissions()>TCPConfig::MAX_RETX_ATTEMPTS){

        _active=false;
        

        while(!_sender.segments_out().empty())_sender.segments_out().pop();

        _sender.send_empty_segment_RST();
        
        while(!segments_out().empty())segments_out().pop();
        seg_2_seg();
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _linger_after_streams_finish=false;

        //return；
    }else{

        seg_2_seg();

        if(!_linger_after_streams_finish&&_receiver.stream_out().input_ended()
            &&_sender.stream_in().eof()
            &&_sender.next_seqno_absolute() ==_sender.stream_in().bytes_written() + 2&&_sender.bytes_in_flight() == 0){

             _active=false;
        }
    }



    

}

void TCPConnection::end_input_stream() {


    _sender.stream_in().end_input();
    
    _sender.fill_window();
    seg_2_seg();
}

void TCPConnection::connect() {

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
