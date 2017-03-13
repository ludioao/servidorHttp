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

    

}