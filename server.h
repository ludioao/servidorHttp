#pragma once
#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include "http.h"

#define ACCEPT_RANGES  "Accept-Ranges: "
#define BYTES          "bytes"
#define CONTENT_TYPE   "Content-Type: "
#define CONTENT_LENGTH "Content-Length: "
#define DATE           "Date: "
#define TMPFILE        "tmpfile.out"

using std::map;
using std::pair;

//typedef              MIMETYPES;

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
};

class HttpServer {

    private: 

        SocketServer server;
        vector<pair<HttpRequest*, string> > cache;
        double elapsedtime;
        pthread_attr_t attr;
        pthread_mutex_t cachemutex;
        map<string, string> MimeTypes;

        
    public:
        // Constructor/Destructor
        HttpServer();
        ~HttpServer();  

        // Inicializar MimeTypes;
        void InitMimeTypes();

        // Multi-process request handling
        void Start(tipoServidor type, bool verbose, long int portNumber);
        void RunMultiProcessed(bool verbose);
        void DispatchRequestToChild(bool verbose, pair<int, string> client);

        // Multi-threaded request handling
        void RunMultiThreaded(bool verbose);
        void* DispatchRequestToThread(bool verbose, pair<int, string> client);
        static void* CallDispatchRequestToThread(void* args); 

        // Request handling methods
        void ParseRequest(HttpRequest& request, bool verbose, const char* recvbuf);
        string HandleRequestThreaded(HttpRequest& request, bool verbose, bool& cached);
        string HandleRequest(HttpRequest& request, bool verbose);

        // Response creating method
        string HandleGet(HttpRequest request, http_status_t status);
        
        string CreateResponseString(HttpRequest request, string response, string body, http_status_t status);
        
        // Helper methods
        http_method_t GetMethod(const string method);
        http_version_t GetVersion(const string version);
        string GetMimeType(string extension);
        void ParseUri(string& uri, string& path, string& query, string& type);
};

#endif
