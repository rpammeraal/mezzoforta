#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>

#include "ui_MainWindow.h"
#include "DataAccessLayer.h"
#include "KeyboardEventCatcher.h"

class QLabel;
class Controller;
class Navigator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class Controller;
    friend class Navigator;
    friend class Chooser;
    friend class PlayerController;
    friend class PlayManager;
    friend class SBTab;
    friend class SBTabAlbumDetail;
    friend class SBTabAlbumEdit;
    friend class SBTabChartDetail;
    friend class SBTabQueuedSongs;
    friend class SBTabPerformerDetail;
    friend class SBTabPerformerEdit;
    friend class SBTabPlaylistDetail;
    friend class SBTabSongDetail;
    friend class SBTabSongEdit;
    friend class SBTabSongsAll;

public:
    MainWindow();

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent * event) Q_DECL_OVERRIDE;

private slots:
    void newFile();
    void openDatabase();
    void save();
    void print();
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void bold();
    void italic();
    void leftAlign();
    void rightAlign();
    void justify();
    void center();
    void setLineSpacing();
    void setParagraphSpacing();
    void about();
    void aboutQt();

private:

    void createMenus();
    void createActions();
    Ui::MainWindow ui;

    //	Menu
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *formatMenu;
    QMenu *helpMenu;
    QActionGroup *alignmentGroup;
    QAction *newAct;
    QAction *openDatabaseAction;
    QAction *saveAct;
    QAction *printAct;
    QAction *exitAct;
    QAction *undoAct;
    QAction *redoAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *boldAct;
    QAction *italicAct;
    QAction *leftAlignAct;
    QAction *rightAlignAct;
    QAction *justifyAct;
    QAction *centerAct;
    QAction *setLineSpacingAct;
    QAction *setParagraphSpacingAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QLabel *infoLabel;

};

#endif // MAINWINDOW_H
