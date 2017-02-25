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
#include <stdlib.h>
#include "SocketServer.h"
#include "HttpServer.h"
#include "HttpRequest.h"


using namespace std;


int main(int argc, char* argv[]) 
{

    // Por padrao, o servidor inicia em modo de processo.
    tipoServidor type = MPROCESS;
    
    bool showLog = true;

    long int runPort = PORT;

    int 
            numberofThreads = 2,
            argOperationType = 1,
            argPort = -1,
            argThread = -1;


    if (argc > 1) {
        
        // Verifica tipo de operacao.
        if (strcmp(argv[argOperationType], "-f") == 0)
        {
            // Porta esta no segundo argumento
            argPort = 2;
            type = MPROCESS;
            if (argc > 2) {
                runPort = atoi(argv[argPort]);
            }            
        }
        else if (strcmp(argv[argOperationType], "-t") == 0)
        {
            argPort = 3;
            argThread = 2;
            type = MTHREADED;
            if (argc > 2) 
                numberofThreads = atoi(argv[argThread]);
            
            if (argc > 3) {
                runPort = atoi(argv[argPort]);
            }

        } 
        else 
        {
            cout << "Modo nao conhecido. Por favor, utilize -f para processos, e -t para threads. " << endl;
        }

    }

    HttpServer HttpdObj;

    HttpdObj.Start(type, showLog, runPort, numberofThreads);
    
    return 0;
}
