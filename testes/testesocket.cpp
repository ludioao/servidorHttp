#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <map>
#include <regex>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <netdb.h>


#define CLIENTBUFFERSIZE 2048

using namespace std;

#define HDR_DELIMETER ": "
#define HDR_ENDLINE "\r\n"

const string get_request(string hostname)
{
    string method = "GET";
    string resource = "/";
    string version = "HTTP/1.1";

    string header = method + " "
            + resource + " "
            + version + HDR_ENDLINE;

    map<string, string>::iterator hdr_iter;

    header.append("Host");
    header.append(HDR_DELIMETER);
    header.append(hostname);
    header.append(HDR_ENDLINE);


    header.append("Connection");
    header.append(HDR_DELIMETER);
    header.append("close");
    header.append(HDR_ENDLINE);
    header.append(HDR_ENDLINE);

    /*for(hdr_iter = m_headers.begin(); hdr_iter != m_headers.end(); ++hdr_iter)
    {
        header.append(hdr_iter->first);
        header.append(HDR_DELIMETER);
        header.append(hdr_iter->second);
        header.append(HDR_ENDLINE);
    }*/

    cout << "Request: ********" << endl;
    cout << header << endl;
    cout << "End request: ***********" << endl;

    return header;
}

int main()
{

    string url_requested = "http://vps.dinamicweb.com.br";
    int port = 80;
    string response = "";

    bool success = true;

    response.clear();

    struct sockaddr_in web_sock_info;
    struct hostent *web_server_addr;

    bzero(&web_sock_info, sizeof(web_sock_info));

    string hostname = "vps.dinamicweb.com.br";
    //web_server_addr = hostname.c_str();//gethostbyname(url_requested.c_str());
    web_server_addr = gethostbyname(hostname.c_str());

    if(web_server_addr == NULL)
    {
        cout << "Error resolving address " << url_requested << endl;
        success = false;
    }

    web_sock_info.sin_family = AF_INET;
    web_sock_info.sin_port = htons(port);
    bcopy((char *)web_server_addr->h_addr, (char *)&web_sock_info.sin_addr.s_addr, web_server_addr->h_length);

    int web_socket = socket(AF_INET, SOCK_STREAM, 0);

    cout << "Url: " << url_requested << "\nPort: " << port << endl;

    if(web_socket == -1)
    {
        cout << "Could not create socket!" << endl;
    }

    success = connect(web_socket, (struct sockaddr*) &web_sock_info, sizeof(web_sock_info));

    if(success == 0)
    {
        cout << "Connected to web server successfully." << endl;
    }
    else
    {
        cout << "There was a problem connecting to the web server" << endl;
    }

    char rcv_buf[CLIENTBUFFERSIZE];

    const string request = get_request(hostname);

    cout << "requested " << request << endl;

    success = write(web_socket, request.c_str(), sizeof(request.c_str()));

    if(success)
    {

        cout << "---- Request sent:" << endl;
        cout << request << endl;
        cout << "End of request ----" << endl;

        cout << "Sent request to server." << endl;
    }

    int num_read = recv(web_socket, rcv_buf, CLIENTBUFFERSIZE, 0);

    cout << "Receiving?" << endl;

    cout << num_read << endl;
    if(num_read > 0)
    {
        rcv_buf[num_read] = '\0';
        cout << rcv_buf << endl;
    }

    cout << "Received: \n" << rcv_buf << endl;

    cout << response << endl;

}