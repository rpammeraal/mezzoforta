QT += widgets sql xml network webenginewidgets multimedia

HEADERS     = \
    MainWindow.h \
    Controller.h \
    Common.h \
    DataAccessLayer.h \
    DataAccessLayerPostgres.h \
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
    AudioDecoderOggVorbisReader.h \
    AudioDecoderWaveReader.h \
    AudioDecoderMP3Reader.h \
    AudioDecoderFlacReader.h \
    AudioDecoderReader.h \
    SBTabQueuedSongs.h \
    SBModelQueuedSongs.h \
    SBSortFilterProxyQueuedSongsModel.h \
    PlayManager.h \
    SBIDSong.h \
    SBIDPerformer.h \
    SBIDAlbum.h \
    SBIDPlaylist.h \
    Properties.h \
    MusicLibrary.h \
    DatabaseSelector.h \
    DBManager.h \
    KeyboardEventCatcher.h \
    OSXNSEventFunctions.h \
    MetaData.h \
    SBCaseInsensitiveString.h \
    SBIDBase.h \
    ScreenItem.h \
    SBTableModel.h \
    SBSortFilterProxyTableModel.h \
    Preloader.h \
    SBIDAlbumPerformance.h \
    SBIDSongPerformance.h \
    SetupWizard.h \
    DataAccessLayerSQLite.h \
    Network.h \
    Configuration.h \
    MusicImportResult.h \
    SBIDOnlinePerformance.h \
    SBIDChart.h \
    SBTabChartDetail.h \
    SBTabChooser.h \
    SBIDChartPerformance.h \
    ProgressDialog.h \
    SBDuration.h \
    SBIDPlaylistDetail.h \
    SearchItemModel.h \
    CacheManager.h \
    CacheTemplate.h \
    Cache.h \
    SBKey.h

SOURCES     = \
    main.cpp \
    MainWindow.cpp \
    Controller.cpp \
    Common.cpp \
    DataAccessLayer.cpp \
    DataAccessLayerPostgres.cpp \
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
    AudioDecoderOggVorbisReader.cpp \
    AudioDecoderWaveReader.cpp \
    AudioDecoderMP3Reader.cpp \
    AudioDecoderFlacReader.cpp \
    AudioDecoderReader.cpp \
    SBTabQueuedSongs.cpp \
    SBModelQueuedSongs.cpp \
    SBSortFilterProxyQueuedSongsModel.cpp \
    PlayManager.cpp \
    SBIDSong.cpp \
    SBIDPerformer.cpp \
    SBIDAlbum.cpp \
    SBIDPlaylist.cpp \
    Properties.cpp \
    MusicLibrary.cpp \
    DatabaseSelector.cpp \
    DBManager.cpp \
    KeyboardEventCatcher.cpp \
    MetaData.cpp \
    SBCaseInsensitiveString.cpp \
    SBIDBase.cpp \
    ScreenItem.cpp \
    SBTableModel.cpp \
    SBSortFilterProxyTableModel.cpp \
    Preloader.cpp \
    SBIDAlbumPerformance.cpp \
    SBIDSongPerformance.cpp \
    SetupWizard.cpp \
    DataAccessLayerSQLite.cpp \
    Network.cpp \
    Configuration.cpp \
    MusicImportResult.cpp \
    SBIDOnlinePerformance.cpp \
    SBIDChart.cpp \
    SBTabChartDetail.cpp \
    SBTabChooser.cpp \
    SBIDChartPerformance.cpp \
    ProgressDialog.cpp \
    SBDuration.cpp \
    SBIDPlaylistDetail.cpp \
    SearchItemModel.cpp \
    CacheManager.cpp \
    Cache.cpp \
    SBKey.cpp

OBJECTIVE_SOURCES += \
    OSXNSEventFunctions.mm

# install
target.path = .
INSTALLS += target

FORMS += \
    MainWindow.ui \
    DatabaseSelector.ui \
    SBDialogRenamePlaylist.ui \
    SBDialogSelectItem.ui \
    SetupWizard.ui \
    MusicImportResult.ui

RESOURCES += \
    resource.qrc


ICON = resources/logo.icns
RC_ICONS = resources/logo.ico

DISTFILES += \
    PlacesDeveloped.txt \
    resources/squarelogo.bmp

unix: LIBS += -lportaudio -L/sw/lib/ -logg -lvorbis -lvorbisfile -lmad -lid3tag -lFLAC -ltag
macx: LIBS += -L/usr/local/lib/ -lportaudio -L/sw/lib/ -logg -lvorbis -lvorbisfile -lmad -lid3tag -lFLAC.8 -framework Foundation
macx: PRE_TARGETDEPS += /usr/local/lib/libportaudio.a /sw/lib/libogg.a /sw/lib/libvorbis.a /sw/lib/libvorbisfile.a /sw/lib/libmad.a /sw/lib/libid3tag.a

INCLUDEPATH += /usr/local/include /sw/include C:/usr/local/include
DEPENDPATH += /usr/local/include /sw/include C:/usr/local/include

win32: LIBS += -LC:/usr/local/lib/ -lportaudio_x86  -llibogg -llibvorbis -llibvorbisfile -llibmad -lzlibstatic -ltag


