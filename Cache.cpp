#include <iostream>
#include <time.h>
#include <fstream>
#include "HttpRequest.h"
#include "Cache.h"


using namespace std;

/*
* Cache Struct
*/

Cache::Cache()
{
    this->Data.clear();
    this->Url.clear();
    this->HeaderAll.clear();
    //this->HostName.clear();
    //this->Path.clear();
    this->Size = 0;
    this->UsageTime = clock();
}

Cache::Cache(string Url, string Data, unsigned long long int Size)
{
    this->Data = Data;
    this->Url = Url;
    this->HeaderAll = "";
    this->Size = Size;
    this->UsageTime = clock();
}


Cache::~Cache()
{
    cout << Url << " destruido";
    while (!headers.empty()) {
        delete headers.back();
        headers.pop_back();
    }
}



string 
CacheService::createResponseString(int index)
{   
    
    time_t now;
    struct tm* gmnow;
    
    int contentlen = 0;
    
    // Cria uma nova resposta.
    string newresponse = "";

    // retorna o tempo GMT
    time(&now);
    gmnow = gmtime(&now);

    // prepara resposta
    // Este programa esta apenas funcionando com metodos GET.
    string body = "";
    
    for (unsigned int i = 0; i < cacheList.size(); i++) {
        
        // percorre cachelist
        if (cacheList[i]->getNumber() == index) {

          // pega os dados.
          body = getCacheDataByIndex(i+1); 
          
          // preenche headers.
          if (cacheList[i]->headers.size() > 0) {
                
                for(auto head : cacheList[i]->headers)
                {
                        if (head->name.compare("Content-Length") == 0) continue;
                        if (head->name.compare("Date") == 0) continue;
                        if (head->value.length() == 0)
                        {
                            newresponse += head->name;
                            newresponse += "\r\n";
                        }
                        else {
                            newresponse += head->name + ": ";
                            //ltrim(head->value);
                            newresponse += head->value;
                            newresponse += "\r\n";
                        }              
                }

          }
        }

        

    }
    

    contentlen = body.length() == 0 ? 0 : body.length()-1;

    newresponse += "Content-Length: ";
    newresponse += std::to_string(contentlen);
    newresponse += "\r\n";
    newresponse += "Date: ";
    newresponse += asctime(gmnow);
    newresponse += "\r\n";

    //console_log("::: BEGIN RESPONSE HEADER :::");
    //console_log(newresponse);
    //console_log("::: END HEADER :::");

    newresponse += body;
    return newresponse;
}


/*
* Cache Service
*/
/*Cache
CacheService::getCache(string host, string path)
{
    return new Cache();   
}*/


string
CacheService::getCachePath(unsigned long int cacheNumber)
{
    return "./cache/" + std::to_string(cacheNumber) + ".cache";
}

void 
CacheService::storeCache(Cache* cacheable)
{
    /*unsigned long int cacheNumber = cacheList.size() + 1;
    cacheable->setNumber(cacheNumber);
    string filePath = this->getCachePath(cacheNumber);

    ofstream cacheFile;
    cacheFile.open(filePath);
    cacheFile << cacheable->getData();
    cacheFile.close();
*/
    cacheList.push_back(cacheable);
}


void 
CacheService::getUrlCaches()
{
    for (unsigned int i = 0; i < cacheList.size(); i++)
    { 
        cout << "URL at " << i << " is " << cacheList[i]->getUrl() << endl;
    }
}


unsigned long int
CacheService::getCacheIndex(string url)
{
    unsigned long int index = MAX_NUMBER;
    
    for ( auto cache : cacheList)
    {
        if (cache->getUrl().compare(url) == 0) {
            index = cache->getNumber();
            cout << "Index for this url is: " << index;
            break; 
        }        
    }
    return index;
}

string
CacheService::getCacheDataByIndex(int i)
{
    string filePath = this->getCachePath(i);
    
    cout << "opening file .. " << filePath << endl;
    
    ifstream file(filePath);
    string str;
    string file_contents;
    while (getline(file, str))
    {
        file_contents += str;
        file_contents.push_back('\n');
    }  

    cout <<"read from cache"<<endl;
    return file_contents;
}

Cache* 
CacheService::getCacheItem(int i)
{
    return cacheList[i];
}



void 
Cache::add_headers(vector<const Header*> myHeaders)
{
    headers = myHeaders;
    string newresponse = "";
      for(auto head : myHeaders)
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
          HeaderAll = newresponse;

    // while(!myHeaders.empty()) {
    //     headers.push_back(myHeaders.back());
    //     delete myHeaders.back();
    //     myHeaders.pop_back();
    // }
}            


void
Cache::print_headers()
{
    for(auto head : headers)
    {
        cout << "Cacheable Header ["   
             << head->name << ": " << head->value + "]" 
             << endl;           
    }
}

