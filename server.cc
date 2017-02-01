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


#include "http.h"
#include "server.h"

using std::cerr;
using std::cout;
using std::endl;
using std::fstream;
using std::make_pair;
using std::pair;
using std::streambuf;
using std::string;
using std::stringstream;
using std::system;
using std::to_string;
using std::vector;
using std::regex;
using std::regex_replace;

static bool running = true;

const unsigned char IS_FILE_FLAG = 0x8;


////////////////////////////////////////////////
//              Sig Handlers                  //
////////////////////////////////////////////////
void handleSigint(int signum) {
    // Turn off event loop
    running = false;
}

void handleSigchld(int signum) {
    // Prevent zombie processes
    int status;
    while (waitpid(-1, &status, WNOHANG) == 0);
}

////////////////////////////////////////////////
//              Misc Helpers                  //
////////////////////////////////////////////////

// Hex to ASCII helper, used for parsing URIs
char hexToAscii(string hex) {
    int ascii = 0;
    int num;
    if (isalpha(hex[0])) {
        num = 16 * (hex[0] - 55);
    } else {
        num = 16 * (hex[0] - 48);
    }
    ascii += num;

    if (isalpha(hex[1])) {
        num = hex[1] - 55;
    } else {
        num = hex[1] - 48;
    }
    ascii += num;
    return (char) ascii;
}


////////////////////////////////////////////////
//              SocketServer                  //
////////////////////////////////////////////////

SocketServer::SocketServer()
{
    cout << "Instanciando SocketServer " << endl;
}

void
SocketServer::Init() {
    int error = 0;
    int flags = 0;

    // Zero initialize buffers and socket addresses
    memset(recvbuf, (char) NULL, sizeof(recvbuf));
    memset(peername, (char) NULL, sizeof(peername));
    memset(&serveraddr, (char) NULL, sizeof(serveraddr));
    memset(&clientaddr, (char) NULL, sizeof(clientaddr));

    // Create a socket and bind to our host address
    listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening < 0) {
        perror("socket");
        close(listening);
        exit(EXIT_FAILURE);
    }

    // Set socket reuse and non-blocking options
    setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, NULL, 0);
    flags = fcntl(listening, F_GETFL, 0);
    fcntl(listening, F_SETFL, flags | O_NONBLOCK);

    // Server socket information
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(getPortNumber());


    cout << "serveraddr.sin_port => " << serveraddr.sin_port <<  endl;
    // Mensagem inicializando; ;

    // Bind socket to a local address
    error = bind(listening, (const struct sockaddr *) &serveraddr, sizeof(serveraddr));
    if (error < 0) {
        perror("bind");
        close(listening);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    error = listen(listening, BACKLOG);
    if (error < 0) {
        perror("listen");
        close(listening);
        exit(EXIT_FAILURE);
    }


}

SocketServer::~SocketServer() {
    // Close listening socket 
    close(listening);
}

void 
SocketServer::setPortNumber(long int number)
{
    portNumber = number;
    
}

long int
SocketServer::getPortNumber()
{
    return portNumber;
}

std::pair<int, string> SocketServer::Connect() {
    // Accept any incoming connections
    socklen_t length = sizeof(clientaddr);
    int error = 0;
    int connection = accept(listening, NULL, NULL);
    string peer = "";

    if (connection > 0) {
        // Get connecting client's network info
        error = getpeername(connection, (struct sockaddr *)&clientaddr, &length);
        if (error < 0) {
            perror("getpeername");
            memset(peername, (char) NULL, sizeof(peername));
        } else {
            inet_ntop(AF_INET, &(clientaddr.sin_addr), peername, length);
            peer = string(peername);
        }
    }
    return make_pair(connection, peer);
}

bool SocketServer::Receive(bool verbose, pair<int, string> client) {
    int connection = client.first;
    string peer = client.second;

    // Receive bytes from client connection
    int count = recv(connection, recvbuf, BUFFER_LENGTH, 0);
    if (count <= 0) {
        return false;
    }
    
    // Logging and NULL termination
    recvbuf[count] = (char) NULL;
    if (verbose) {
        cout << "Received " << count << " bytes from " << peer << ":\n";
        cout << recvbuf << endl;
    }
    return true;
}

bool SocketServer::SendResponse(string buffer, int connection) {
    // Send buffer over socket, no need for NULL termination
    int count = send(connection, buffer.c_str(), buffer.length() - 1, 0);
    if (count < 0) {
        perror("send");
        return false;
    }
    return true;
}

