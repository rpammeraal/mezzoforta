#ifndef DISPLAYONLYDELEGATE_H
#define DISPLAYONLYDELEGATE_H

#include <QStyledItemDelegate>

class DisplayOnlyDelegate : public QStyledItemDelegate
{
public:
    DisplayOnlyDelegate(int c);
    ~DisplayOnlyDelegate();

    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;

private:
    int disabledEditColumn;
};

#endif // DISPLAYONLYDELEGATE_H
