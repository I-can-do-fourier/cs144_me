
#include <map>
#include <string>
#include<iostream>

using namespace std;


void printme(std::map<int,std::string> m){

    auto mit=m.begin();
    while(mit!=m.end()){

        cout<<mit->first<<","<<mit->second<<"  ";
        mit++;
    }
}

std::string combine(const string &s1,const string &s2,const uint64_t index1,const uint64_t index2){

    
    string left;
    string right;

    int in1;int in2;
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

int main(int argc, char const *argv[])
{


    size_t a=100;
    size_t b=1000;

    printf("minus:%d",a-b);

    /**
     * 
     * 
     *      ====   =======      ||     ======    ||     =======
     *            ===           ||       ====    || ===
     *    
     *      ==== ==  =======    ||  
     *         =======          ||
    */

    /**
     * 
     * 
     * 记录右侧index。
     * 
    */

   /**
    * 
    * 帮我想一个c++的数据结构以及相应的算法。它要实现如下的功能。
    * 1.能够储存区间。例如[1,3],[5,10]这样的区间
    * 2.能够保证一定的次序。例如，我可以让这些区间按照某些方式在数据结构里放置，方便快速查找
    * 3.当我插入一个新的区间时，要保证数据结构中的区间不能重叠。例如[1,4]和[2,7]就有重叠的部分。因此可能需要删除某个重叠的区间
    * 
    * 
   */
    std::map<int,std::string> m;

    m[9]="789";
    m[4]="4";
    m[3]="23";
    m[0]="0";

    printme(m);
    
    cout<<"\n";

    std::string s="!@#";
    int lb=-3;
    int rb=-1;

    cout<<"haha\n";


    int ecount=0;
    int icount=0;

    auto it=m.lower_bound(lb);
    auto fit=it;

    auto eit=fit;


    cout << (it->first-it->second.size()+1)<<"\n";
    //cout << it->first-it->second.size()+1-rb<<"\n";
    if(it==m.end()||(it->first-(int)it->second.size()+1>rb)){

        icount=rb-lb+1;
        m[rb]=s;
        cout<<icount<<","<<ecount<<"\n";
        printme(m);
        return 0;

    }
    while(it!=m.end()&&(it->first-it->second.size()+1<=rb)){

        ecount=ecount+it->second.size();
        eit=it;
        it++;
        
    }

    icount=max(rb,eit->first)-min(lb,fit->first-(int)fit->second.size()+1)+1;

    int _capacity;
    //if(icount-ecount>(_capacity-))

    string ins=combine(fit->second,s,fit->first-fit->second.size()+1,lb);
    ins=combine(eit->second,ins,eit->first-eit->second.size()+1,min(lb,fit->first-(int)fit->second.size()+1));

    m.erase(fit,it);
    m[max(rb,eit->first)]=ins;
    //it--;
    // cout << m[10]<<"\n"<<it->first<<"\n";
    cout<<icount<<","<<ecount<<"\n";

    printme(m);




    return 0;
}


