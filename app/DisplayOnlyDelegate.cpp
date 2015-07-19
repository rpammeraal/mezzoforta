#include "DisplayOnlyDelegate.h"

#include <QDebug>

DisplayOnlyDelegate::DisplayOnlyDelegate(int c):QStyledItemDelegate()
{
    disabledEditColumn=c;
}

DisplayOnlyDelegate::~DisplayOnlyDelegate()
{

}

QWidget *
DisplayOnlyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if(index.column()!=disabledEditColumn)
    {
        return QStyledItemDelegate::createEditor(parent,option,index);
    }
    return NULL;
}
