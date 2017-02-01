#pragma once
#ifndef HTTPSERVER_H
#define HTTPSERVER_H

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
#include "SocketServer.h"
#include "HttpRequest.h"

#define ACCEPT_RANGES  "Accept-Ranges: "
#define BYTES          "bytes"
#define CONTENT_TYPE   "Content-Type: "
#define CONTENT_LENGTH "Content-Length: "
#define DATE           "Date: "
#define TMPFILE        "tmpfile.out"

using namespace std;

class HttpServer {

    private: 

        SocketServer server;
        vector<pair<HttpRequest*, string> > cache;
        double elapsedtime;
        pthread_attr_t attr;
        //pthread_mutex_t cachemutex;
        map<string, string> MimeTypes;
        int maxthreads;

        
    public:
        // Constructor/Destructor
        HttpServer();
        ~HttpServer();  

        // Inicializar MimeTypes;
        void InitMimeTypes();

        // Multi-process request handling
        void Start(tipoServidor type, bool verbose, long int portNumber, int);
        void RunMultiProcessed(bool verbose);
        void DispatchRequestToChild(bool verbose, pair<int, string> client);

        // Multi-threaded request handling
        void RunMultiThreaded(bool verbose);
        void* DispatchRequestToThread(bool verbose, pair<int, string> client);
        static void* CallDispatchRequestToThread(void* args); 

        // Request handling methods
        void ParseRequest(HttpRequest& request, bool verbose, const char* recvbuf);
        string HandleRequestThreaded(HttpRequest& request, bool verbose);
        string HandleRequest(HttpRequest& request, bool verbose);

        // Response creating method
        string HandleGet(HttpRequest request, http_status_t status);
        
        string CreateResponseString(HttpRequest request, string response, string body, http_status_t status);   

        /*
        Metodos helpers
        */
        http_method_t GetMethod(const string method);
        http_version_t GetVersion(const string version);
        string GetMimeType(string extension);
        void ParseUri(string& uri, string& path, string& query, string& type);
        string CreateIndexHtml(const string path);
        string CreateIndexList(const string path);
        bool IsDirectory(const string path);
};


#endif

