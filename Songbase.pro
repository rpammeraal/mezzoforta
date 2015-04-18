QT += widgets sql

HEADERS     = \
    MainWindow.h \
    DataAccessLayer.h \
    SBSortFilterProxyModel.h \
    Controller.h \
    Common.h \
    DisplayOnlyDelegate.h
SOURCES     = \
    main.cpp \
    MainWindow.cpp \
    DataAccessLayer.cpp \
    SBSortFilterProxyModel.cpp \
    Controller.cpp \
    Common.cpp \
    DisplayOnlyDelegate.cpp

# install
target.path = .
INSTALLS += target

FORMS += \
    MainWindow.ui

RESOURCES += \
    resource.qrc


ICON = resources/moose.icns
RC_ICONS = resources/moose.ico

DISTFILES += \
    PlacesDeveloped.txt
