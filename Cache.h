#ifndef __CACHE_H__
#define __CACHE_H__
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define MAX_NUMBER 10101010

class Cache {
    
    //private:

    public:
        vector<const Header*> headers;
        string Data;
        unsigned long long int Size;
        //string Path;
        string Url;
        //string HostName;
        clock_t UsageTime;
        unsigned long int fileNumber;
        string HeadResponse;

        Cache();
        Cache(string, string, unsigned long long int);

        // void set_headresponse(string val){ HeadResponse = val };
        // void get_headresponse() { return HeadResponse };
        void add_headers(vector<const Header*>);           
        void print_headers(); 
        vector<const Header*> get_headers() { return headers; }
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

        Cache* getCacheItem(int i);

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