bool SocketServer::Close(int connection) {
    // Close connection specified by file descriptor
    int error = close(connection);
    if (error < 0) {
        perror("close");
        return false;
    }
    return true;
}

////////////////////////////////////////////////
//              HttpServer                    //
////////////////////////////////////////////////
HttpServer::HttpServer()
{
    elapsedtime = 0.0;
    InitMimeTypes();
}


HttpServer::~HttpServer() {
    cout << "Finalizando HTTPServer " << endl;
}



void
HttpServer::InitMimeTypes()
{    
    MimeTypes["doc"]    = "application/msword";
    MimeTypes["bin"]    = "application/octet-stream";
    MimeTypes["dll"]    = "application/octet-stream";
    MimeTypes["exe"]    = "application/octet-stream";
    MimeTypes["pdf"]    = "application/pdf";
    MimeTypes["p7c"]    = "application/pkcs7-mime";
    MimeTypes["ai"]     = "application/postscript";
    MimeTypes["eps"]    = "application/postscript";
    MimeTypes["ps"]     = "application/postscript";
    MimeTypes["rtf"]    = "application/rtf";
    MimeTypes["fdf"]    = "application/vnd.fdf";
    MimeTypes["arj"]    = "application/x-arj";
    MimeTypes["gz"]     = "application/x-gzip";
    MimeTypes["class"]  = "application/x-java-class";
    MimeTypes["js"]     = "application/x-javascript";
    MimeTypes["lzh"]    = "application/x-lzh";
    MimeTypes["lnk"]    = "application/x-ms-shortcut";
    MimeTypes["tar"]    = "application/x-tar";
    MimeTypes["hlp"]    = "application/x-winhelp";
    MimeTypes["cert"]   = "application/x-x509-ca-cert";
    MimeTypes["zip"]    = "application/zip";
    MimeTypes["cab"]    = "application/x-compressed";
    MimeTypes["arj"]    = "application/x-compressed";
    MimeTypes["aif"]    = "audio/aiff";
    MimeTypes["aifc"]   = "audio/aiff";
    MimeTypes["aiff"]   = "audio/aiff";
    MimeTypes["au"]     = "audio/basic";
    MimeTypes["snd"]    = "audio/basic";
    MimeTypes["mid"]    = "audio/midi";
    MimeTypes["rmi"]    = "audio/midi";
    MimeTypes["mp3"]    = "audio/mpeg";
    MimeTypes["vox"]    = "audio/voxware";
    MimeTypes["wav"]    = "audio/wav";
    MimeTypes["ra"]     = "audio/x-pn-realaudio";
    MimeTypes["ram"]    = "audio/x-pn-realaudio";
    MimeTypes["bmp"]    = "image/bmp";
    MimeTypes["gif"]    = "image/gif";
    MimeTypes["jpeg"]   = "image/jpeg";
    MimeTypes["jpg"]    = "image/jpeg";
    MimeTypes["tif"]    = "image/tiff";
    MimeTypes["tiff"]   = "image/tiff";
    MimeTypes["xbm"]    = "image/xbm";
    MimeTypes["wrl"]    = "model/vrml";
    MimeTypes["htm"]    = "text/html";
    MimeTypes["html"]   = "text/html";
    MimeTypes["c"]      = "text/plain";
    MimeTypes["cpp"]    = "text/plain";
    MimeTypes["def"]    = "text/plain";
    MimeTypes["h"]      = "text/plain";
    MimeTypes["txt"]    = "text/plain";
    MimeTypes["rtx"]    = "text/richtext";
    MimeTypes["rtf"]    = "text/richtext";
    MimeTypes["java"]   = "text/x-java-source";
    MimeTypes["css"]    = "text/css";
    MimeTypes["mpeg"]   = "video/mpeg";
    MimeTypes["mpg"]    = "video/mpeg";
    MimeTypes["mpe"]    = "video/mpeg";
    MimeTypes["avi"]    = "video/msvideo";
    MimeTypes["mov"]    = "video/quicktime";
    MimeTypes["qt"]     = "video/quicktime";
    MimeTypes["shtml"]  = "wwwserver/html-ssi";
    MimeTypes["asa"]    = "wwwserver/isapi";
    MimeTypes["asp"]    = "wwwserver/isapi";
    MimeTypes["cfm"]    = "wwwserver/isapi";
    MimeTypes["dbm"]    = "wwwserver/isapi";
    MimeTypes["isa"]    = "wwwserver/isapi";
    MimeTypes["plx"]    = "wwwserver/isapi";
    MimeTypes["url"]    = "wwwserver/isapi";
    MimeTypes["cgi"]    = "wwwserver/isapi";
    MimeTypes["php"]    = "wwwserver/isapi";
    MimeTypes["wcgi"]   = "wwwserver/isapi";
}

