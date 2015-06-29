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
    SonglistScreenHandler.h \
    Context.h \
    ScreenStack.h \
    SBModelSong.h \
    SBModelAlbum.h \
    SBModelPerformer.h \
    SBIDExtended.h \
    LeftColumnChooser.h \
    ExternalData.h \
    RenamePlaylist.h \
    SBStandardItemModel.h \
    SBSqlQueryModel.h \
    SBLabel.h

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
    SonglistScreenHandler.cpp \
    Context.cpp \
    ScreenStack.cpp \
    SBModelSong.cpp \
    SBModelAlbum.cpp \
    SBModelPerformer.cpp \
    SBIDExtended.cpp \
    LeftColumnChooser.cpp \
    ExternalData.cpp \
    RenamePlaylist.cpp \
    SBStandardItemModel.cpp \
    SBSqlQueryModel.cpp \
    SBLabel.cpp

# install
target.path = .
INSTALLS += target

FORMS += \
    MainWindow.ui \
    DatabaseSelector.ui \
    RenamePlaylist.ui

RESOURCES += \
    resource.qrc


ICON = resources/moose.icns
RC_ICONS = resources/moose.ico

DISTFILES += \
    PlacesDeveloped.txt \
    install.txt \
    resources/moose7.2.bmp
