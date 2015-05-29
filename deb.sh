#!/bin/bash

ROOT=./debian-pkg
DEBIAN=$ROOT/DEBIAN/
CONTENTS=$ROOT/usr/local/ps3muxer/
APP=$ROOT/usr/share/applications/
ICONS=$ROOT/usr/share/pixmaps/

mkdir -p $DEBIAN
mkdir -p $CONTENTS
mkdir -p $APP
mkdir -p $ICONS

cp debian/control $DEBIAN
cp debian/postinst $DEBIAN
cp debian/postrm $DEBIAN

cp debian/ps3muxer.desktop $APP
cp icons/film_go.png $ICONS/ps3muxer.png

cp ps3muxer $CONTENTS
cp ps3muxer.cfg $CONTENTS
cp ps3muxer_ru.qm $CONTENTS
cp tsMuxeR $CONTENTS


fakeroot -- dpkg -b $ROOT debian-pkg.deb
dpkg-name -o debian-pkg.deb

rm -rf $ROOT