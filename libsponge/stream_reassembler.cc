#include "stream_reassembler.hh"

#include<map>
#include<iostream>



// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity),m(){}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    DUMMY_CODE(data, index, eof);

    // string d=data;
    // if(_output.bytes_written()==index){

    //     size_t written= _output.write(data);
    //     if(written<data.size()){

            
    //     }
    // }

    if(eof)endIdx=index+data.size();
    AddTempCash(data,index);

    if(_output.bytes_written()==endIdx)_output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const { 
    
    
    return unassembled; 
    
}

bool StreamReassembler::empty() const { 
    
    
    return  unassembled==0; 
    
}


void StreamReassembler::AddTempCash(const std::string &data, const size_t index){

    
    //std::map<int,std::string> m;

    //printme();
    
    //cout<<"\n";

    // std::string s="!@#";
    // int lb=-3;
    // int rb=-1;

    // cout<<"haha\n";

    /**
     * 
     * =======
     *      ======
    */

   if(data.size()==0)return;

    string d=data;



    size_t ecount=0;
    size_t icount=0;

    size_t lb=index;
    size_t rb=index+data.size()-1;

    string s="";
    for(auto it = m.cbegin(); it != m.cend(); ++it){ 
    
        s=s+to_string(it->first-it->second.size()+1)+","+to_string(it->first)+"  ";

    }
    
    //如果index和已经写入到stream的部分重合，就需要截断
    if(index<_output.bytes_written()){

        //如果完全重合，返回
        if(rb<_output.bytes_written())return;
        d=data.substr(_output.bytes_written()-index,data.size());
        lb=_output.bytes_written();
    }


    //判断data整体是否超出了capacity的范围(capacity定义见下方或pdf)
    //超出部分需要截断
    if(overflow(rb)<0){

        //完全超出，返回
        if(overflow(lb)<0)return;

        rb=_output.bytes_read()+_capacity-1;
        d=d.substr(0,rb-lb+1);
    }


    /**
     * 
     * 将data放入map中,重叠部分需要合并
     * 
     * 大致思路
     * 1.找到第一个重叠的期间in1
     * 2.找到最后一块重叠的区间in2
     * 3.将data与第一块和第二块重叠区间合并
     * 4.删除[in1,in2]之间的区间,将合并后的区间放入map中
     * 
     * 需要注意边界情况
    */

    auto it=m.lower_bound(lb);

    //size_t itv=it->first;
    //cout<<itv;
    auto fit=it;

    auto eit=fit;


    //cout << (it->first-it->second.size()+1)<<"\n";
    //cout << it->first-it->second.size()+1-rb<<"\n";
    if(it==m.end()||(it->first-it->second.size()+1>rb)){

        //不和任意的区域重叠，即在map尾部，直接放入

        icount=rb-lb+1;

        //if(overflow(icount))return;
        

        m[rb]=d;
        unassembled=unassembled+icount;
        // cout<<icount<<","<<ecount<<"\n";
        // printme(m);
        //return;

    }else{

        
  
        //找到最后重叠的重叠部分
        while(it!=m.end()&&(it->first-it->second.size()+1<=rb)){

            ecount=ecount+it->second.size();
            eit=it;
            it++;

        }

        icount=max(rb,eit->first)-min(lb,fit->first-fit->second.size()+1)+1;

        //if(overflow(icount-ecount))return;

        //合并
        string ins=combine(fit->second,d,fit->first-fit->second.size()+1,lb);
        ins=combine(eit->second,ins,eit->first-eit->second.size()+1,min(lb,fit->first-fit->second.size()+1));

        size_t right=eit->first;
        m.erase(fit,it);
        //m[max(rb,eit->first)]=ins;不要使用已经被erased的元素
        m[max(rb,right)]=ins;
        unassembled=unassembled+icount-ecount;
        
    }

    //it--;
    // cout << m[10]<<"\n"<<it->first<<"\n";
    //cout<<icount<<","<<ecount<<"\n";



    //printme();

    //将能够塞到steam的元素都塞进去
    auto be=m.begin();
    while(be!=m.end()&&_output.bytes_written()==(be->first-be->second.size()+1)){

        _output.write(be->second);

        unassembled=unassembled-be->second.size();
        
        auto t=be;
        be++;
        m.erase(t);
        
    }
    // if(_output.bytes_written()==(be->first-be->second.size()+1)){

        
    //     _output.write(be->second);

    //     unassembled=unassembled-be->second.size();
        
    //     m.erase(be);
    //     return;

    // }

    return;
    
}

void StreamReassembler::printme(){

    
    cout<<"map:"<<m.size()<<endl;

    if(m.size()==0)return;
    // size_t a=1;
    // cerr<<a;
    //auto mit=m.begin();

    string s="";
for(auto it = m.cbegin(); it != m.cend(); ++it)
{
    //printf("%lu,%s   ",it->first,it->second.c_str());
    //s=s+to_string(it->first-it->second.size()+1)+","+to_string(it->first)+"  ";
    //printf("%lu,%s   ",it->first,it->second.c_str());
}

    printf("%s\n",s.c_str());

}

std::string combine(const string &s1,const string &s2,const size_t index1,const size_t index2){

    
    string left;
    string right;

    size_t in1;size_t in2;
    if(index1<index2){
        
        left=s1;right=s2;
        in1=index1;in2=index2;
    }else{

        left=s2;right=s1;
        in1=index2;in2=index1;
    }

    if(in2+right.size()<=in1+left.size())return left;

    /**
     * 
     *    111111
     *       111112  
     * 
    */
    string res=left+right.substr(in1+left.size()-in2,right.size());

    return res;
}

int StreamReassembler::overflow(size_t rb){

    /**
     * 
     * 
     * 是否超出capacity要根据指导书中的那张图来判断
     * 超出的条件并不是总量，而是最后最后一个byte的位置
     * 
    */

    // if(_output.bytes_read()+_capacity-1>=rb){
    //    return _output.bytes_read()+_capacity-1-rb;
    // }

    // return rb-(_output.bytes_read()+_capacity-1);
    
    //fix bug
    return _output.bytes_read()+_capacity-1-rb;

}

void StreamReassembler::discard(){

    size_t total=_output.buffer_size()+unassembled;
    if(total<=_capacity){
        return;
    }

    auto it=m.end();
    it--;
    
    while(total>_capacity){

    }
}