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
        size_t Size;
        //string Path;
        string Url;
        //string HostName;
        clock_t UsageTime;
        unsigned long int fileNumber;
        string HeadResponse;

        string HeaderAll;

        Cache();
        ~Cache();

        Cache(string, string, size_t);

        // void set_headresponse(string val){ HeadResponse = val };
        // void get_headresponse() { return HeadResponse };
        void add_headers(vector<const Header*>);           
        void print_headers(); 
        vector<const Header*> get_headers() { return headers; }
        void setNumber(const unsigned long int val) { fileNumber = val; };
        unsigned long int getNumber() { return fileNumber; };
        string getData() { return Data; };
        string getUrl() { return Url; };
        inline void Reset() {
            Url = "";
            Data = "";
            fileNumber = MAX_NUMBER;
            HeadResponse = "";
            while (!headers.empty()) {
                delete headers.back();
                headers.pop_back();
            }
        }
        string createResponseHeader();
};

class CacheService {
    
    private:
        // .. cacheList.
        vector<Cache*> cacheList;
        size_t cacheLimit;

    public:
        //CacheService();
        //CacheService(size_t limit){ this->cacheLimit = limit; };
        
        // ... Cache search
        Cache getCache(string, string);        

        string getCacheDataByIndex(int i);

        Cache* getCacheItem(int i);

        string getCachePath(unsigned long int cacheNumber);

        string createResponseString(int index);


        // ... Cache delete.
        void deleteCache(int index);
        // ... Cache save.
        void storeCache(Cache*);
        // void findCache();
        bool alreadyInCache(string, string);        

        long int getCacheSize(){ return cacheList.size(); };
        unsigned long int getCacheIndex(string);

        void getUrlCaches();
        void removeFromCache();
        size_t getCacheLimit(){ return cacheLimit; };
        void setCacheLimit(size_t val) { cacheLimit = 0; };

};

#endif



