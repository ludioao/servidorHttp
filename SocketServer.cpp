#include "SocketServer.h"
#include "HttpServer.h"

// Nova instancia do SocketServer
SocketServer::SocketServer()
{
    cout << "Instanciando SocketServer " << endl;
}

// Inicializando o socket
void
SocketServer::Init() {
    
    int error = 0,
        flags = 0;

    // Inicializar buffer e enderecos de sockets.
    memset(recvbuf, (char) NULL, sizeof(recvbuf));
    memset(peername, (char) NULL, sizeof(peername));
    memset(&serveraddr, (char) NULL, sizeof(serveraddr));
    memset(&clientaddr, (char) NULL, sizeof(clientaddr));

    // Cria o socket e vincula ao endereco local
    listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening < 0) {
        perror("Socket error: Falha no bind.");
        close(listening);
        exit(EXIT_FAILURE);
    }

    
    // Inicializa o socket em modo de Reuso e com opcoes nao-bloqueantes
    setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, NULL, 0);
    flags = fcntl(listening, F_GETFL, 0);
    fcntl(listening, F_SETFL, flags | O_NONBLOCK);

    // Informacao do Socket
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(getPortNumber());

    //cout << "serveraddr.sin_port => " << serveraddr.sin_port <<  endl;
    // Mensagem inicializando; ;

    // Bind socket to a local address
    // Vincula o socket p/ um endereco local
    error = bind(listening, (const struct sockaddr *) &serveraddr, sizeof(serveraddr));
    if (error < 0) {
        perror("Socket error: Fail to bind server to local host address.");
        close(listening);
        exit(EXIT_FAILURE);
    }

    // Escuta as conexoes.
    error = listen(listening, BACKLOG);
    if (error < 0) {
        perror("Socket error: Fail to listen to connections."); // 
        close(listening); // Finaliza o socket de escuta.
        exit(EXIT_FAILURE);
    }


}

SocketServer::~SocketServer() {
    // Finaliza o socket de escuta.
    close(listening);
}

// Funcao helper para setar a porta.
void 
SocketServer::setPortNumber(long int number)
{
    portNumber = number;    
}

// Recuperar numero da porta.
long int
SocketServer::getPortNumber()
{
    return portNumber;
}

// Iniciar conexao do socket.
std::pair<int, string> SocketServer::Connect() {
    
    // Recebe qualquer requisicao.
    socklen_t length = sizeof(clientaddr);
    int error = 0;
    int currentConnection = accept(listening, NULL, NULL);
    string peer = "";

    if (currentConnection > 0) {
        // Recebe as informacoes da conexao.
        error = getpeername(currentConnection, (struct sockaddr *)&clientaddr, &length);
        if (error < 0) {
            perror("Socket Error: Fail to get name of connected peer socket.");
            memset(peername, (char) NULL, sizeof(peername));
        } else {
            // converter o endereco ip p/ texto.
            inet_ntop(AF_INET, &(clientaddr.sin_addr), peername, length);
            peer = string(peername);
        }
    }
    return make_pair(currentConnection, peer);
}

// Socket comeca a receber as requisicoes.
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