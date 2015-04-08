QT += widgets sql

HEADERS     = \
    MainWindow.h \
    DataAccessLayer.h \
    SBSortFilterProxyModel.h
SOURCES     = \
    main.cpp \
    MainWindow.cpp \
    DataAccessLayer.cpp \
    SBSortFilterProxyModel.cpp

# install
target.path = .
INSTALLS += target

FORMS += \
    MainWindow.ui

RESOURCES += \
    resource.qrc


ICON = resources/moose.icns
RC_ICONS = resources/moose.ico
