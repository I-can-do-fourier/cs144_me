#include "byte_stream.hh"
#include<iostream>
// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t cp):capacity(cp){ 
    
    DUMMY_CODE(cp); 
    
}

size_t ByteStream::write(const string &data) {
    DUMMY_CODE(data);

    if(data.size()<=remaining_capacity()){


        buffer.append(data);
        written=written+data.size();
        return data.size();

    }

    int count=this->remaining_capacity();
    buffer.append(data.substr(0,count));
    written=written+count;
    
    return count;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    DUMMY_CODE(len);
    return buffer.substr(0,len);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    
    
    DUMMY_CODE(len); 

    //string res=buffer.substr(0,len);
    //std::unique_ptr<std::string> buf (new string(buffer->substr(len,buffer->size())));
    buffer=buffer.substr(len,buffer.size());

    total_read=total_read+len;//这句话一定要写在这，不能卸载read里

}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    DUMMY_CODE(len);

    int l;
    if(len>buffer.size())l=buffer.size();
    else l=len;
    std::string res=peek_output(l);
    pop_output(l);

    return res;
}

void ByteStream::end_input() {
    
    
    //<<"set end";
    end=1;
    
    
}

bool ByteStream::input_ended() const { return end; }

size_t ByteStream::buffer_size() const { return buffer.size(); }

bool ByteStream::buffer_empty() const { 
    
    
    if(buffer.size()==0)return true;
    return false; 
    
    
}

bool ByteStream::eof() const { 
    
    return (input_ended()&&buffer.size()==0);
 }

size_t ByteStream::bytes_written() const { return written; }

size_t ByteStream::bytes_read() const { return total_read; }

size_t ByteStream::remaining_capacity() const { return capacity-buffer.size();}
