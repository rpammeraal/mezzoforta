QT       += core gui widgets sql xml network webenginewidgets multimedia httpserver

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql network webenginewidgets multimedia

CONFIG 		+= c++11 import_plugins static sdk_no_version_check plugin
QTPLUGIN    +=  QPSQL
QT_DEBUG_PLUGINS=1

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Preferences.cpp \
    SBHtmlAlbumsAll.cpp \
    SBHtmlChartsAll.cpp \
    SBHtmlPerformersAll.cpp \
    SBHtmlPlaylistsAll.cpp \
    SBHtmlSongsAll.cpp \
    SongAlbumNotes.cpp \
    SqlQuery.cpp \
    WebService.cpp \
    main.cpp \
    MainWindow.cpp \
    AudioDecoder.cpp \
    AudioDecoderFactory.cpp \
    AudioDecoderFlac.cpp \
    AudioDecoderFlacReader.cpp \
    AudioDecoderMP3.cpp \
    AudioDecoderMP3Reader.cpp \
    AudioDecoderOggVorbis.cpp \
    AudioDecoderOggVorbisReader.cpp \
    AudioDecoderReader.cpp \
    AudioDecoderWave.cpp \
    AudioDecoderWaveReader.cpp \
    BackgroundThread.cpp \
    Cache.cpp \
    CacheManager.cpp \
    Chooser.cpp \
    Common.cpp \
    CompleterFactory.cpp \
    Configuration.cpp \
    Context.cpp \
    Controller.cpp \
    DBManager.cpp \
    DataAccessLayer.cpp \
    DataAccessLayerPostgres.cpp \
    DataAccessLayerSQLite.cpp \
    DatabaseSelector.cpp \
    ExternalData.cpp \
    KeyboardEventCatcher.cpp \
    MetaData.cpp \
    MusicImportResult.cpp \
    MusicLibrary.cpp \
    Navigator.cpp \
    Network.cpp \
    PlayManager.cpp \
    PlayerController.cpp \
    Preloader.cpp \
    ProgressDialog.cpp \
    Properties.cpp \
    SBCaseInsensitiveString.cpp \
    SBDialogChart.cpp \
    SBDialogRenamePlaylist.cpp \
    SBDialogSelectItem.cpp \
    SBDuration.cpp \
    SBIDAlbum.cpp \
    SBIDAlbumPerformance.cpp \
    SBIDBase.cpp \
    SBIDChart.cpp \
    SBIDChartPerformance.cpp \
    SBIDOnlinePerformance.cpp \
    SBIDPerformer.cpp \
    SBIDPlaylist.cpp \
    SBIDPlaylistDetail.cpp \
    SBIDSong.cpp \
    SBIDSongPerformance.cpp \
    SBKey.cpp \
    SBLabel.cpp \
    SBMediaPlayer.cpp \
    SBMessageBox.cpp \
    SBModel.cpp \
    SBModelQueuedSongs.cpp \
    SBSortFilterProxyQueuedSongsModel.cpp \
    SBSortFilterProxyTableModel.cpp \
    SBSqlQueryModel.cpp \
    SBStandardItemModel.cpp \
    SBTab.cpp \
    SBTabAlbumDetail.cpp \
    SBTabAlbumEdit.cpp \
    SBTabChartDetail.cpp \
    SBTabChooser.cpp \
    SBTabPerformerDetail.cpp \
    SBTabPerformerEdit.cpp \
    SBTabPlaylistDetail.cpp \
    SBTabQueuedSongs.cpp \
    SBTabSongDetail.cpp \
    SBTabSongEdit.cpp \
    SBTabSongsAll.cpp \
    SBTableModel.cpp \
    ScreenItem.cpp \
    ScreenStack.cpp \
    SearchItemModel.cpp \
    SetupWizard.cpp 

OBJECTIVE_SOURCES += \
    OSXNSEventFunctions.mm

