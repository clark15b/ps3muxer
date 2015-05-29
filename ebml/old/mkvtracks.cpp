#include "ebml.h"
#include <stdio.h>


int main(int argc,char** argv)
{
    fprintf(stderr,"mkvtracks 1.00 Simple MKV parser\n\nCopyright (C) 2009 Anton Burdinuk\n\nclark15b@gmail.com\nhttp://code.google.com/p/tsdemuxer\n\n");

    if(argc<2)
    {
        fprintf(stderr,"USAGE: ./mkvtracks filename.mkv\n");
        return 0;
    }

    try
    {
        ebml::init();

        ebml::stream stream(argv[1]);

        ebml::doc m;

        ebml::parse(&stream,m);

        if(strcasecmp(m.doc_type.c_str(),"matroska"))
            throw(ebml::exception("not a matroska"));

//        if(m.version!=1 && m.read_version!=1)
//            throw(ebml::exception("unknown matroska version"));

        for(std::map<u_int32_t,ebml::track>::const_iterator i=m.tracks.begin();i!=m.tracks.end();++i)
        {
            const ebml::track& t=i->second;
            printf("track=%i, codec=%s, lang=%s, delay=%ims\n",t.id,t.codec.c_str(),t.lang.c_str(),t.timecode==-1?0:t.timecode);
        }
    }
    catch(const std::exception& e)
    {
        fprintf(stderr,"%s\n",e.what());
    }

    return 0;
}
