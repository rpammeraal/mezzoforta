QT += widgets sql xml network webkitwidgets

HEADERS     = \
    MainWindow.h \
    Controller.h \
    Common.h \
    DisplayOnlyDelegate.h \
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
    LeftColumnChooser.h \
    ExternalData.h \
    SBStandardItemModel.h \
    SBSqlQueryModel.h \
    SBLabel.h \
    SBDialogRenamePlaylist.h \
    SBDialogSelectSongAlbum.h \
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
    Navigator.h

SOURCES     = \
    main.cpp \
    MainWindow.cpp \
    Controller.cpp \
    Common.cpp \
    DisplayOnlyDelegate.cpp \
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
    LeftColumnChooser.cpp \
    ExternalData.cpp \
    SBStandardItemModel.cpp \
    SBSqlQueryModel.cpp \
    SBLabel.cpp \
    SBDialogRenamePlaylist.cpp \
    SBDialogSelectSongAlbum.cpp \
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
    Navigator.cpp

# install
target.path = .
INSTALLS += target

FORMS += \
    MainWindow.ui \
    DatabaseSelector.ui \
    SBDialogRenamePlaylist.ui \
    SBDialogSelectSongAlbum.ui

RESOURCES += \
    resource.qrc


ICON = resources/moose.icns
RC_ICONS = resources/moose.ico

DISTFILES += \
    PlacesDeveloped.txt \
    resources/moose7.2.bmp
