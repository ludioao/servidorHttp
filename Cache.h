#ifndef __CACHE_H__
#define __CACHE_H__
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define MAX_NUMBER 10101010

class Cache {
    public:
        Cache();
        Cache(string, string, unsigned long long int);
        string Data;
        unsigned long long int Size;
        //string Path;
        string Url;
        //string HostName;
        clock_t UsageTime;
        unsigned long int fileNumber;

        void setNumber(const unsigned long int val) { fileNumber = val; };
        unsigned long int getNumber() { return fileNumber; };
        string getData() { return Data; };
        string getUrl() { return Url; };


};

class CacheService {
    
    private:
        // .. cacheList.
        vector<Cache*> cacheList;

    public:
        // ... Cache search
        Cache getCache(string, string);        

        string getCacheDataByIndex(int i);

        string getCachePath(unsigned long int cacheNumber);

        // ... Cache delete.
        void deleteCache(int index);
        // ... Cache save.
        void storeCache(Cache*);
        // void findCache();
        bool alreadyInCache(string, string);        

        long int getCacheSize();
        unsigned long int getCacheIndex(string);

};

#endif



