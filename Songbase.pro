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
    SBModelGenrelist.h

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
    SBModelGenreList.cpp

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
