#ifndef __EBML_H
#define __EBML_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32 
#include <unistd.h>
#else
#include <io.h>
#include <fcntl.h>
#endif
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <map>
#include <string>

#ifndef _WIN32
#define O_BINARY 0
#else
typedef unsigned char           u_int8_t;
typedef unsigned short          u_int16_t;
typedef unsigned long           u_int32_t;
typedef unsigned long long      u_int64_t;
#endif

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

//#ifdef _WIN32
#ifdef _MSC_VER
inline int strcasecmp(const char* s1,const char* s2) { return lstrcmpiA(s1,s2); }
#endif


namespace ebml
{
    class exception : public std::exception
    {
    protected:
        std::string _what;
    public:
        explicit exception(const std::string& s):_what(s) {}
        virtual ~exception(void) throw() {}
        virtual const char* what(void) const throw() {return _what.c_str();}
    };


    class track
    {
    public:
        u_int32_t   id;
        std::string codec;
        std::string lang;
        u_int32_t   timecode;
        u_int32_t   duration;

        track(void):id(0),timecode(-1),duration(0) {}
    };

    class doc
    {
    public:
        std::string doc_type;
        u_int8_t    version;
        u_int8_t    read_version;

        std::map<u_int32_t,ebml::track> tracks;
        u_int32_t   blocks;

        doc(void):version(0),read_version(0),blocks(0) {}
    };


    class stream_base
    {
    public:
        virtual int read(char* p,int l)=0;
        virtual int skip(u_int64_t l)=0;
        int getTag(u_int32_t& tag);
        int getLen(u_int64_t& len);
    };


    class stream : public stream_base
    {
    protected:
        int fd;

        enum { max_buf_len=2048 };

        char buf[max_buf_len];

        int len,offset;
    public:
        stream(void):fd(-1),len(0),offset(0) {}
        stream(const char* path):fd(-1),len(0),offset(0) { open(path); }
        ~stream(void) { close(); }

        bool open(const char* name);
        void close(void);
        int isOpened(void) { return fd==-1?0:1; }

        virtual int read(char* p,int l);
        virtual int skip(u_int64_t l);
    };


    class segment : public stream_base
    {
    protected:
        stream_base* source;
        u_int64_t length;
    public:
        segment(stream_base* s,u_int64_t len):source(s),length(len) {}

        virtual int read(char* p,int l)
        {
            if(l>length)
                l=length;

            int n=source->read(p,l);

            length-=n;

            return n;
        }

        virtual int skip(u_int64_t l)
        {
            if(l>length)
                l=length;

            if(source->skip(l))
                return -1;

            length-=l;

            return 0;
        }
    };

    void init(void);
    void parse(ebml::stream_base* s,doc& m) throw(std::exception);
}

#endif