// End of file


void 
HttpServer::Start(tipoServidor type, bool verbose, long int port_Operacao, int numberThreads) {
    
    // Add signal handlers
    signal(SIGINT, handleSigint);
    signal(SIGCHLD, handleSigchld);

    // Porta default
    server.setPortNumber(port_Operacao);
    server.Init();

    long int actualPortNumber = server.getPortNumber();
    
    cout << "Inicializando servidor na porta " << actualPortNumber << endl;
    cout << "Modo de operacao:";

    // Run with flag options
    if (type == MPROCESS) {
        cout << " MULTIPROCESSO " << endl;
        RunMultiProcessed(verbose);

    } else if (type == MTHREADED) {
        cout << " THREADED " << endl;
        server.setMaxThreads(numberThreads);
        RunMultiThreaded(verbose);
        cout << "numero de threads "<< numberThreads << endl;
        
    } 
}

void 
HttpServer::RunMultiProcessed(bool verbose) {
    pid_t pid;
    pair<int, string> client;
    int connection;
    
    if (verbose) {
        cout << "Server starting...\n\n";
        cout << "Currently at port " << server.getPortNumber() << endl;
    }

    // Event loop
    while (running) {

        // Wait for a connection
        client = server.Connect();
        connection = client.first;

        if (connection > 0) {
            // Fork a new server process to handle client connection
            pid = fork();
            if (pid < 0) {
                // Error 
                perror("fork");
                continue;
            } else if (pid == 0) {
                // Child process
                DispatchRequestToChild(verbose, client);
            } else {
                // Parent process
                server.Close(connection);
            }
        }
        // Sleep if not connected
        usleep(SLEEP_MSEC);
    }
    if (verbose) {
        cout << "Server shutting down...\n";
    }
}

void 
HttpServer::DispatchRequestToChild(bool verbose, pair<int, string> client) {
    HttpRequest request;
    string response;
    time_t begin;
    time_t end;
    int connection = client.first;

    // End process when elapsed time exceeds time out interval
    time(&begin);
    time(&end);
    elapsedtime = difftime(end, begin);

    do {
        if (server.Receive(verbose, client)) { 
            // Handle request and send response
            ParseRequest(request, verbose, server.get_buffer());
            response = HandleRequest(request, verbose);
            server.SendResponse(response, connection);
        }
        // Calculate elapsed time
        time(&end);
        elapsedtime = difftime(end, begin);
    } while (elapsedtime < TIME_OUT);

    // Close connection and exit
    server.Close(connection);
    exit(EXIT_SUCCESS);
}

void 
HttpServer::RunMultiThreaded(bool verbose) {
    vector<pthread_t> threadlist;
    pthread_t newthread;
    mthreaded_request_args args;
    pair<int, string> client;
    int connection;
    int error;

    // Initialize thread attributes and mutex
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (verbose) {
        cout << "Server starting...\n\n";
    }

    // Event loop waits for any new connections
    while (running) {
        client = server.Connect();
        connection = client.first;

  /*      cout << "threadlist  " << threadlist.size() << endl;

        cout << "connection passou a ser " << connection << endl;

        cout << "threadlist size is " << threadlist.size() << endl;
        cout << "maxthreads size is " << getMaxThreads() << endl;
*/
        if (connection > 0 && (int) threadlist.size() <= server.getMaxThreads()) {
            // Create a new thread and dispatch thread
            args.verbose = verbose;
            args.client = client;
            args.ptr = this;

            // Add new thread to threadlist
            error = pthread_create(&newthread, &attr, HttpServer::CallDispatchRequestToThread, &args);
            if (error < 0) {
                perror("pthread_create");
            }
            threadlist.push_back(newthread);
        }
        // Sleep if no connections
        usleep(SLEEP_MSEC);
    }

    if (verbose) {
        cout << "Server shutting down...\n";
    }

    // Join all finished threads
    while (!threadlist.empty()) {
        error = pthread_join(threadlist.back(), NULL);
        if (error < 0) {
            perror("pthread_join");
        }
        threadlist.pop_back();
    }

    // Clean up mutex and attributes
    pthread_attr_destroy(&attr);
    

    // Allow main thread to exit while waiting for any threads to finish
    pthread_exit(NULL);
}

