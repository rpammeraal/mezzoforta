#ifndef DIALOG_H
#define DIALOG_H

#include <QMainWindow>
#include <QtSql>

#include "ui_mainwindow.h"
#include "DataAccessLayer.h"

class QLabel;
class Controller;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class Controller;

public:
    MainWindow();
    int getErrorState();
    QString getErrorDescription();

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);

    void resizeWindow();

private slots:
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
    Controller* c;

    void createMenus();
    void createActions();
    Ui::MainWindow ui;

    //	Common
    int hasFatalError;
    QString errorDescription;
    DataAccessLayer* dal;

    void hideColumns(QTableView* tv);
    void setErrorState(const QString& errorState);

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
