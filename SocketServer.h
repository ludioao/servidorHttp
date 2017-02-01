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
#define BACKLOG        128 // limite maximo 

enum tipoServidor {
    MPROCESS = 0, MTHREADED
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

        // Numero maximo de Threads p/ programa processar.
        int maxThreads;

        
        // Enderecos para o servidor e o cliente
        struct sockaddr_in serveraddr;
        struct sockaddr_in clientaddr;

        char peername[INET_ADDRSTRLEN];

        // Helper de arquivo socket.
        int listening; // helper p/ escuta.
        char recvbuf[BUFFER_LENGTH + 1]; // buffer de recebimento

    public:
        // Constructor/Destructor
        SocketServer();
        ~SocketServer();

        // Recebendo o buffer.
        const char* get_buffer() { return recvbuf; }

        // Inicializando Socket.
        void Init();

        // Metodo wrapper de chamada de socket.
        pair<int, string> Connect();
        bool Receive(bool verbose, pair<int, string> client);
        bool SendResponse(string buffer, int connection);
        bool Close(int connection);

        // Seta porta de operacao
        void setPortNumber(long int number);
        long int getPortNumber();

        // Metodos helper.
        int getMaxThreads(){ return maxThreads; };
        void setMaxThreads(int val){ maxThreads = val; /* std::cout << "setei ess abagaca " << maxThreads << std::endl; */ };
};
#endif