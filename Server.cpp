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

#include "HttpRequest.h"
#include "HttpServer.h"

using namespace std;

static bool running = true;

const unsigned char IS_FILE_FLAG = 0x8;

/*
    Helper para Controle de Signals.
*/
void handleSigint(int signum) {
    // Finaliza o loop.
    running = false;
}

void handleSigchld(int signum) {
    // Previne processos zombies.
    int status;
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


/*
Tipo de Servidor;
Verbose => Usado para debugar;
Port_Operacao => Porta para iniciar Operacao;
numberThreads => Quando o tipo de servidor eh Multithread. Entao utiliza-se o numero de threads fixo.
*/
void 
HttpServer::Start(tipoServidor type, bool verbose, long int port_Operacao, int numberThreads) {
    
    signal(SIGINT, handleSigint); // Envia sinal de inicializacao. 
    signal(SIGCHLD, handleSigchld); // Envia sinal p/ processo filho.

    // Porta default
    server.setPortNumber(port_Operacao);
    server.Init();

    long int actualPortNumber = server.getPortNumber();
    
    cout << "Inicializando servidor na porta " << actualPortNumber << endl;
    cout << "Modo de operacao:";

    // Recebe o tipo de arquivo.
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

/*
Inicia o servidor em modo 
de MultiProcesso.
*/
void 
HttpServer::RunMultiProcessed(bool verbose) {
    pid_t pid;
    pair<int, string> client;
    int actualConnection;
    
    if (verbose) {
        cout << "Server inicializando...\n\n";
        cout << "Portal atual: " << server.getPortNumber() << endl;
    }

    // Evento de loop.
    while (running) {

        // Aguarda por uma conexao.
        client = server.Connect();
        actualConnection = client.first;

        if (actualConnection > 0) {
            // Executa fork de um novo processo filho p/ receber a conexao cliente.
            pid = fork();
            if (pid < 0) {
                perror("Http Server: Fail to fork child.");
                continue;
            } else if (pid == 0) {
                // Processo filho
                DispatchRequestToChild(verbose, client);
            } else {
                // Voltando p/ processo pai. Finaliza a conexao.
                server.Close(actualConnection);
            }
        }
        
        // processo fica em stand-by
        usleep(SLEEP_MSEC);
    }
    if (verbose) {
        cout << "Servidor desligando\n";
    }
}



/*Despacha a requisicao pro processo filho.*/
void 
HttpServer::DispatchRequestToChild(bool verbose, pair<int, string> client) {
    HttpRequest request;
    string response;
    time_t begin, end; // definicao de tmepo da requisicao.
    int actualConnection = client.first;

    // Finaliza processo quando o tempo passado excede o tempo limite
    time(&begin);    time(&end);
    elapsedtime = difftime(end, begin);

    do {
        if (server.Receive(verbose, client)) { 
            // Recebe request e envia resposta.
            ParseRequest(request, verbose, server.get_buffer());
            response = HandleRequest(request, verbose);
            server.SendResponse(response, actualConnection);
        }
        // Calcula o tempo passado.
        time(&end);
        elapsedtime = difftime(end, begin);
    } while (elapsedtime < TIME_OUT);

    // Fecha conexao e finaliza o processo.
    server.Close(actualConnection);
    exit(EXIT_SUCCESS);
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
        if (actualConnection > 0 && (int) threadList.size() <= server.getMaxThreads()) {
            
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

    do {
        if (server.Receive(verbose, client)) { 
            // Recebe a requisicao e responde.
            ParseRequest(*request, verbose, server.get_buffer());

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
HttpServer::ParseRequest(HttpRequest& request, bool verbose, const char* recvbuf) {
    int i = 0;
    http_method_t method;
    http_version_t version;
    string  buffer = "",
            path = "",
            query = "",
            type = "",
            uri = "";
    
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
    }

    // captar URI (endereco de arquivo.)
    while (i <= BUFFER_LENGTH && !isspace(recvbuf[i])) {
        uri += recvbuf[i];
        i++;
    }
    path = DIRECTORY;

    // parsear a url recebida e remover caracteres especiais.
    ParseUri(uri, path, query, type);
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
    }

    // Preenche a struct de request.
    request.Initialize(method, version, copy, path, query, type);
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
    
    // stat, usado p/ verificar se o arquivo eh uma pasta e mostrar a lista de arquivos.

    // informacao do request http
    http_method_t method = request.get_method();

    // variaveis temporarias;
    int index = 0, c = 0;
    
    cout << "PATH ==> " << path << endl;

    if (IsDirectory(path)) {
        body = CreateIndexHtml(path);
    }
    else {
        
        // Abre o arquivo.
        file.open(path, fstream::in);
        if (!file.good()) {
            perror("Server Error: Fail to open file.");
            status = NOT_FOUND;
        }

        if (method == GET && status == OK) {           
                
                // parsea todos os caracteres do arquivo
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

    // leia o diretorio inteiro.
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
    
    // fecha a pasta.
    closedir(dirObj);

    // remove os links de recursao.
    directories.erase( std::remove(directories.begin(), directories.end(), string(".")), directories.end() );

    // ordena os arquivos pelo nome
    sort(files.begin(), files.end());
    // ordena os diretorios pelo nome
    sort(directories.begin(), directories.end());

    // prepara a resposta.
    string response;
    string fileFormatted = "";
    string dirName = "";

    // remove o "www" do caminho..
    string urlPath = regex_replace(actualFullPath, regex("\\www"), "");

    // remove o ultimo caractere nulo.
    if (urlPath.back() == '\0') {
        urlPath.pop_back();
    }

    // adiciona as pastas na lista
    for (string dir : directories)
    {
        // adiciona barra no final p/ navegar entre as pastas.
        if (urlPath.back() != '/') {
            urlPath = urlPath + "/";
        }

        // renomear links ".."
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


// Criar response string.
string 
HttpServer::CreateResponseString(HttpRequest request, string response, string body, http_status_t status) {
    
    time_t now;
    struct tm* gmnow;

    int contentlen = body.length() == 0 ? 0 : body.length() - 1;
    http_method_t method = request.get_method();
    string type = request.get_content_type();

    // Cria uma nova resposta.
    string newresponse = response;

    // retorna o tempo GMT
    time(&now);
    gmnow = gmtime(&now);

    // prepara resposta
    newresponse += HTTPSERVER_VERSION;
    newresponse += SPACE;
    newresponse += statuses[status];
    newresponse += CRLF;

    // Este programa esta apenas funcionando com metodos GET.
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
    // adiciona hora e resposta.
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

HttpRequest::HttpRequest(http_method_t method, http_version_t version, string copy, string path, string query, string type) {
    Initialize(method, version, copy, path, query, type);
}

HttpRequest::HttpRequest() {
    Initialize(INVALID_METHOD, INVALID_VERSION, "", "", "", "");
}

void HttpRequest::Initialize(http_method_t method, http_version_t version, string copy, string path, string query, string type) {
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
            header = new Header(name, value);
            headers.push_back(header);
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