void* 
HttpServer::DispatchRequestToThread(bool verbose, pair<int, string> client) {
    HttpRequest *request = new HttpRequest;
    string response;
    time_t begin;
    time_t end;
    int connection = client.first;
    
        
    // End process when elapsed time exceeds time out interval
    time(&begin);
    time(&end);
    elapsedtime = difftime(end, begin);

    do {
        if (server.Receive(verbose, client)) { 
            // Handle request and send response
            ParseRequest(*request, verbose, server.get_buffer());

            // Lock mutex before call
            response = HandleRequestThreaded(*request, verbose);
            server.SendResponse(response, connection);
        }
        // Calculate elapsed time
        time(&end);
        elapsedtime = difftime(end, begin);
    } while (elapsedtime < TIME_OUT);


    // Close connection and exit
    server.Close(connection);
    pthread_exit(NULL);
}

void* 
HttpServer::CallDispatchRequestToThread(void* args) {
    // Unpack arguments and return a function pointer
    mthreaded_request_args* arguments = (mthreaded_request_args*) args;
    bool verbose = arguments->verbose;
    pair<int, string> client = arguments->client;
    void* ptr = arguments->ptr;
    return ((HttpServer*) ptr)->DispatchRequestToThread(verbose, client); 
}


void 
HttpServer::ParseRequest(HttpRequest& request, bool verbose, const char* recvbuf) {
    int i = 0;
    http_method_t method;
    http_version_t version;
    string buffer = "";
    string path = "";
    string query = "";
    string type = "";
    string uri = "";

    // Keep a copy of the original request string
    string copy = recvbuf;

    // Get method string
    while (i <= BUFFER_LENGTH && !isspace(recvbuf[i])) {
        buffer += recvbuf[i];
        i++;
    }
    i++;

    // Parse method
    method = GetMethod(buffer);
    if (verbose && method == INVALID_METHOD) {
        cout << "Unrecognized HTTP method\n";
    }

    // Get URI
    while (i <= BUFFER_LENGTH && !isspace(recvbuf[i])) {
        uri += recvbuf[i];
        i++;
    }
    path = DIRECTORY;

    // Parse URI, sanitizing output
    ParseUri(uri, path, query, type);
    i++;
    if (verbose && path.length() > URI_MAX_LENGTH) {
        cout << "Request URI too long.\n";
    }

    // Get version number
    buffer = "";
    while (i <= BUFFER_LENGTH && !isspace(recvbuf[i])) {
        buffer += recvbuf[i];
        i++;
    }

    // Skip to next line
    while (isspace(recvbuf[i])) {
        i++;
    }

    // Parse version number
    version = GetVersion(buffer);
    if (verbose && version == INVALID_VERSION) {
        cout << "Invalid HTTP version.\n";
    }

    // Fill request struct
    request.Initialize(method, version, copy, path, query, type);
}

string 
HttpServer::HandleRequestThreaded(HttpRequest& request, bool verbose) {
    string response = "";

    response = HandleRequest(request, verbose);

    return response;
}

string 
HttpServer::HandleRequest(HttpRequest& request, bool verbose) {
    bool toolong = request.get_flag();
    http_status_t status = OK;
    http_method_t method = request.get_method();
    http_version_t version = request.get_version();
    string path = request.get_path();
    string type = request.get_content_type();
    string response = "";
    fstream file;


    // HTTP flow diagram starts here
    if (toolong) {
        // Request entity too large
        status = REQUEST_ENTITY_TOO_LARGE;
    } else if (version == INVALID_VERSION) {
        // Invalid request
        status = BAD_REQUEST;
    } else if (method == INVALID_METHOD) {
        // Unrecognized method
        status = NOT_IMPLEMENTED;
    } else if (path.length() > URI_MAX_LENGTH) {
        // Request URI too large
        status = REQUEST_URI_TOO_LARGE;
    } else {
        // Handle each HTTP method
        if (method == GET) {
            // Get URI resource by opening file
            response = HandleGet(request, status);
        } else {
            // Unimplemented methods
            cout << "Not implemented yet\n";
            status = NOT_IMPLEMENTED;
        }
    }

    // Send response
    if (verbose) {
        cout << endl << "Response: " << response << endl << endl;
    }
    return response;
}