HEADERS += \
    AudioDecoder.h \
    AudioDecoderFactory.h \
    AudioDecoderFlac.h \
    AudioDecoderFlacReader.h \
    AudioDecoderMP3.h \
    AudioDecoderMP3Reader.h \
    AudioDecoderOggVorbis.h \
    AudioDecoderOggVorbisReader.h \
    AudioDecoderReader.h \
    AudioDecoderWave.h \
    AudioDecoderWaveReader.h \
    BackgroundThread.h \
    Cache.h \
    CacheManager.h \
    CacheTemplate.h \
    Chooser.h \
    Common.h \
    CompleterFactory.h \
    Configuration.h \
    Context.h \
    Controller.h \
    DBManager.h \
    DataAccessLayer.h \
    DataAccessLayerPostgres.h \
    DataAccessLayerSQLite.h \
    DatabaseSelector.h \
    ExternalData.h \
    KeyboardEventCatcher.h \
    MainWindow.h \
    MetaData.h \
    MusicImportResult.h \
    MusicLibrary.h \
    Navigator.h \
    Network.h \
    OSXNSEventFunctions.h \
    PlayManager.h \
    PlayerController.h \
    Preferences.h \
    Preloader.h \
    ProgressDialog.h \
    Properties.h \
    SBCaseInsensitiveString.h \
    SBDialogChart.h \
    SBDialogRenamePlaylist.h \
    SBDialogSelectItem.h \
    SBDuration.h \
    SBHtmlAlbumsAll.h \
    SBHtmlChartsAll.h \
    SBHtmlPerformersAll.h \
    SBHtmlPlaylistsAll.h \
    SBHtmlSongsAll.h \
    SBIDAlbum.h \
    SBIDAlbumPerformance.h \
    SBIDBase.h \
    SBIDChart.h \
    SBIDChartPerformance.h \
    SBIDOnlinePerformance.h \
    SBIDPerformer.h \
    SBIDPlaylist.h \
    SBIDPlaylistDetail.h \
    SBIDSong.h \
    SBIDSongPerformance.h \
    SBKey.h \
    SBLabel.h \
    SBMediaPlayer.h \
    SBMessageBox.h \
    SBModel.h \
    SBModelQueuedSongs.h \
    SBSortFilterProxyQueuedSongsModel.h \
    SBSortFilterProxyTableModel.h \
    SBSqlQueryModel.h \
    SBStandardItemModel.h \
    SBTab.h \
    SBTabAlbumDetail.h \
    SBTabAlbumEdit.h \
    SBTabChartDetail.h \
    SBTabChooser.h \
    SBTabPerformerDetail.h \
    SBTabPerformerEdit.h \
    SBTabPlaylistDetail.h \
    SBTabQueuedSongs.h \
    SBTabSongDetail.h \
    SBTabSongEdit.h \
    SBTabSongsAll.h \
    SBTableModel.h \
    ScreenItem.h \
    ScreenStack.h \
    SearchItemModel.h \
    SetupWizard.h \
    SongAlbumNotes.h \
    SqlQuery.h \
    WebService.h

FORMS += \
    DatabaseSelector.ui \
    MainWindow.ui \
    MusicImportResult.ui \
    PreferencesWindow.ui \
    SBDialogChart.ui \
    SBDialogRenamePlaylist.ui \
    SBDialogSelectItem.ui \
    SetupWizard.ui  \
    SongAlbumNotes.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

ICON = resources/images/logo.icns
RC_ICONS = resources/images/logo.ico

DISTFILES += \
    resources/images/AllSongs.png \
    resources/images/ChartIcon.png \
    resources/images/NoAlbumCover.png \
    resources/images/NoBandPhoto.png \
    resources/images/PlaylistIcon.png \
    resources/images/README \
    resources/images/SongIcon.png \
    resources/images/blank icon.png \
    resources/images/default.qss \
    resources/images/logo.icns \
    resources/images/logo.ico \
    resources/images/playing.png \
    resources/images/splash.png \
    resources/images/squarelogo.png \
    resources/images/www/index.html

#	Manually added
# export LDFLAGS="-L/usr/local/opt/libpq/lib"
# export CPPFLAGS="-I/usr/local/opt/libpq/include"
INCLUDEPATH += /usr/local/include /opt/homebrew/include/ /opt/homebrew/opt/libpq/include
DEPENDPATH +=  /usr/local/include /opt/homebrew/include/ /opt/homebrew/opt/libpq/include

unix: LIBS += -lportaudio -L /usr/local/lib  -logg -lvorbis -lvorbisfile -lmad -lid3tag -lFLAC -ltag -lpq
macx: LIBS += -L/usr/local/lib/ -lportaudio  -L/usr/local/lib -logg -lvorbis -lvorbisfile -lmad -lid3tag -lFLAC -framework Foundation -L$$[QT_INSTALL_PLUGINS]/imageformats -lpq -L/opt/homebrew/lib/ -L/opt/homebrew/opt/libpq/lib
