QT += widgets sql xml network webkitwidgets multimedia

HEADERS     = \
    MainWindow.h \
    Controller.h \
    Common.h \
    DatabaseSelector.h \
    DataAccessLayer.h \
    DataAccessLayerPostgres.h \
    SBModelPlaylist.h \
    SBModelGenrelist.h \
    SBID.h \
    Context.h \
    ScreenStack.h \
    SBModelSong.h \
    SBModelAlbum.h \
    SBModelPerformer.h \
    SBIDExtended.h \
    ExternalData.h \
    SBStandardItemModel.h \
    SBSqlQueryModel.h \
    SBLabel.h \
    SBDialogRenamePlaylist.h \
    BackgroundThread.h \
    SBTime.h \
    SBTab.h \
    SBTabPerformerEdit.h \
    SBTabSongEdit.h \
    SBTabSongsAll.h \
    SBTabPlaylistDetail.h \
    SBTabPerformerDetail.h \
    SBTabAlbumDetail.h \
    SBTabSongDetail.h \
    Navigator.h \
    Chooser.h \
    SBTabAlbumEdit.h \
    SBModel.h \
    CompleterFactory.h \
    SBMessageBox.h \
    SBDialogSelectItem.h \
    PlayerController.h \
    SBMediaPlayer.h \
    SBTabCurrentPlaylist.h \
    SBModelCurrentPlaylist.h \
    SBAudioDecoder.h \
    SBAudioDecoderFactory.h \
    SBAudioDecoderWave.h \
    SBAudioDecoderOggVorbis.h

SOURCES     = \
    main.cpp \
    MainWindow.cpp \
    Controller.cpp \
    Common.cpp \
    DatabaseSelector.cpp \
    DataAccessLayer.cpp \
    DataAccessLayerPostgres.cpp \
    SBModelPlaylist.cpp \
    SBModelGenreList.cpp \
    SBID.cpp \
    Context.cpp \
    ScreenStack.cpp \
    SBModelSong.cpp \
    SBModelAlbum.cpp \
    SBModelPerformer.cpp \
    SBIDExtended.cpp \
    ExternalData.cpp \
    SBStandardItemModel.cpp \
    SBSqlQueryModel.cpp \
    SBLabel.cpp \
    SBDialogRenamePlaylist.cpp \
    BackgroundThread.cpp \
    SBTime.cpp \
    SBTab.cpp \
    SBTabPerformerEdit.cpp \
    SBTabSongEdit.cpp \
    SBTabSongsAll.cpp \
    SBTabPlaylistDetail.cpp \
    SBTabPerformerDetail.cpp \
    SBTabAlbumDetail.cpp \
    SBTabSongDetail.cpp \
    Navigator.cpp \
    Chooser.cpp \
    SBTabAlbumEdit.cpp \
    SBModel.cpp \
    CompleterFactory.cpp \
    SBMessageBox.cpp \
    SBDialogSelectItem.cpp \
    PlayerController.cpp \
    SBMediaPlayer.cpp \
    SBTabCurrentPlaylist.cpp \
    SBModelCurrentPlaylist.cpp \
    SBAudioDecoder.cpp \
    SBAudioDecoderFactory.cpp \
    SBAudioDecoderWave.cpp \
    SBAudioDecoderOggVorbis.cpp

# install
target.path = .
INSTALLS += target

FORMS += \
    MainWindow.ui \
    DatabaseSelector.ui \
    SBDialogRenamePlaylist.ui \
    SBDialogSelectItem.ui

RESOURCES += \
    resource.qrc


ICON = resources/moose.icns
RC_ICONS = resources/moose.ico

DISTFILES += \
    PlacesDeveloped.txt \
    resources/moose7.2.bmp

macx: LIBS += -L/usr/local/lib/ -lportaudio

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

macx: PRE_TARGETDEPS += /usr/local/lib/libportaudio.a
