#include <QtWidgets>
#include <QDebug>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "MainWindow.h"
#include "Navigator.h"

MainWindow::MainWindow()
{
    Context::instance()->setMainWindow(this);
    ui.setupUi(this);

    //createMenus();
    createActions();

    return;
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << "contextMenuEvent" ":event=" << event ;
    QMenu menu(this);
    menu.addAction(cutAct);
    //menu.addAction(copyAct);
    //menu.addAction(pasteAct);
    menu.exec(event->globalPos());
}

void
MainWindow::keyPressEvent(QKeyEvent * event)
{
    qDebug() << SB_DEBUG_INFO;
    Context::instance()->getNavigator()->keyPressEvent(event);
    QMainWindow::keyPressEvent(event);
}

void MainWindow::createMenus()
{
    return;

    //menuBar=new QMenuBar(0);
    //memenuBar->setNativeMenuBar(0);

    //mefileMenu = menuBar->addMenu(tr("&File"));
    //fileMenu->addAction(NewAct);
    //mefileMenu->addAction(openDatabaseAction);
    //fileMenu->addAction(saveAct);
    //fileMenu->addAction(printAct);
    //mefileMenu->addSeparator();
    //mefileMenu->addAction(exitAct);

//    editMenu = menuBar()->addMenu(tr("&Edit"));
//    editMenu->addAction(undoAct);
//    editMenu->addAction(redoAct);
//    editMenu->addSeparator();
//    editMenu->addAction(cutAct);
//    editMenu->addAction(copyAct);
//    editMenu->addAction(pasteAct);
//    editMenu->addSeparator();
//
//    helpMenu = menuBar()->addMenu(tr("&Help"));
//    helpMenu->addAction(aboutAct);
//    helpMenu->addAction(aboutQtAct);
//
//    formatMenu = editMenu->addMenu(tr("&Format"));
//    formatMenu->addAction(boldAct);
//    formatMenu->addAction(italicAct);
//    formatMenu->addSeparator()->setText(tr("Alignment"));
//    formatMenu->addAction(leftAlignAct);
//    formatMenu->addAction(rightAlignAct);
//    formatMenu->addAction(justifyAct);
//    formatMenu->addAction(centerAct);
//    formatMenu->addSeparator();
//    formatMenu->addAction(setLineSpacingAct);
//    formatMenu->addAction(setParagraphSpacingAct);
}

