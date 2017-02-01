#ifndef SOCKET_H
#define SOCKET_H
#include <map>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <algorithm>    // std::sort
#include <vector>       // std::vector
#include <iostream>

using namespace std;


#define BUFFER_LENGTH  8191
#define BACKLOG        128

enum tipoServidor {
    MPROCESS = 0, MTHREADED, EVENTED,
};

struct mthreaded_request_args {
    pair<int, string> client;
    void* ptr;
    bool verbose;
};

class SocketServer {
    private:
        
        // Porta do Servidor
        long int portNumber;

        int maxThreads;


        // Addresses for the server and the client
        struct sockaddr_in serveraddr;
        struct sockaddr_in clientaddr;
        char peername[INET_ADDRSTRLEN];

         
        // Socket file descriptors
        int listening;
        char recvbuf[BUFFER_LENGTH + 1];
    public:
        // Constructor/Destructor
        SocketServer();
        ~SocketServer();

        // Receiving buffer
        const char* get_buffer() { return recvbuf; }

        void Init();

        // Socket call wrapper methods
        pair<int, string> Connect();
        bool Receive(bool verbose, pair<int, string> client);
        bool SendResponse(string buffer, int connection);
        bool Close(int connection);

        // Seta porta de operacao
        void setPortNumber(long int number);
        long int getPortNumber();

        // Helper methods
        int getMaxThreads(){ return maxThreads; };
        void setMaxThreads(int val){ maxThreads = val; std::cout << "setei ess abagaca " << maxThreads << std::endl; };
};
#endif