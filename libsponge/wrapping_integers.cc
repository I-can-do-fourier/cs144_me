#include "wrapping_integers.hh"
#include<iostream>
// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    DUMMY_CODE(n, isn);

    uint64_t ISN(isn.raw_value());
    n=n+ISN;
    printf("%lu",n%UINT32_MAX);
    cout<<n<<endl;
    //operator<<(isn);
    //cout<<isn;
    n=n%(1ll << 32);//UINT32_MAX不行；

    return WrappingInt32{static_cast<uint32_t>(n & 0xFFFFFFFF)};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    DUMMY_CODE(n, isn, checkpoint);

    uint64_t ISN(isn.raw_value());
    uint64_t N(n.raw_value());


    //uint64_t diff=UINT64_MAX;

    uint64_t BIG=1ll << 32;

    while(N<ISN){

        N=N+BIG;
    }


    uint64_t abs=N-ISN;

    if(abs>=checkpoint)return abs;

    uint64_t m=(checkpoint-abs)/BIG;//求一个大致的倍数;加快计算速度
    
    abs=abs+BIG*m;

    while(checkpoint>=abs){

        abs=abs+BIG;
        //cout<<abs<<endl;

    }

        

    if(abs<BIG)return abs;

    if((abs-checkpoint)>=checkpoint-(abs-BIG)){

       abs=abs-BIG;
       
    }

    


    return abs;
}
