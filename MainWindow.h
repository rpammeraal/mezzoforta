#ifndef DIALOG_H
#define DIALOG_H

#include <QMainWindow>
#include <QtSql>

#include "ui_mainwindow.h"
#include "DataAccessLayer.h"

class QAction;
class QActionGroup;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTableView;
class QTextEdit;
class QGridLayout;
class QTreeView;
class QSortFilterProxyModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    int getErrorState();
    QString getErrorDescription();

protected:
    //void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);

    void resizeWindow();

private slots:
    void playlistSelected(const QItemSelection &selected, const QItemSelection &deselected);
    void applyFilter(const QString& filter="");
    void newFile();
    void open();
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


    //	Common
    QGridLayout* layoutGrid;
    int hasFatalError;
    QString errorDescription;
    DataAccessLayer* dal;
    void createLayout();
    QSortFilterProxyModel* songListFilter;

    void setPlaylist(const int playlistID);

    void hideColumns(QTableView* tv);
    void setErrorState(const QString& errorState);

    //	Search
    QLineEdit* searchEdit;

    //	Main
    QTableView* songList;

    //	Tree
    QTableView* metaList;

    //	Menu
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *formatMenu;
    QMenu *helpMenu;
    QActionGroup *alignmentGroup;
    QAction *newAct;
    QAction *openAct;
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

#endif // DIALOG_H