string 
HttpServer::HandleGet(HttpRequest request, http_status_t status) {
    // File streams for reading
    fstream file;

    // String variables for file streams and the HTTP response
    string body = "";
    string path = request.get_path();
    string type = request.get_content_type();
    string copy = request.get_copy();
    string response = "";
    
    // Stat - used to check if is a Directory and showList.

    // HTTP request info
    http_method_t method = request.get_method();

    // Misc values
    int index = 0;
    int c = 0;
    
    cout << "PATH ==> " << path << endl;


    if (IsDirectory(path)) {
        body = CreateIndexHtml(path);
    }
    else {
        // Open file and try to fulfill request
        
        file.open(path, fstream::in);
        if (!file.good()) {
            perror("fstream::open");
            status = NOT_FOUND;
        }

        if (method == GET && status == OK) {           
                // Get all characters in file
                c = file.get();
                while (c != EOF && index <= BODY_LENGTH) {
                    body += (char) c;
                    index++;
                    c = file.get();
                }           
        }
        file.close();        
    }

    response = CreateResponseString(request, response, body, status);

    return response;
}

bool
HttpServer::IsDirectory(const string path)
{
    struct stat s;
    if (stat(path.c_str(), &s) == 0)
        if (s.st_mode & S_IFDIR)
            return true;
    
    return false;
}

string
HttpServer::CreateIndexHtml(const string path)
{

    string   response = "<!doctype html><html><head>";
            response += "<title>Index of /" + path  + "</title>";
            response += "</head><body>";
            response += "<h1>Index of " + path + "</h1>";
            response += CreateIndexList(path);
            response += "</body>";
            response += "</html> ";

    return response;

}

string
HttpServer::CreateIndexList(const string actualFullPath)
{

    // Get Files and Directories
    DIR *dirObj;
    struct dirent *node;

    dirObj = opendir(actualFullPath.c_str());

    vector<string> files;
    vector<string> directories;

    // read entire directory
    if (dirObj != NULL)
    {
        while( (node = readdir(dirObj)) ) {
            if (node->d_type == IS_FILE_FLAG) {
                files.push_back( string(node->d_name) );
            }
            else {
                directories.push_back( string(node->d_name) );
            }
        }
    }
    // close directory
    closedir(dirObj);

    // remove dot links
    //directories.erase( std::remove(directories.begin(), directories.end(), string("..")), directories.end() );
    directories.erase( std::remove(directories.begin(), directories.end(), string(".")), directories.end() );


    // sort files by name
    sort(files.begin(), files.end());
    // sort directories by name
    sort(directories.begin(), directories.end());

    // response
    string response;
    string fileFormatted = "";
    string dirName = "";

    // get current path to make an url.
    string urlPath = regex_replace(actualFullPath, regex("\\www"), "");

    // remove last character "null"
    if (urlPath.back() == '\0') {
        urlPath.pop_back();
    }



    // add directories to response
    for (string dir : directories)
    {
        // add last trailing slash to navigate between directories
        if (urlPath.back() != '/') {
            urlPath = urlPath + "/";
        }

        if (dir.compare("..") == 0) {
           fileFormatted = urlPath + "../";
           dirName = "Parent directory";
        }
        else {
            dirName = dir;
            fileFormatted = urlPath + dir;
        }

        response += "<a href='" + fileFormatted + "'>"+ dirName + "</a><br/>\n";
    }
    
    response += "<ul>"; 
    for (string file : files)
    {
        response += "<li>";
        fileFormatted = urlPath + file;
        response += "<a href='" + fileFormatted + "'>"+ file + "</a>\n";
        response += "</li>";
    }
    response += "</ul>";

    return response;
}




string 
HttpServer::CreateResponseString(HttpRequest request, string response, string body, http_status_t status) {
    // Time structs for GMT time
    time_t now;
    struct tm* gmnow;

    // Request fields
    int contentlen = body.length() == 0 ? 0 : body.length() - 1;
    http_method_t method = request.get_method();
    //http_version_t version = ;
    string type = request.get_content_type();

    // Create a new response
    string newresponse = response;

    // Get GMT time
    time(&now);
    gmnow = gmtime(&now);

    // Append status line and body to buffer
    newresponse += HTTPSERVER_VERSION;
    newresponse += SPACE;
    newresponse += statuses[status];
    newresponse += CRLF;

    // For GET only
    if (status == OK && method == GET) {
        newresponse += ACCEPT_RANGES;
        newresponse += BYTES;
        newresponse += CRLF;
        newresponse += CONTENT_TYPE;
        newresponse += type;
        newresponse += CRLF;
        newresponse += CONTENT_LENGTH;
        newresponse += std::to_string(contentlen);
        newresponse += CRLF;
    }
    // Add time and body
    newresponse += DATE;
    newresponse += asctime(gmnow);
    newresponse += CRLF;
    newresponse += body;
    return newresponse;
}