void MainWindow::createActions()
{
    //newAct = new QAction(tr("&New"), this);
    //newAct->setShortcuts(QKeySequence::New);
    //newAct->setStatusTip(tr("Create a new file"));
    //connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openDatabaseAction = new QAction(tr("&Open..."), this);
    openDatabaseAction->setShortcuts(QKeySequence::Open);
    openDatabaseAction->setStatusTip(tr("Open an existing file"));
    connect(openDatabaseAction, SIGNAL(triggered()), this, SLOT(openDatabase()));

//    saveAct = new QAction(tr("&Save"), this);
//    saveAct->setShortcuts(QKeySequence::Save);
//    saveAct->setStatusTip(tr("Save the document to disk"));
//    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
//
//    printAct = new QAction(tr("&Print..."), this);
//    printAct->setShortcuts(QKeySequence::Print);
//    printAct->setStatusTip(tr("Print the document"));
//    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

//    undoAct = new QAction(tr("&Undo"), this);
//    undoAct->setShortcuts(QKeySequence::Undo);
//    undoAct->setStatusTip(tr("Undo the last operation"));
//    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));
//
//    redoAct = new QAction(tr("&Redo"), this);
//    redoAct->setShortcuts(QKeySequence::Redo);
//    redoAct->setStatusTip(tr("Redo the last operation"));
//    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));
//
    cutAct = new QAction(tr("Ku&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

//    copyAct = new QAction(tr("&Copy"), this);
//    copyAct->setShortcuts(QKeySequence::Copy);
//    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
//                             "clipboard"));
//    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));
//
//    pasteAct = new QAction(tr("&Paste"), this);
//    pasteAct->setShortcuts(QKeySequence::Paste);
//    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
//                              "selection"));
//    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));
//
//    boldAct = new QAction(tr("&Bold"), this);
//    boldAct->setCheckable(true);
//    boldAct->setShortcut(QKeySequence::Bold);
//    boldAct->setStatusTip(tr("Make the text bold"));
//    connect(boldAct, SIGNAL(triggered()), this, SLOT(bold()));
//
//    QFont boldFont = boldAct->font();
//    boldFont.setBold(true);
//    boldAct->setFont(boldFont);
//
//    italicAct = new QAction(tr("&Italic"), this);
//    italicAct->setCheckable(true);
//    italicAct->setShortcut(QKeySequence::Italic);
//    italicAct->setStatusTip(tr("Make the text italic"));
//    connect(italicAct, SIGNAL(triggered()), this, SLOT(italic()));
//
//    QFont italicFont = italicAct->font();
//    italicFont.setItalic(true);
//    italicAct->setFont(italicFont);
//
//    setLineSpacingAct = new QAction(tr("Set &Line Spacing..."), this);
//    setLineSpacingAct->setStatusTip(tr("Change the gap between the lines of a "
//                                       "paragraph"));
//    connect(setLineSpacingAct, SIGNAL(triggered()), this, SLOT(setLineSpacing()));
//
//    setParagraphSpacingAct = new QAction(tr("Set &Paragraph Spacing..."), this);
//    setParagraphSpacingAct->setStatusTip(tr("Change the gap between paragraphs"));
//    connect(setParagraphSpacingAct, SIGNAL(triggered()),
//            this, SLOT(setParagraphSpacing()));
//
//    aboutAct = new QAction(tr("&About"), this);
//    aboutAct->setStatusTip(tr("Show the application's About box"));
//    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
//
//    aboutQtAct = new QAction(tr("About &Qt"), this);
//    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
//    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
//    connect(aboutQtAct, SIGNAL(triggered()), this, SLOT(aboutQt()));
//
//    leftAlignAct = new QAction(tr("&Left Align"), this);
//    leftAlignAct->setCheckable(true);
//    leftAlignAct->setShortcut(tr("Ctrl+L"));
//    leftAlignAct->setStatusTip(tr("Left align the selected text"));
//    connect(leftAlignAct, SIGNAL(triggered()), this, SLOT(leftAlign()));
//
//    rightAlignAct = new QAction(tr("&Right Align"), this);
//    rightAlignAct->setCheckable(true);
//    rightAlignAct->setShortcut(tr("Ctrl+R"));
//    rightAlignAct->setStatusTip(tr("Right align the selected text"));
//    connect(rightAlignAct, SIGNAL(triggered()), this, SLOT(rightAlign()));
//
//    justifyAct = new QAction(tr("&Justify"), this);
//    justifyAct->setCheckable(true);
//    justifyAct->setShortcut(tr("Ctrl+J"));
//    justifyAct->setStatusTip(tr("Justify the selected text"));
//    connect(justifyAct, SIGNAL(triggered()), this, SLOT(justify()));
//
//    centerAct = new QAction(tr("&Center"), this);
//    centerAct->setCheckable(true);
//    centerAct->setShortcut(tr("Ctrl+E"));
//    centerAct->setStatusTip(tr("Center the selected text"));
//    connect(centerAct, SIGNAL(triggered()), this, SLOT(center()));
//
//    alignmentGroup = new QActionGroup(this);
//    alignmentGroup->addAction(leftAlignAct);
//    alignmentGroup->addAction(rightAlignAct);
//    alignmentGroup->addAction(justifyAct);
//    alignmentGroup->addAction(centerAct);
//    leftAlignAct->setChecked(true);
}

void MainWindow::newFile()
{
    infoLabel->setText(tr("Invoked <b>File|New</b>"));
}

void MainWindow::openDatabase()
{
    infoLabel->setText(tr("Invoked <b>File|OpenDatabase</b>"));
}

void MainWindow::save()
{
    //ininfoLabel->setText(tr("Invoked <b>File|Save</b>"));
}

void MainWindow::print()
{
    //ininfoLabel->setText(tr("Invoked <b>File|Print</b>"));
}

void MainWindow::undo()
{
    //ininfoLabel->setText(tr("Invoked <b>Edit|Undo</b>"));
}

void MainWindow::redo()
{
    //ininfoLabel->setText(tr("Invoked <b>Edit|Redo</b>"));
}

void MainWindow::cut()
{
    qDebug() << SB_DEBUG_INFO;
    //ininfoLabel->setText(tr("Invoked <b>Edit|Cut</b>"));
}

void MainWindow::copy()
{
    //ininfoLabel->setText(tr("Invoked <b>Edit|Copy</b>"));
}

void MainWindow::paste()
{
    //ininfoLabel->setText(tr("Invoked <b>Edit|Paste</b>"));
}

void MainWindow::bold()
{
    //ininfoLabel->setText(tr("Invoked <b>Edit|Format|Bold</b>"));
}

void MainWindow::italic()
{
    //ininfoLabel->setText(tr("Invoked <b>Edit|Format|Italic</b>"));
}

void MainWindow::leftAlign()
{
    //ininfoLabel->setText(tr("Invoked <b>Edit|Format|Left Align</b>"));
}

void MainWindow::rightAlign()
{
    //ininfoLabel->setText(tr("Invoked <b>Edit|Format|Right Align</b>"));
}

void MainWindow::justify()
{
    //ininfoLabel->setText(tr("Invoked <b>Edit|Format|Justify</b>"));
}

void MainWindow::center()
{
    //infoLabel->setText(tr("Invoked <b>Edit|Format|Center</b>"));
}

void MainWindow::setLineSpacing()
{
    //ininfoLabel->setText(tr("Invoked <b>Edit|Format|Set Line Spacing</b>"));
}

void MainWindow::setParagraphSpacing()
{
    //ininfoLabel->setText(tr("Invoked <b>Edit|Format|Set Paragraph Spacing</b>"));
}

void MainWindow::about()
{
    //infoLabel->setText(tr("Invoked <b>Help|About</b>"));
    QMessageBox::about(this, tr("About Menu"),
            tr("The <b>Menu</b> example shows how to create "
               "menu-bar menus and context menus."));
}

void MainWindow::aboutQt()
{
    //ininfoLabel->setText(tr("Invoked <b>Help|About Qt</b>"));
}
