QT += widgets sql

HEADERS     = \
    MainWindow.h \
    Controller.h \
    Common.h \
    DisplayOnlyDelegate.h \
    DatabaseSelector.h \
    DataAccessLayer.h \
    DataAccessLayerPostgres.h \
    SBModel.h \
    SBModelSonglist.h \
    SBModelPlaylist.h \
    SBModelGenrelist.h \
    SBID.h \
    SonglistScreenHandler.h \
    Context.h \
    ScreenStack.h

SOURCES     = \
    main.cpp \
    MainWindow.cpp \
    Controller.cpp \
    Common.cpp \
    DisplayOnlyDelegate.cpp \
    DatabaseSelector.cpp \
    DataAccessLayer.cpp \
    DataAccessLayerPostgres.cpp \
    SBModel.cpp \
    SBModelSonglist.cpp \
    SBModelPlaylist.cpp \
    SBModelGenreList.cpp \
    SBID.cpp \
    SonglistScreenHandler.cpp \
    Context.cpp \
    ScreenStack.cpp

# install
target.path = .
INSTALLS += target

FORMS += \
    MainWindow.ui \
    DatabaseSelector.ui

RESOURCES += \
    resource.qrc


ICON = resources/moose.icns
RC_ICONS = resources/moose.ico

DISTFILES += \
    PlacesDeveloped.txt \
    install.txt \
    resources/moose7.2.bmp
