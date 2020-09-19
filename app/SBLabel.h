#ifndef SBLABEL_H
#define SBLABEL_H

#include <QObject>
#include <QLabel>

#include "SBIDBase.h"

class QMouseEvent;


class SBLabel : public QLabel
{
    Q_OBJECT

public:
    SBLabel();
    SBLabel(QWidget* parent, Qt::WindowFlags f=Qt::WindowFlags());
    SBLabel(const QString& text, QWidget* parent=0, Qt::WindowFlags f=Qt::WindowFlags());

    //	Inherited methods
    virtual void mousePressEvent(QMouseEvent* me);
    void setKey(SBKey key);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *ev);

private:
    SBKey _key;

};

#endif // SBLABEL_H
