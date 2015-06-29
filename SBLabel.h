#ifndef SBLABEL_H
#define SBLABEL_H

#include <QObject>
#include <QLabel>

#include "SBID.h"

class QMouseEvent;


class SBLabel : public QLabel
{
    Q_OBJECT

public:
    SBLabel();
    SBLabel(QWidget* parent, Qt::WindowFlags f=0);
    SBLabel(const QString& text, QWidget* parent=0, Qt::WindowFlags f=0);

    //	Inherited methods
    virtual void mousePressEvent(QMouseEvent* me);
    void setSBID(const SBID& nid);

private:
    SBID id;
};

#endif // SBLABEL_H
