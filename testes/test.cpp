/**
    C++ client example using sockets
*/
#include<iostream>    //cout
#include<stdio.h> //printf
#include<cstring>    //strlen
#include<string>  //string
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<netdb.h> //hostent

#define HDR_DELIMETER   ": "
#define HDR_ENDLINE     "\r\n"

using namespace std;

/**
TCP Client class
*/
class TcpClient
{
    private:
        int sock;
        string address;
        int port;
        struct sockaddr_in server;
        
    public:
        TcpClient();
        bool conn(string, int);
        bool send_data(string data);
        string receive(int);
};
 
TcpClient::TcpClient()
{
    sock = -1;
    port = 0;
    address = "";
}
 
/**
    Connect to a host on a certain port number
*/
bool TcpClient::conn(string address , int port)
{
    //create socket if it is not already created
    if(sock == -1)
    {
        //Create socket
        sock = socket(AF_INET , SOCK_STREAM , 0);
        if (sock == -1)
        {
            perror("Could not create socket");
        }
         
        cout<<"Socket created\n";
    }
    else    {   /* OK , nothing */  }
     
    //setup address structure
    if(inet_addr(address.c_str()) == -1)
    {
        struct hostent *he;
        struct in_addr **addr_list;
         
        //resolve the hostname, its not an ip address
        if ( (he = gethostbyname( address.c_str() ) ) == NULL)
        {
            //gethostbyname failed
            herror("gethostbyname");
            cout<<"Failed to resolve hostname\n";
             
            return false;
        }
         
        //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
        addr_list = (struct in_addr **) he->h_addr_list;
 
        for(int i = 0; addr_list[i] != NULL; i++)
        {
            //strcpy(ip , inet_ntoa(*addr_list[i]) );
            server.sin_addr = *addr_list[i];
             
            cout<<address<<" resolved to "<<inet_ntoa(*addr_list[i])<<endl;
             
            break;
        }
    }
     
    //plain ip address
    else
    {
        server.sin_addr.s_addr = inet_addr( address.c_str() );
    }
     
    server.sin_family = AF_INET;
    server.sin_port = htons( port );
     
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    cout<<"Connected\n";
    return true;
}
 
/**
    Send data to the connected host
*/
bool TcpClient::send_data(string data)
{
    //Send some data
    if( send(sock , data.c_str() , strlen( data.c_str() ) , 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }
    cout<<"Data send\n";
     
    return true;
}
 
/**
    Receive data from the connected host
*/
string TcpClient::receive(int size=512)
{
    char buffer[size];
    string reply;
     
    //Receive a reply from the server
    if( recv(sock , buffer , sizeof(buffer) , 0) < 0)
    {
        puts("recv failed");
    }
     
    reply = buffer;
    return reply;
}
 
int main(int argc , char *argv[])
{
    TcpClient c;
    string host;
     
    cout << "Enter hostname : ";
    cin >> host;
     
    //connect to host
    c.conn(host , 80);
     
    //send some data
    
    string request_data;
    request_data = "GET /sisadv/public/ HTTP/1.1\n";
    //request_data.append( HDR_ENDLINE );
    request_data.append("Host");
    request_data.append(HDR_DELIMETER);
    request_data.append(host);
    request_data.append(HDR_ENDLINE);
    request_data.append("Connection");
    request_data.append(HDR_DELIMETER);
    request_data.append("close");
    request_data.append(HDR_ENDLINE);
    request_data.append(HDR_ENDLINE);
            
    cout << "Sending data..." << endl;
    cout << request_data << endl;
    cout << endl;

    c.send_data(request_data.c_str());

     
    //receive and echo reply
    cout<<"----------------------------\n\n";
    cout<<c.receive( 2048 );
    cout<<"\n\n----------------------------\n\n";
     
    //done
    return 0;
}