/*
Arquivo principal do servidor HTTP.
*/
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
#include <string>
#include "Functions.h"
#include "HttpRequest.h"
#include "HttpServer.h"

using namespace std;


const unsigned char IS_FILE_FLAG = 0x8;



static bool running = true;


// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}


/*
    Helper para Controle de Signals.
*/
void handleSigint(int signum) {
    #ifdef DEBUG_THREAD
        console_log("Signal is " + signum);
    #endif        
    // Finaliza o loop.
    running = false;
}

void handleSigchld(int signum) {
    // Previne processos zombies.
    int status;
    #ifdef DEBUG_THREAD
        console_log("Signal is " + signum);
    #endif
    while (waitpid(-1, &status, WNOHANG) == 0);
}

/*
    Helper p/ conversao de Hexadecimal p/ ASCII 
*/
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





// Construtor do HttpServer.
HttpServer::HttpServer() {
    elapsedtime = 0.0;
    InitMimeTypes();
}

// Destrutor do HttpServer.
HttpServer::~HttpServer() {
    cout << "Finalizando HTTPServer " << endl;
}

// Definindo MimeTypes.
void
HttpServer::InitMimeTypes() {    
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
    //MimeTypes["jpg"]    = "image/jpeg";
    MimeTypes["png"]    = "image/";    
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


/*
Tipo de Servidor;
Verbose => Usado para debugar;
Port_Operacao => Porta para iniciar Operacao;
numberThreads => Quando o tipo de servidor eh Multithread. Entao utiliza-se o numero de threads fixo.
*/
void 
HttpServer::Start(bool verbose, long int port_Operacao) {
    
    signal(SIGINT, handleSigint); // Envia sinal de inicializacao. 
    signal(SIGCHLD, handleSigchld); // Envia sinal p/ processo filho.

    // Porta default
    server.setPortNumber(port_Operacao);
    server.Init();

    long int actualPortNumber = server.getPortNumber();

    console_log("Inicializando servidor na porta " + actualPortNumber);
    
    // Recebe o tipo de arquivo.
    //server.setMaxThreads(numberThreads);
    RunMultiThreaded(verbose);    
}


void 
HttpServer::RunMultiThreaded(bool verbose) {
    vector<pthread_t> threadList;
    pthread_t newThread;
    mthreaded_request_args threadArgs;
    pair<int, string> client;
    int actualConnection;
    int error;

    // Inicializa tratamento das threads e mutex.
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (verbose) {
        cout << "Server inicializando...\n\n";
    }

    // Loop escutando por cliente.
    while (running) {
        client = server.Connect();
        actualConnection = client.first;

  /*      cout << "threadList  " << threadList.size() << endl;

        cout << "actualConnection passou a ser " << actualConnection << endl;

        cout << "threadList size is " << threadList.size() << endl;
        cout << "maxthreads size is " << getMaxThreads() << endl;
*/
        if (actualConnection > 0) {
            
            // Cria uma nova thread e despacha.
            threadArgs.verbose = verbose;
            threadArgs.client = client;
            threadArgs.ptr = this;

            // Adiciona a Thread em uma lista.
            error = pthread_create(&newThread, &attr, HttpServer::CallDispatchRequestToThread, &threadArgs);
            if (error < 0) {
                perror("HttpServer: THREAD - fail to pthread_create");
            }
            threadList.push_back(newThread);
        }
        // Sleep if no actualConnections
        usleep(SLEEP_MSEC);
    }

    if (verbose) {
        cout << "Servidor desligando\n";
    }

    // Pthread Join em todas as threads finalizadas.
    while (!threadList.empty()) {
        error = pthread_join(threadList.back(), NULL);
        if (error < 0) {
            perror("HttpServer: THREAD - fail to pthread_join");
        }
        threadList.pop_back();
    }

    // Limpa mutex e seus atributos.
    pthread_attr_destroy(&attr);

    // Permite que a thread principal finalize enquanto aguarda as demais.
    pthread_exit(NULL);
}

void* 
HttpServer::DispatchRequestToThread(bool verbose, pair<int, string> client) {
    HttpRequest *request = new HttpRequest;
    string response;
    time_t begin;
    time_t end;
    int actualConnection = client.first;

    // Finaliza o processo quando o tempo de resposta excede o limite.
    time(&begin);
    time(&end);
    elapsedtime = difftime(end, begin);

    Pthread_detach(pthread_self());

    do {
        if (server.Receive(verbose, client)) { 
            // Recebe a requisicao e responde.
            ParseRequest(*request, verbose, server.get_buffer());

            if (request->get_invalid() == true)
                break;

            // Trava a mutex antes de chama-lo.
            response = getThreadRequest(*request, verbose);
            server.SendResponse(response, actualConnection);
        }
        // Calcula o tempo corrido.
        time(&end);
        elapsedtime = difftime(end, begin);
    } while (elapsedtime < TIME_OUT);

    // Fecha a conexao atual e finaliza.
    server.Close(actualConnection);
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
HttpServer::Pthread_detach(pthread_t tid)
{
    int rc;
    if ((rc = pthread_detach(tid)) != 0)
        perror("Pthread_detach error");
}

void 
HttpServer::ParseRequest(HttpRequest& request, bool verbose, const char* recvbuf) {
    int i = 0;
    http_method_t method;
    http_version_t version;
    int port = DEFAULT_REMOTE_PORT;
    string  buffer = "",
            path = "",
            query = "",
            type = "",
            uri = "",
            hostName = "",
            uriRequested = "",
            userAgent = "";
    
    // Mantem uma copia da requisicao original
    string copy = recvbuf;

    // Capta o metodo da requisicao.
    while (i <= BUFFER_LENGTH && !isspace(recvbuf[i])) {
        buffer += recvbuf[i];
        i++;
    }
    i++;

    // Metodo parser.
    method = GetMethod(buffer);
    if (verbose && method == INVALID_METHOD) {
        cout << "Metodo HTTP nao reconhecido.\n";
        request.set_invalid(true);
        return;
    }
    else {
            
            // captar URI (endereco de arquivo.)
            while (i <= BUFFER_LENGTH && !isspace(recvbuf[i])) {
                uri += recvbuf[i];
                i++;
            }
            

            // parsear a url recebida e remover caracteres especiais.
            cout << "Parseando URI" << endl;
            ParseUri(uri, path, query, type, hostName, port);
            i++; 
            if (verbose && path.length() > URI_MAX_LENGTH) {
                cout << "Requisicao muito longa.\n";
            }

            // Get version number
            buffer = "";
            while (i <= BUFFER_LENGTH && !isspace(recvbuf[i])) {
                buffer += recvbuf[i];
                i++;
            }

            // pula pra proxima linha
            while (isspace(recvbuf[i])) 
                i++;
            
            // parsea a versao do http.
            version = GetVersion(buffer);
            if (verbose && version == INVALID_VERSION) {
                cout << "Versao do HTTP invalida..\n";
                request.set_invalid(true);
                return;
            }
            console_log("server requested.....");
            console_log("uri selected => " + uri);
            console_log("Method: " + method);
            console_log("version: " + version);
            console_log("copy: " + copy);
            console_log("path: " + path);
            console_log("query: " + query);
            console_log("hostname: " + hostName);
            console_log("port: " + port);
            console_log(".............................");
            path = uri;

            // Preenche a struct de request.
            request.Initialize(method, version, copy, path, query, type, hostName, port);
    }

   
}

// Wrapper pra receber requisicao do thread.
string 
HttpServer::getThreadRequest(HttpRequest& request, bool verbose) {
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

    console_log("content type is " + type);
 
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
    
    // file string to get.
    fstream file;
    string body = "";
    string path = request.get_path();
    string type = request.get_content_type();
    string copy = request.get_copy();
    string response = "";

    Cache* itemCache;
    
    // stat, usado p/ verificar se o arquivo eh uma pasta e mostrar a lista de arquivos.

    // informacao do request http
    http_method_t method = request.get_method();

    // variaveis temporarias;
    //int index = 0, c = 0;
    
    cout << "PATH ==> " << path << endl;


    //perror("Server Cache: Not found in cache");
    //status = NOT_FOUND;

    status = OK;

    // check if is in cache service.
    if (method == GET && status == OK) 
    {
        // get host name
        string host = request.get_host_name();

        // call for host.
        cout << "Calling for host: " << host << endl;

        // cache url.
        string cacheUrl = host + path;
        console_log("URL is " + cacheUrl);

        unsigned long int index = cache.getCacheIndex(cacheUrl);
        // Retrieve from cache.
        if (index != MAX_NUMBER)
        {
            body = cache.getCacheDataByIndex(index);
            itemCache = cache.getCacheItem(index);

            console_log("cache retrieved >>>");
            console_log(body);
            console_log("<<< cache retrieved");

            console_log("header count is " + itemCache->headers.size());
            

            response = CreateResponseString(request, response, body, status, itemCache);

            //removeHeaderFromContent(request, response);
        }
        else 
        {
            // Download and get file!!!!
            string fileContentRemote = downloadFile(request, host, path, request.get_port());
            body = fileContentRemote.c_str();
            ltrim(body);
            
            //removeHeaderFromContent()
            
            // Save to Cache    
            console_log("Saving to cache...");
            
            removeHeaderFromContent(request, body);

            ltrim(body); 

            console_log("body retrieved >>> ");   
            console_log(body);
            console_log("<<< body retrieved");

            Cache* temporaryCache = new Cache(host + path, body, 1024);
            temporaryCache->add_headers( request.get_headers() );
            cache.storeCache(temporaryCache);
            
            temporaryCache->print_headers();

            response = CreateResponseString(request, response, body, status, temporaryCache);

            
        }    
    }

    return response;
}

void
HttpServer::removeHeaderFromContent(HttpRequest &request, string &content)
{
        int pointer = content.find("\r\n\r\n");
        if (pointer != (int) string::npos)
        {
            string content_header = content.substr(0, pointer);
            request.ParseHeaders(content_header.c_str(), 0);    

            content = content.substr(pointer, content.length());
        }
        
        

        //request.printHeaders();
}



string
HttpServer::downloadFile(HttpRequest &request, string hostName, string uri, int port)
{
    ClientSocket c;

    string headers = "";

    string response = "";
    response.clear();
     
    //connect to host
    c.connectToHost(hostName, port);
     
    //send some data    
    string request_data;
    request_data = "GET " + uri + " HTTP/1.1\n";
    //request_data.append( HDR_ENDLINE );
    request_data.append("Host");
    request_data.append(HDR_DELIMETER);
    request_data.append(hostName);
    request_data.append(HDR_ENDLINE);
    request_data.append("Connection");
    request_data.append(HDR_DELIMETER);
    request_data.append("close");
    request_data.append(HDR_ENDLINE);
    request_data.append(HDR_ENDLINE);

    /*for (auto head : headers)
    {

    }*/
            
    console_log("Sending data to server...");
    console_log(request_data);
    console_log("");

    c.sendDataHost(request_data.c_str());
     
    //receive and echo reply
    response = c.receiveFromHost( );

    // get headers and remove from content.
    //removeHeaderFromContent(request, response);

    return response;

}

// Criar response string.
string 
HttpServer::CreateResponseString(HttpRequest request, string response, string body, http_status_t status, const Cache* item) {
    
    time_t now;
    struct tm* gmnow;
    
    int contentlen = body.length() == 0 ? 0 : body.length() - 1;
    http_method_t method = request.get_method();

    // Cria uma nova resposta.
    string newresponse = response;

    // retorna o tempo GMT
    time(&now);
    gmnow = gmtime(&now);

    // prepara resposta
    // Este programa esta apenas funcionando com metodos GET.
    if (status == OK && method == GET) {
          for(auto head : item->headers)
          {
              if (head->name.compare("Content-Length") == 0) continue;
              if (head->name.compare("Date") == 0) continue;
              if (head->value.length() == 0)
              {
                  newresponse += head->name;
                  newresponse += CRLF;
              }
              else {
                  newresponse += head->name + ": ";
                  //ltrim(head->value);
                  newresponse += head->value;
                  newresponse += CRLF;
              }
              
          }
    }


    newresponse += CONTENT_LENGTH;
    newresponse += std::to_string(contentlen);
    newresponse += CRLF;
    newresponse += DATE;
    newresponse += asctime(gmnow);
    newresponse += CRLF;
    newresponse += body;


    delete item;

    return newresponse;
}


http_method_t 
HttpServer::GetMethod(const string method) {
    if (method.compare("GET") == 0) {
        return GET;
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
HttpServer::ParseUri(string& uri, string& path, string& query, string& type, string &hostName, int &port) {
    int relpath = uri.find(PREVDIR);
    int protocolIndex = uri.find("://");
    int length;
    int i = 0;
    string hex = "";
    string extension = "";
    string tmpProtocol = "";
    string tmphostName;
    string tmpPort = "";
    string tmpUri = "";
    char c;



    /* Verifica protocolo requisitado */
    if (protocolIndex != (int) string::npos) {
        tmpProtocol = uri.substr(0, protocolIndex);
        tmphostName = uri.substr(protocolIndex + 3, uri.length());
        //console_log("Temphostname is " + tmphostName);

        //cout << "Protocolo requisitado " << tmpProtocol << endl;        
        if (! (tmpProtocol.compare("http") == 0 ) ) { // || tmpProtocol.compare("https") 
            perror("Protocolo invalido!");            
        }
    }
    
    int firstBackslash = tmphostName.find("/");

    
    if (firstBackslash != (int) string::npos) {
        hostName = tmphostName.substr(0, firstBackslash);

        tmpUri = tmphostName.substr(firstBackslash, tmphostName.length());
        uri = tmpUri;
        
        // Verifica se tem porta passada explicitamente.
        int portIndex = hostName.find(":");
        if (portIndex != (int) string::npos) {
            tmpPort = hostName.substr(portIndex + 1, hostName.length());
            
            port = stoi(tmpPort);
            console_log("SERVER PARSER: Currently at port " + port);

            hostName = hostName.substr(0, portIndex);
            console_log("SERVER PARSER: Hostname is " + hostName);
        }
    }

    // indice dos ":"
    
    //Remove caracteres, verificando do tipo "/.."
    while (relpath != (int) string::npos) {
        uri.erase(relpath, strlen(PREVDIR));
        relpath = uri.find(PREVDIR);
    }
    length = uri.length();
    
    // Retorna o caminho correto antes de enviar, 
    // tambem remove caracteres especiais q nao estao na tabela ascii.

    // %HEX eh interpretado como um caractere com o valor hexadecimal da tabela Ascii.
    // + eh interpretado como um caractere de espaco
    while (i <= length && uri[i] != '?') {
        if (uri[i] == '%') {
            // Conversao ascii nao segura.
            i++;
            hex += uri[i];
            i++;
            hex += uri[i];
            c = hexToAscii(hex);
        } else if (uri[i] == '+') {
            c = ' '; // Espaco
        } else {
            c = uri[i];   // Ascii ok!!
        }
        path += c;
        i++;
    }

    // pula caractere '?'
    i++;

    // retorna a query
    while (i <= length) {
        query += uri[i];
        i++;
    }

    // pega o tipo do caminho.
    i = path.length();
    while (i > 0 && path[i] != '.') {
        i--;
    }
    i++;
    while (i <= (int) path.length() && !isspace(path[i]) && path[i] != (char) NULL) {
        extension += path[i];
        i++;
    }




    // Interpreta o mimetype usando a extensao
    type = GetMimeType(extension);
}


/*Funcoes do HTTPREQUEST.*/

HttpRequest::HttpRequest(http_method_t method, http_version_t version, string copy, string path, string query, string type,  string hostName, int port) {
    Initialize(method, version, copy, path, query, type, hostName, port);
}

HttpRequest::HttpRequest() {
    Initialize(INVALID_METHOD, INVALID_VERSION, "", "", "", "", "", 80);
}

void HttpRequest::Initialize(http_method_t method, http_version_t version, string copy, string path, string query, string type,  string hostName, int port) {
    toolong = false;
    this->method = method;
    this->version = version;
    this->copy = copy;
    this->path = path;
    this->query = query;
    this->type = type;
    this->host_name = hostName;
    this->port = port;
    this->invalid = false;
}

void HttpRequest::ParseHeaders(const char* buffer, int index) {
    int i = index;
    int length = strlen(buffer);
    string name = "";
    string value = "";
    int asciiN;
    
    const Header* header;

    int iteraction = -1;

    while (i <= length) {
        // Pega o nome do cabecalho.
        while (i <= length && buffer[i] != ':') {
            name += buffer[i];
            i++;
        }
        // incrementa em 2.
        i += 2;

        // retorna o valor do header.
        while (i <= length && buffer[i] != '\r') {
            value += buffer[i];
            i++;
        }
        // pula os caracteres \r\n
        i += 2;
        
    
        // adiciona a nova struct de header.
        if (!isspace(name.c_str()[0])) {

                // parser do primeiro...
                if (++iteraction == 0) {
                    // get first \n
                    asciiN = name.find("\n");
                    string headReply = name.substr(0, asciiN);
                    header = new Header(headReply, "");
                    headers.push_back(header);
                    
                    console_log("headreply is " + headReply);

                    // get date.
                    string dateReply = name.substr(asciiN, name.length());

                    int dateHeadIndex = dateReply.find(":");
                    if (dateHeadIndex != (int) string::npos) {
                        string dateHead = dateReply.substr(0, dateHeadIndex);
                        string dateValue = dateReply.substr(dateHeadIndex, dateReply.length());
                        
                        header = new Header(dateHead, dateValue);
                        headers.push_back(header);
                    }
                }
                else {                  
                        header = new Header(name, value);
                        headers.push_back(header);
                }
       
        }
        
        // Limpa os campos.
        name = "";
        value = "";
    }
}

/*
    reseta o http.
 */
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


void
HttpRequest::printHeaders()
{

            /*for(auto head : headers)
            {
               console_log("header addiciotnal " + head->name + " value "  + head->value);           
            }*/
}


/*
* Client Socket
*/


ClientSocket::ClientSocket()
{
    sock = -1;
    port = 0;
    address = "";
}
 
/**
    Connect to a host on a certain port number
*/
bool ClientSocket::connectToHost(string address , int port)
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
            console_log("Failed to resolve hostname");
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
bool ClientSocket::sendDataHost(string data)
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
string ClientSocket::receiveFromHost()
{
    
    char buffer[BUFFER_LENGTH];
    memset( buffer, '\0', sizeof(char)*BUFFER_LENGTH );
    string reply = "";
  
    while (recv(sock, buffer, sizeof(buffer) + 1, 0))
    {
        reply += buffer;
        console_log(buffer);
        memset( buffer, '\0', sizeof(char)*BUFFER_LENGTH );
    }


    return reply;
}
 