http_method_t 
HttpServer::GetMethod(const string method) {
    if (method.compare("GET") == 0) {
        return GET;
    } else if (method.compare("POST") == 0) {
        return POST;
    } else if (method.compare("PUT") == 0) {
        return PUT;
    } else if (method.compare("DELETE") == 0) {
        return DELETE;
    }
    return INVALID_METHOD;
}

/*
Funcao para verificar se a versao do HTTP client eh 1.1
*/
http_version_t 
HttpServer::GetVersion(const string version) {
    if (version.compare("HTTP/1.1") == 0) 
        return ONE_POINT_ONE;

    return INVALID_VERSION;
}



/*Funcao que busca o mimeType no vector*/
string 
HttpServer::GetMimeType(string extension) {

    /* iterator p/ buscar mimetype */
    map<string, string>::iterator it;
    it = MimeTypes.find(extension);
    /* conteudo padrao, p/ caso nao tenha na lista de mimetypes */
    string contentType = "unknown";


    if(it != MimeTypes.end())
            contentType = (*it).second;
    return contentType;
}

void 
HttpServer::ParseUri(string& uri, string& path, string& query, string& type) {
    int relpath = uri.find(PREVDIR);
    int length;
    int i = 0;
    string hex = "";
    string extension = "";
    char c;

    // Sanitize string, checking for weird relative paths such as "/.."
    // "/." is ok
    while (relpath != (int) string::npos) {
        uri.erase(relpath, strlen(PREVDIR));
        relpath = uri.find(PREVDIR);
    }
    length = uri.length();
    
    // Get path right before query, while sanitizing unsafe ascii characters
    // "%HEX" is interpreted as the character with ascii value HEX
    // "+" is interpreted as a space character
    while (i <= length && uri[i] != '?') {
        if (uri[i] == '%') {
            // Unsafe ascii conversion
            i++;
            hex += uri[i];
            i++;
            hex += uri[i];
            c = hexToAscii(hex);
        } else if (uri[i] == '+') {
            // Space
            c = ' ';
        } else {
            // Safe ascii
            c = uri[i];   
        }
        path += c;
        i++;
    }

    // Skip past '?'
    i++;

    // Get query
    while (i <= length) {
        query += uri[i];
        i++;
    }

    // Get type from path
    i = path.length();
    while (i > 0 && path[i] != '.') {
        i--;
    }
    i++;
    while (i <= (int) path.length() && !isspace(path[i]) && path[i] != (char) NULL) {
        extension += path[i];
        i++;
    }

    cout << "call extension " << endl;

    // Interpret MIME type using extension string
    type = GetMimeType(extension);
}

////////////////////////////////////////////////
//              HttpRequest                   //
////////////////////////////////////////////////
HttpRequest::HttpRequest(http_method_t method, http_version_t version, string copy, string path, string query, string type) {
    Initialize(method, version, copy, path, query, type);
}

HttpRequest::HttpRequest() {
    Initialize(INVALID_METHOD, INVALID_VERSION, "", "", "", "");
}

void HttpRequest::Initialize(http_method_t method, http_version_t version, string copy, string path, string query, string type) {
    // Call parent initialization
    toolong = false;
    this->method = method;
    this->version = version;
    this->copy = copy;
    this->path = path;
    this->query = query;
    this->type = type;
}

void HttpRequest::ParseHeaders(const char* buffer, int index) {
    int i = index;
    int length = strlen(buffer);
    string name = "";
    string value = "";
    const Header* header;

    while (i <= length) {
        // Get name of header
        while (i <= length && buffer[i] != ':') {
            name += buffer[i];
            i++;
        }
        // Increment twice for : 
        i += 2;

        // Get value of header
        while (i <= length && buffer[i] != '\r') {
            value += buffer[i];
            i++;
        }
        // Increment twice for \r\n
        i += 2;
        
        // Add new header struct
        if (!isspace(name.c_str()[0])) {
            header = new Header(name, value);
            headers.push_back(header);
        }
        
        // Reset fields
        name = "";
        value = "";
    }
}

void HttpRequest::Reset() {
    path = "";
    type = "";
    method = INVALID_METHOD;
    version = INVALID_VERSION;
    while (!headers.empty()) {
        delete headers.back();
        headers.pop_back();
    }
}

HttpRequest::~HttpRequest() {}
