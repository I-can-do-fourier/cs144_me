#include "socket.hh"
#include "util.hh"

#include <cstdlib>
#include <iostream>

#include<string.h>

using namespace std;

void get_URL(const string &host, const string &path) {
    // Your code here.

    // You will need to connect to the "http" service on
    // the computer whose name is in the "host" string,
    // then request the URL path given in the "path" string.

    // Then you'll need to print out everything the server sends back,
    // (not just one call to read() -- everything) until you reach
    // the "eof" (end of file).

    /*
        create a socket
        needn't to use bind
        
    */

     TCPSocket ts;
     Address ad(host,"http");

      /*
        connect with host
    */

    ts.connect(ad);




   
    /*
        
        request the bytestream from the path of the host
    */ 

    string mesg;

    mesg="GET "+path+" HTTP/1.1\r\n";
    mesg=mesg+"Host: "+host+"\r\n";
    mesg=mesg+"Connection: close\r\n\r\n";//这里要敲两下回车
  
    
    cout<<mesg<<endl;
    //BufferViewList bf

    size_t size=ts.write(mesg,true);//默认是阻塞态

    if(size<strlen(mesg.c_str())){

        printf("error000");
        throw  runtime_error("write fewer bytes than needed");
    }



   /*
   
    receive the bytes until the end of the file
   */

    std::string s;
    while(true){

        if(ts.eof())break;
        s=ts.read();
        printf("%s",s.c_str());
        //cout << s;
        

    }

    //cout<<endl;

    //cerr << "Function called: get_URL(" << host << ", " << path << ").\n";
    //cerr << "Warning: get_URL() has not been implemented yet.\n";
}

int main(int argc, char *argv[]) {
    try {
        if (argc <= 0) {
            abort();  // For sticklers: don't try to access argv[0] if argc <= 0.
        }

        // The program takes two command-line arguments: the hostname and "path" part of the URL.
        // Print the usage message unless there are these two arguments (plus the program name
        // itself, so arg count = 3 in total).
        if (argc != 3) {
            cerr << "Usage: " << argv[0] << " HOST PATH\n";
            cerr << "\tExample: " << argv[0] << " stanford.edu /class/cs144\n";
            return EXIT_FAILURE;
        }

        // Get the command-line arguments.
        const string host = argv[1];
        const string path = argv[2];

        // Call the student-written function.
        get_URL(host, path);
    } catch (const exception &e) {
        cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

