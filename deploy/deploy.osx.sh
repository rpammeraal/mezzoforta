#!/bin/bash

<<<<<<< HEAD
#	Requires linuxdeployqt: https://github.com/probonopd/linuxdeployqt
#	Installed manually (not with help of QT UI)
#	Also install patchelf
#	And appimagetool

NAME=MezzoForta
NAME_APP=$NAME.app


MEZZOFORTA_ROOT=/Users/roy/mezzoforta
MEZZOFORTA_SRC=$MEZZOFORTA_ROOT/app
MEZZOFORTA_APP=$MEZZOFORTA_SRC/$NAME.app
MEZZOFORTA_DMG=$MEZZOFORTA_SRC/$NAME.dmg
MEZZOFORTA_PRO=$NAME.pro
MEZZOFORTA_EXEC=$MEZZOFORTA_SRC/$NAME_APP

QT_ROOT=/Users/roy/Qt/6.4.2
QT_BIN=$QT_ROOT/macos/bin



DEPLOY_DIR=/Users/roy/deploy
ARTIFACTS_DIR=$DEPLOY_DIR/artifacts

ARTIFACTS_FILE_TEMPLATE=$ARTIFACTS_DIR/`date '+%Y%m%d-%H:%M:%S'`
ARTIFACTS_FILE_MAKE=$ARTIFACTS_FILE_TEMPLATE.make
ARTIFACTS_FILE_DEPLOY=$ARTIFACTS_FILE_TEMPLATE.macdeployqt
mkdir -p $ARTIFACTS_DIR

echo -----------------------------------------------------------
echo Deploying
echo -----------------------------------------------------------

echo 1.	Getting latest and greatest
cd $MEZZOFORTA_ROOT
git pull -q -f

echo 2.	Generating makefile
cd $MEZZOFORTA_SRC
rm -f Makefile
$QT_BIN/qmake -makefile -config release $MEZZOFORTA_PRO

echo 3.	Removing .app directory
rm -rf $MEZZOFORTA_APP
rm -rf $MEZZOFORTA_DMG

echo 4.	Building app
make clean
make | tee $ARTIFACTS_FILE_MAKE

echo 5.	Moving frameworks
mkdir -p $MEZZOFORTA_APP/Contents/Frameworks
cp -Rpf \
	$QT_ROOT/macos/lib/QtQuickWidgets.framework \
	$QT_ROOT/macos/lib/QtWebEngineCore.framework \
	$QT_ROOT/macos/lib/QtWebEngineWidgets.framework \
	$QT_ROOT/macos/lib/QtWebSockets.framework \
		MezzoForta.app/Contents/Frameworks

echo 6.	Creating image
$QT_BIN/macdeployqt NAME_APP -dmg | tee $ARTIFACTS_FILE_DEPLOY

echo 7.	Move DMG file to deploy directory
mv -f $MEZZOFORTA_DMG $DEPLOY_DIR
