#include "ebml.h"
#include <string.h>

#ifndef _WIN32
#include <arpa/inet.h>
#else
#include <windows.h>
#endif

#ifdef _MSC_VER
#define fopen64 fopen
#define fseeko64 _fseeki64
#define ftello64 _ftelli64
#endif

#if __APPLE__ & __MACH__
#define fopen64 fopen
#define fseeko64 fseeko
#define ftello64 ftello
#endif


namespace ebml
{
    void frames_cache::clear(void)
    {
        memset((char*)frames,0,sizeof(frames));
        nframes=0;
    }

    u_int32_t frames_cache::insert(u_int32_t n)
    {
        if(!nframes)
            frames[0]=n;
        else
        {
            if(nframes<max_reframes)
            {
                int i;
                for(i=nframes;i>0;i--)
                {
                    if(frames[i-1]<n)
                    {
                        frames[i]=n;
                        break;
                    }else
                        frames[i]=frames[i-1];
                }
                if(!i)
                    frames[0]=n;
            }else
            {
                int i;
                for(i=0;i<max_reframes-1;i++)
                {
                    if(frames[i+1]>n)
                    {
                        frames[i]=n;
                        break;
                    }else
                        frames[i]=frames[i+1];
                }
                if(i>=max_reframes-1)
                    frames[max_reframes-1]=n;
            }
        }
        nframes++;

        if(nframes>=max_reframes)
            return frames[1]-frames[0];

        return 0;
    }

    int file::open(const char* filename) throw()
    {
        cluster_timecode=0;
        track_id=0;
        track_codec.clear();
        track_lang.clear();
        track_duration=0;
        track_compression=-1;

        return (fp=fopen64(filename,"rb"))?0:-1;
    }

    void file::close(void) throw()
    {
        if(fp)
        {
            fclose(fp);
            fp=0;
        }
    }

    u_int64_t file::tell(void) throw(std::exception)
    {
        u_int64_t pos=ftello64(fp);
        if(pos==-1)
            throw(exception("tell(): can't get current file position"));
        return pos;
    }

    void file::seek(u_int64_t offset,int whence) throw(std::exception)
    {
        if(fseeko64(fp,offset,whence)==-1)
            throw(exception("fseek(): can't set file position"));
    }

    u_int32_t file::gettag(void) throw(std::exception)
    {
        int ch=fgetc(fp);

        if(ch==EOF)
            return 0;

        int len=0;

        u_int8_t c;

        for(c=(u_int8_t)ch;!(c&0x80) && len<sizeof(u_int32_t);c<<=1,len++);

        if(!(c&0x80))
            throw(exception("gettag(): invalid ebml element tag length"));

        u_int32_t tag=ch;

        for(int i=0;i<len;i++)
        {
            ch=fgetc(fp);

            if(ch==EOF)
                throw(exception("gettag(): broken file"));

            tag=(tag<<8)|(u_int8_t)ch;
        }

        return tag;
    }


    u_int64_t file::getlen(void) throw(std::exception)
    {
        int ch=fgetc(fp);

        if(ch==EOF)
            throw(exception("getlen(): broken file"));

        int len=0;

        u_int8_t c;

        for(c=(u_int8_t)ch;!(c&0x80) && len<sizeof(u_int64_t);c<<=1,len++);

        if(!(c&0x80))
            throw(exception("getlen(): invalid ebml element length size"));

        ch&=~(0x80>>len);

        u_int64_t length=ch;

        for(int i=0;i<len;i++)
        {
            ch=fgetc(fp);

            if(ch==EOF)
                throw(exception("getlen(): broken file"));

            length=(length<<8)|(u_int8_t)ch;
        }

        return length;
    }


    std::string file::getstring(int len) throw(std::exception)
    {
        char s[256];
        if(len>sizeof(s))
            len=sizeof(s);

        if(fread(s,1,len,fp)!=len)
            throw(exception("getstring(): broken file"));

        return std::string(s,len);
    }

    u_int32_t file::getint(int len) throw(std::exception)
    {
        u_int32_t n=0;

        if(len>sizeof(n))
            throw(exception("getint(): integer too large"));


        if(fread(((char*)&n)+(sizeof(n)-len),1,len,fp)!=len)
            throw(exception("getint(): broken file"));

        return ntohl(n);
    }


