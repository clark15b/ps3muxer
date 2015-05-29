#/bin/bash

DST=./ps3muxer_src

mkdir -p $DST/ebml/
mkdir -p $DST/icons/

cp ebml/ebml.* $DST/ebml/
cp icons/*.png $DST/icons/
cp icons/*.ico $DST/icons/

cp execwindow.cpp $DST/
cp execwindow.h $DST/
cp execwindow.ui $DST/
cp main.cpp $DST/
cp mainwindow.cpp $DST/
cp mainwindow.h $DST/
cp mainwindow.ui $DST/
cp ps3muxer.cfg $DST/
cp ps3muxer.iss $DST/
cp ps3muxer.pro $DST/
cp ps3muxer.qrc $DST/
cp ps3muxer.rc $DST/
cp ps3muxer_ru.ts $DST/
cp ps3muxer_win32.cfg $DST/
cp version.h $DST/
cp rm.cpp $DST/
cp ../LICENSE $DST/

tar c $DST | gzip -c > $DST.tar.gz

rm -rf $DST