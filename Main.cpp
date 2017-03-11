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
    bool showLog = true;

    long int runPort = PORT;

    int 
            numberofThreads = 2,
            argPort = -1,
            argThread = -1;


    if (argc > 1) {

            argPort = 2;
            argThread = 1;
            numberofThreads = atoi(argv[argThread]);
            if (argc > 2) 
                runPort = atoi(argv[argPort]);        
    }

    HttpServer HttpdObj;

    HttpdObj.Start(showLog, runPort, numberofThreads);
    
    return 0;
}
