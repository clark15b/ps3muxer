#include "ebml.h"
#include <stdio.h>
#include <sys/types.h>

namespace ebml
{
    enum
    {
        t_struct        = 1,
        t_string        = 2,
        t_uint          = 3,
        t_block         = 4
    };

    class element_info
    {
    public:
        u_int8_t type;
        std::string name;
        u_int8_t id;

        element_info(void):type(0),id(0) {}
        element_info(u_int8_t _type,const std::string& _name,u_int8_t _id=0):type(_type),name(_name),id(_id) {}
    };

    std::map<u_int32_t,element_info> semantic;

    void init(void)
    {
        semantic[0x1a45dfa3]    = element_info(t_struct,"EBML");
        semantic[0x4282]        = element_info(t_string,"DocType",1);
        semantic[0x4287]        = element_info(t_uint  ,"DocTypeVersion",2);
        semantic[0x4285]        = element_info(t_uint  ,"DocTypeReadVersion",3);

        semantic[0x18538067]    = element_info(t_struct,"Segment");
        semantic[0x1549A966]    = element_info(t_struct,"Info");
        semantic[0x2AD7B1]      = element_info(t_uint  ,"TimecodeScale",4);
        semantic[0x1654ae6b]    = element_info(t_struct,"Tracks");
        semantic[0xae]          = element_info(t_struct,"TrackEntry");
        semantic[0x86]          = element_info(t_string,"Codec",5);
        semantic[0xd7]          = element_info(t_uint  ,"TrackNumber",6);
        semantic[0x22B59C]      = element_info(t_string,"Language",7);
        semantic[0x23e383]      = element_info(t_uint  ,"DefaultDuration",10);

        semantic[0x1f43b675]    = element_info(t_struct,"Cluster");
        semantic[0xe7]          = element_info(t_uint  ,"Timecode",7);
        semantic[0xa0]          = element_info(t_struct,"BlockGroup");
        semantic[0xa1]          = element_info(t_block ,"Block",9);
        semantic[0xa3]          = element_info(t_block ,"SimpleBlock",9);
    }
}

int ebml::stream_base::getTag(u_int32_t& tag)
{
    tag=0;

    u_int8_t buf[4];

    if(read((char*)buf,1)!=1)
        return 0;

    int l=1;

    u_int8_t p=buf[0];

    while(!(p&0x80) && l<=sizeof(buf))
        { l++; p<<=1; }

    if(!(p&0x80))
        return -1;

    if(read((char*)buf+1,l-1)!=l-1)
        return -1;

    for(int i=0;i<l;i++)
        tag=(tag<<8)+buf[i];

    return l;
}

int ebml::stream_base::getLen(u_int64_t& len)
{
    len=0;

    u_int8_t buf[8];

    if(read((char*)buf,1)!=1)
        return -1;

    int l=1;

    u_int8_t p=buf[0];

    while(!(p&0x80) && l<=sizeof(buf))
        { l++; p<<=1; }

    if(!(p&0x80))
        return -1;

    buf[0]&=(~(0x80>>l-1));

    if(read((char*)buf+1,l-1)!=l-1)
        return -1;

    for(int i=0;i<l;i++)
        len=(len<<8)+buf[i];

    return l;
}


bool ebml::stream::open(const char* name)
{
    close();

    fd=::open(name,O_LARGEFILE|O_BINARY|O_RDONLY,0644);

    if(fd!=-1)
    {
        len=offset=0;
        return true;
    }

    return false;
}


void ebml::stream::close(void)
{
    if(fd!=-1)
    {
        ::close(fd);
        fd=-1;
    }
}

int ebml::stream::read(char* p,int l)
{
    if(fd==-1)
        return 0;

    const char* tmp=p;

    while(l>0)
    {
        int n=len-offset;

        if(n>0)
        {
            if(n>l)
                n=l;

            memcpy(p,buf+offset,n);
            p+=n;
            offset+=n;
            l-=n;
        }else
        {
            int m=::read(fd,buf,max_buf_len);
            if(m==-1 || !m)
                break;
            len=m;
            offset=0;
        }
    }

    return p-tmp;
}

int ebml::stream::skip(u_int64_t l)
{
    int n=len-offset;

    if(n<l)
    {
        l-=n;
        len=offset=0;
        if(lseek(fd,l,SEEK_CUR)==(off_t)-1)
            return -1;
    }else
        offset+=l;

    return 0;
}


void ebml::parse(ebml::stream_base* s,ebml::doc& m) throw(std::exception)
{
    u_int32_t   track_id=0;
    std::string track_codec;
    std::string track_lang;
    u_int32_t   track_duration=0;

    while(m.blocks<256)
    {
        u_int32_t tag;
        u_int64_t len;

        char buf[512];

        std::string string_value;
        u_int32_t   uint_value=0;

        int rc=s->getTag(tag);

        if(!rc)
            break;

        if(rc<0)
            throw(exception("invalid element tag"));

        if(s->getLen(len)<1)
            throw(exception("invalid element length"));

        element_info& info=semantic[tag];

        switch(info.type)
        {
        case t_block:
            if(s->read(buf,3)!=3 || s->skip(len-3))
                throw(exception("unexpected end of file"));
            if(info.id==9)
            {
                track& t=m.tracks[((unsigned char*)buf)[0]&0x7f];
                if(t.id>0 && t.timecode==-1)
                    t.timecode=(((unsigned char*)buf)[1]<<8)+((unsigned char*)buf)[2];
                m.blocks++;
            }
            break;
        case t_string:
            if(len>sizeof(buf))
                throw(exception("string too long"));
            if(s->read(buf,len)!=len)
                throw(exception("unexpected end of file"));
            string_value.assign(buf,len);
            break;
        case t_uint:
            if(len>sizeof(buf) || len>sizeof(uint_value))
                throw(exception("number too long"));
            if(s->read(buf,len)!=len)
                throw(exception("unexpected end of file"));
            uint_value=0;
            for(int i=0;i<len;i++)
                uint_value=(uint_value<<8)+((unsigned char*)buf)[i];
            break;
        case t_struct:
            {
                segment ss(s,len);
                parse(&ss,m);
            }
            break;
        default:
            if(s->skip(len))
                throw(exception("unexpected end of file"));
        }

        switch(info.id)
        {
        case 1:  m.doc_type=string_value; break;
        case 2:  m.version=uint_value; break;
        case 3:  m.read_version=uint_value; break;
        case 5:  track_codec=string_value; break;
        case 6:  track_id=uint_value; break;
        case 7:  track_lang=string_value; break;
        case 10: track_duration=uint_value; break;
        }
    }

    if(track_id>0)
    {
        track& t=m.tracks[track_id];

        t.id=track_id;
        t.codec=track_codec;
        t.lang=track_lang;
        t.duration=track_duration;
    }
}
