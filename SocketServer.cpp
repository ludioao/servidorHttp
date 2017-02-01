#include "SocketServer.h"
#include "HttpServer.h"

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