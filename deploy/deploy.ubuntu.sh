#!/bin/bash

#	Requires linuxdeployqt: https://github.com/probonopd/linuxdeployqt
#	Installed manually (not with help of QT UI)
#	Also install patchelf
#	And appimagetool

APP_NAME=MezzoForta\!

MEZZOFORTA_ROOT=/home/roy/mezzoforta
MEZZOFORTA_APP=$MEZZOFORTA_ROOT/app
MEZZOFORTA_EXEC=$MEZZOFORTA_APP/MezzaForte

QT_ROOT=/home/roy/Qt/5.15.1
QT_BIN=$QT_ROOT/gcc_64/bin


TMP_DIR=/tmp
APP_DIR=$TMP_DIR/$APP_NAME

DEPLOY_DIR=/home/roy/deploy
TAR_BALL=$DEPLOY_DIR/$APP_NAME.tar.xz
ARTIFACTS_DIR=$DEPLOY_DIR/artifacts

ARTIFACTS_FILE_TEMPLATE=$ARTIFACTS_DIR/`date '+%Y%m%d-%H:%M:%S'`
ARTIFACTS_FILE_MAKE=$ARTIFACTS_FILE_TEMPLATE.make
ARTIFACTS_FILE_DEPLOY=$ARTIFACTS_FILE_TEMPLATE.linuxdeployqt
mkdir -p $ARTIFACTS_DIR

echo -----------------------------------------------------------
echo Deploying in $APP_DIR
echo -----------------------------------------------------------

echo 1.	Getting latest and greatest
cd $MEZZOFORTA_ROOT
git pull -q -f

echo 2.	Generating makefile
cd $MEZZOFORTA_APP
rm -f Makefile
$QT_BIN/qmake -makefile -config release MezzaForte.pro

echo 3.	Modifying makefile
mv Makefile Makefile.tmp
cat Makefile.tmp |sed 's/-Wall -Wextra/-Wall -Wextra -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated-copy/g' > Makefile
rm Makefile.tmp

echo 4.	Building app
make clean
make |& tee $ARTIFACTS_FILE_MAKE

echo 5.	Creating deploy structure
rm -rf $APP_DIR
mkdir -p $APP_DIR/usr/bin
mkdir -p $APP_DIR/usr/lib
mkdir -p $APP_DIR/usr/share/applications
mkdir -p $APP_DIR/usr/share/icons/hicolor/256x256/apps

#	Creating MezzoForta.desktop
cat << EOF > $APP_DIR/usr/share/applications/MezzoForta!.desktop
[Desktop Entry]
Type=Application
Name=MezzoForta!
Comment=Playing Whatever We Want!
Exec=MezzoForta
Icon=squarelogo
Categories=Music;
EOF

#	Moving exec
cp $MEZZOFORTA_EXEC $APP_DIR/usr/bin/MezzoForta
cp $MEZZOFORTA_APP/resources/squarelogo.png $APP_DIR/usr/share/icons/hicolor/256x256/apps

#	Creating image
echo 6.	Creating image
cd $DEPLOY_DIR
pwd
$QT_BIN/linuxdeployqt $APP_DIR/usr/share/applications/MezzoForta\!.desktop -appimage -verbose=1 | tee $ARTIFACTS_FILE_DEPLOY
mv MezzoForta\!-x86_64.AppImage MezzoForta\!.AppImage
