#!/bin/bash

#	Requires linuxdeployqt: https://github.com/probonopd/linuxdeployqt
#	Installed manually (not with help of QT UI)
#	Also install patchelf
#	And appimagetool

APP_NAME=MezzoForta\!

MEZZOFORTA_ROOT=/Users/roy/mezzoforta
MEZZOFORTA_APP=$MEZZOFORTA_ROOT/app
MEZZOFORTA_EXEC=$MEZZOFORTA_APP/MezzaForte.app
MEZZOFORTA_DMG=$MEZZOFORTA_APP/Mezzaforte.dmg

QT_ROOT=/Users/roy/Qt/5.15.1
QT_BIN=$QT_ROOT/clang_64/bin


TMP_DIR=/tmp
TMP_APP_DIR=$TMP_DIR/$APP_NAME

DEPLOY_DIR=/Users/roy/deploy
ARTIFACTS_DIR=$DEPLOY_DIR/artifacts

ARTIFACTS_FILE_TEMPLATE=$ARTIFACTS_DIR/`date '+%Y%m%d-%H:%M:%S'`
ARTIFACTS_FILE_MAKE=$ARTIFACTS_FILE_TEMPLATE.make
ARTIFACTS_FILE_DEPLOY=$ARTIFACTS_FILE_TEMPLATE.macdeployqt
mkdir -p $ARTIFACTS_DIR

echo -----------------------------------------------------------
echo Deploying in $TMP_APP_DIR
echo -----------------------------------------------------------

echo 1.	Getting latest and greatest
cd $MEZZOFORTA_ROOT
git pull -q -f

echo 2.	Generating makefile
cd $MEZZOFORTA_APP
rm -f Makefile
$QT_BIN/qmake -makefile -config release MezzaForte.pro

echo 3.	Removing .app directory
rm -rf $MEZZOFORTA_EXEC
rm -rf $MEZZOFORTA_DMG

echo 4.	Building app
#make clean
make | tee $ARTIFACTS_FILE_MAKE

echo 5.	Moving frameworks
mkdir -p MezzaForte.app/Contents/Frameworks
cp -Rp \
	$QT_ROOT/clang_64/lib/QtQuickWidgets.framework \
	$QT_ROOT/clang_64/lib/QtWebEngine.framework \
	$QT_ROOT/clang_64/lib/QtWebEngineCore.framework \
	$QT_ROOT/clang_64/lib/QtWebEngineWidgets.framework \
	$QT_ROOT/clang_64/lib/QtWebSockets.framework \
		MezzaForte.app/Contents/Frameworks

echo 6.	Creating image
$QT_BIN/macdeployqt MezzaForte.app -dmg | tee ARTIFACTS_FILE_DEPLOY

echo 7.	Move DMG file to deploy directory
mv -f $MEZZOFORTA_DMG $DEPLOY_DIR
