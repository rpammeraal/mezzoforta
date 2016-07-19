QT += widgets sql xml network webkitwidgets multimedia

HEADERS     = \
    MainWindow.h \
    Controller.h \
    Common.h \
    DataAccessLayer.h \
    DataAccessLayerPostgres.h \
    SBID.h \
    Context.h \
    ScreenStack.h \
    ExternalData.h \
    SBStandardItemModel.h \
    SBSqlQueryModel.h \
    SBLabel.h \
    SBDialogRenamePlaylist.h \
    BackgroundThread.h \
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
    AudioDecoder.h \
    AudioDecoderFactory.h \
    AudioDecoderFlac.h \
    AudioDecoderMP3.h \
    AudioDecoderOggVorbis.h \
    AudioDecoderWave.h \
    DataEntityAlbum.h \
    DataEntityCurrentPlaylist.h \
    DataEntityGenrelist.h \
    DataEntityPerformer.h \
    DataEntityPlaylist.h \
    DataEntitySong.h \
    AudioDecoderOggVorbisReader.h \
    AudioDecoderWaveReader.h \
    AudioDecoderMP3Reader.h \
    AudioDecoderFlacReader.h \
    AudioDecoderReader.h \
    SBTabQueuedSongs.h \
    SBModelQueuedSongs.h \
    SBSortFilterProxyQueuedSongsModel.h \
    Duration.h \
    PlayManager.h \
    SBIDSong.h \
    SBIDPerformer.h \
    SBIDAlbum.h \
    SBIDPlaylist.h \
    Properties.h \
    MusicLibrary.h \
    DatabaseSelector.h \
    DBManager.h

SOURCES     = \
    main.cpp \
    MainWindow.cpp \
    Controller.cpp \
    Common.cpp \
    DataAccessLayer.cpp \
    DataAccessLayerPostgres.cpp \
    SBID.cpp \
    Context.cpp \
    ScreenStack.cpp \
    ExternalData.cpp \
    SBStandardItemModel.cpp \
    SBSqlQueryModel.cpp \
    SBLabel.cpp \
    SBDialogRenamePlaylist.cpp \
    BackgroundThread.cpp \
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
    AudioDecoder.cpp \
    AudioDecoderFactory.cpp \
    AudioDecoderFlac.cpp \
    AudioDecoderMP3.cpp \
    AudioDecoderOggVorbis.cpp \
    AudioDecoderWave.cpp \
    DataEntityAlbum.cpp \
    DataEntityCurrentPlaylist.cpp \
    DataEntityGenreList.cpp \
    DataEntityPerformer.cpp \
    DataEntityPlaylist.cpp \
    DataEntitySong.cpp \
    AudioDecoderOggVorbisReader.cpp \
    AudioDecoderWaveReader.cpp \
    AudioDecoderMP3Reader.cpp \
    AudioDecoderFlacReader.cpp \
    AudioDecoderReader.cpp \
    SBTabQueuedSongs.cpp \
    SBModelQueuedSongs.cpp \
    SBSortFilterProxyQueuedSongsModel.cpp \
    Duration.cpp \
    PlayManager.cpp \
    SBIDSong.cpp \
    SBIDPerformer.cpp \
    SBIDAlbum.cpp \
    SBIDPlaylist.cpp \
    Properties.cpp \
    MusicLibrary.cpp \
    DatabaseSelector.cpp \
    DBManager.cpp

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

macx: LIBS += -L/usr/local/lib/ -lportaudio -L/sw/lib/ -logg -lvorbis -lvorbisfile -lmad -lid3tag -lFLAC.8
macx: PRE_TARGETDEPS += /usr/local/lib/libportaudio.a /sw/lib/libogg.a /sw/lib/libvorbis.a /sw/lib/libvorbisfile.a /sw/lib/libmad.a /sw/lib/libid3tag.a

INCLUDEPATH += /usr/local/include /sw/include C:/usr/local/include
DEPENDPATH += /usr/local/include /sw/include C:/usr/local/include

win32: LIBS += -LC:/usr/local/lib/ -lportaudio_x86  -llibogg -llibvorbis -llibvorbisfile -llibmad

