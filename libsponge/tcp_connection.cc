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
    

    return _sender.stream_in().buffer_size();
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

    if(hd.rst){
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _active=false;
        _linger_after_streams_finish=false;

        return;
    }
    
    _receiver.segment_received(seg);
    if(hd.ack){

        _sender.ack_received(hd.ackno,hd.win);
    }

    if(seg.length_in_sequence_space()>0){

        _sender.fill_window();
        seg_2_seg();
    }

     if (_receiver.ackno().has_value()&&seg.length_in_sequence_space() == 0 ) {
         _sender.send_empty_segment();
         seg_2_seg();
    }   

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
        
        if(_since_time_wait>10*_cfg.rt_timeout){

            _linger_after_streams_finish=false;
            _active=false;
        }

        //return;

    }else if(_active&&_sender.consecutive_retransmissions()>TCPConfig::MAX_RETX_ATTEMPTS){

        _active=false;
        


        _sender.send_empty_segment_RST();
        seg_2_seg();
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _linger_after_streams_finish=false;

        //return；
    }

    

}

void TCPConnection::end_input_stream() {


    _sender.stream_in().end_input();
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

        segments_out().push(_sender.segments_out().front());
        _sender.segments_out().pop();

    }

}
