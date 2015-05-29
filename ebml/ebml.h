#ifndef __EBML_H
#define __EBML_H

#include <stdexcept>
#include <string>
#include <map>
#include <stdio.h>

#ifndef _WIN32
#include <sys/types.h>
#else
typedef unsigned char           u_int8_t;
typedef unsigned short          u_int16_t;
typedef unsigned long           u_int32_t;
typedef unsigned long long      u_int64_t;
#endif


namespace ebml
{
    enum { timecode_limit=60000, max_reframes=30 };

    class exception : public std::exception
    {
    protected:
        std::string _what;
    public:
        explicit exception(const std::string& s):_what(s) {}
        virtual ~exception(void) throw() {}
        virtual const char* what(void) const throw() {return _what.c_str();}
    };

    class frames_cache
    {
    protected:
        u_int32_t frames[max_reframes];

        int nframes;
    public:
        frames_cache(void) { clear(); }

        void clear(void);

        u_int32_t insert(u_int32_t n);

    };

    enum track_type { tt_unk=0, tt_video=1, tt_audio=2, tt_sub=3 };

    struct track
    {
        u_int32_t id;
        std::string codec;
        std::string lang;
        int type;
        float fps;              // only for type=tt_video
        u_int32_t compression;
        

        u_int32_t start_timecode;

        frames_cache frames;

        track(void):id(0),start_timecode(-1),fps(0),type(tt_unk),compression(-1) { frames.clear(); }
    };

    class file
    {
    protected:
        FILE* fp;
        u_int32_t cluster_timecode;

        u_int32_t track_id;
        std::string track_codec;
        std::string track_lang;
        u_int32_t track_duration;
        u_int32_t track_compression;

        u_int64_t tell(void) throw(std::exception);
        void seek(u_int64_t offset,int whence) throw(std::exception);
        u_int32_t gettag(void) throw(std::exception);          // 0 - eof
        u_int64_t getlen(void) throw(std::exception);
        std::string getstring(int len) throw(std::exception);
        u_int32_t getint(int len) throw(std::exception);

        int parse(std::map<u_int32_t,track>& dst,int depth) throw(std::exception);
    public:
        file(void):fp(0),cluster_timecode(0),track_id(0),track_duration(0),track_compression(-1) {}
        ~file(void) { close(); }

        int open(const char* filename) throw();
        void close(void) throw();

        int parse(std::map<u_int32_t,track>& dst) throw(std::exception);

    };
}


#endif
