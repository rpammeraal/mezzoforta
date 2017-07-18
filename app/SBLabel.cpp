#include <Qt>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>

#include "SBLabel.h"

#include "Common.h"
#include "SBIDBase.h"

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
    if (me->button() == Qt::LeftButton && this->geometry().contains(me->pos()))
    {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        SB_RETURN_VOID_IF_NULL(_ptr);
        QByteArray ba=_ptr->encode();

        mimeData->setData("application/vnd.text.list", ba);
        drag->setMimeData(mimeData);
        const QPixmap* pm=this->pixmap();
        if(pm && pm->isNull()==0)
        {
            drag->setPixmap(pm->scaledToWidth(50));
        }
        else
        {
            QString l=_ptr->iconResourceLocation();
            QPixmap pb(l);
            pb.load(l);
            drag->setPixmap(pb.scaledToWidth(50));
        }
        drag->exec();
    }
}

void
SBLabel::setPtr(const SBIDPtr& ptr)
{
    _ptr=ptr;
}

///	Protected methods
void
SBLabel::contextMenuEvent(QContextMenuEvent *ev)
{
    emit customContextMenuRequested(ev->globalPos());
}
