#include <Qt>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>

#include "SBLabel.h"

#include "Common.h"
#include "SBID.h"

SBLabel::SBLabel()
{

}


SBLabel::SBLabel(QWidget* parent, Qt::WindowFlags f) : QLabel(parent,f)
{

}

SBLabel::SBLabel(const QString& text, QWidget* parent, Qt::WindowFlags f) : QLabel(text,parent,f)
{

}

void
SBLabel::mousePressEvent(QMouseEvent* me)
{
    qDebug() << SB_DEBUG_INFO << _id;
    if (me->button() == Qt::LeftButton && this->geometry().contains(me->pos()))
    {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        QByteArray ba=_id.encode();
        mimeData->setData("application/vnd.text.list", ba);
        drag->setMimeData(mimeData);
        const QPixmap* pm=this->pixmap();
        if(pm && pm->isNull()==0)
        {
            qDebug() << SB_DEBUG_INFO;
            drag->setPixmap(pm->scaledToWidth(50));
        }
        else
        {
            QString l=_id.getIconResourceLocation();
            QPixmap pb(l);
            int result=pb.load(l);
            qDebug() << SB_DEBUG_INFO << l << result;
            drag->setPixmap(pb.scaledToWidth(50));
        }
        drag->exec();
    }
}

void
SBLabel::setSBID(const SBID &id)
{
    _id=id;
    qDebug() << SB_DEBUG_INFO << this << _id;
}
