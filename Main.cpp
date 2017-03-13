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
#include "Cache.h"
#include "SocketServer.h"
#include "HttpServer.h"
#include "HttpRequest.h"



using namespace std;


int main(int argc, char* argv[]) 
{

    // Por padrao, o servidor inicia em modo de processo.
    bool showLog = true;

    long int runPort = PORT;

    if (argc > 1) {
            runPort = atoi(argv[1]);
            // cacheSize = atoi(argv[2]);
    }

    HttpServer HttpdObj;

    HttpdObj.Start(showLog, runPort);
    
    return 0;
}