    int file::parse(std::map<u_int32_t,track>& dst,int depth) throw(std::exception)
    {
        u_int32_t tag=gettag();

        if(!tag)
            return -1;

        u_int64_t len=getlen();

        u_int64_t next_pos=tell();

        next_pos+=len;

        int rc=0;
        
        switch(tag)
        {
        case 0xa1:              // block
        case 0xa3:
            {
                u_int64_t track=getlen();

                u_int16_t tc=0;

                if(fread((char*)&tc,1,sizeof(tc),fp)!=sizeof(tc))
                    throw(exception("parse(): can't read block timecode"));

                u_int32_t timecode=cluster_timecode+ntohs(tc);

                ebml::track& t=dst[track];

                if(t.start_timecode==-1)
                    t.start_timecode=timecode;

/*
                if(t.type==tt_video)
                {
                    u_int32_t duration=t.frames.insert(timecode);
                    fprintf(stderr,"%i\n",duration);
                }
*/

                if(timecode>timecode_limit)
                    return -1;
            }
            break;
        case 0xa0:              // container
        case 0x1a45dfa3:
        case 0x18538067:
        case 0x1549A966:
        case 0x1654ae6b:
        case 0xae:
        case 0x1f43b675:
        case 0x6d80:
        case 0x6240:
        case 0x5034:
            while(tell()<next_pos && !(rc=parse(dst,depth+1)));

            if(tag==0xae)
            {
                track& t=dst[track_id];
                t.id=track_id; track_id=0;
                t.codec.swap(track_codec); track_codec.clear();
                t.lang.swap(track_lang); track_lang.clear();
                t.compression=track_compression; track_compression=-1;

                const char* p=t.codec.c_str();
                if(!strncmp(p,"V_",2))
                    t.type=tt_video;
                else if(!strncmp(p,"A_",2))
                    t.type=tt_audio;
                else if(!strncmp(p,"S_",2))
                    t.type=tt_sub;

                if(t.type==tt_video)
                    t.fps=1000000000./track_duration;
                track_duration=0;
            }

            break;
        case 0xe7:              // cluster timecode (uint)
            cluster_timecode=getint(len);
            break;
        case 0x86:              // codec (string)
            track_codec=getstring(len);
            break;
        case 0xd7:              // track number (uint)
            track_id=getint(len);
            break;
        case 0x22b59c:          // language (string)
            track_lang=getstring(len);
            break;
        case 0x23e383:          // frame length
            track_duration=getint(len);
            break;
        case 0x4254:            // compression alg
            track_compression=getint(len);
            break;
        }

        seek(next_pos,SEEK_SET);

        return rc;
    }

    int file::parse(std::map<u_int32_t,track>& dst) throw(std::exception)
    {
        while(!parse(dst,0));

        return 0;
    }

}

#ifdef MKVTRACKS
int main(int argc,char** argv)
{
    fprintf(stderr,"mkvtracks 2.00 Simple MKV parser\n\nCopyright (C) 2011 Anton Burdinuk\n\nclark15b@gmail.com\nhttp://code.google.com/p/tsdemuxer\n\n");
    if(argc<2)
    {
        fprintf(stderr,"USAGE: ./mkvtracks filename.mkv\n");
        return 0;
    }

    ebml::file mkv;

    if(!mkv.open(argv[1]))
    {
        try
        {
            std::map<u_int32_t,ebml::track> tracks;

            mkv.parse(tracks);

            for(std::map<u_int32_t,ebml::track>::const_iterator i=tracks.begin();i!=tracks.end();++i)
            {
                const ebml::track& t=i->second;
                printf("track %i: codec=%s, lang=%s, delay=%i",t.id,t.codec.c_str(),t.lang.c_str(),t.start_timecode);

                if(t.type==ebml::tt_video)
                    printf(", fps=%.3f",t.fps);
                if(t.compression!=-1)
                    printf(", compression=%i",t.compression);

                printf("\n");
            }
        }
        catch(const std::exception& e)
        {
            fprintf(stderr,"*** %s\n",e.what());
        }

        mkv.close();
    }else
        fprintf(stderr,"file is not found: %s\n",argv[1]);
        
    return 0;
}
#endif
