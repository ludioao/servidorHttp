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
    //this->HostName.clear();
    //this->Path.clear();
    this->Size = 0;
    this->UsageTime = clock();
}

Cache::Cache(string Url, string Data, unsigned long long int Size)
{
    this->Data = Data;
    this->Url = Url;
    this->Size = Size;
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
    unsigned long int cacheNumber = cacheList.size() + 1;
    cacheable->setNumber(cacheNumber);
    string filePath = this->getCachePath(cacheNumber);

    ofstream cacheFile;
    cacheFile.open(filePath);
    cacheFile << cacheable->getData();
    cacheFile.close();

    cacheList.push_back(cacheable);
}


unsigned long int
CacheService::getCacheIndex(string url)
{
    unsigned long int index = MAX_NUMBER;
    for (unsigned int i = 0; i < cacheList.size(); i++)
    {   
        if (cacheList[i]->getUrl().compare(url) == 0) 
        {
            cout << "found at " << i << endl;
            index = cacheList[i]->getNumber();
            break;
        }
    }
    
    cout << "index is " << index << endl;

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