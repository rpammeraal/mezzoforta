#ifndef LEFTCOLUMNCHOOSER_H
#define LEFTCOLUMNCHOOSER_H

#include <QList>
#include <QObject>

#include <SBID.h>

class QStandardItem;
class QStandardItemModel;

class LeftColumnChooser : public QObject
{
    Q_OBJECT
public:
    static QStandardItemModel* getModel();

public:

signals:

public slots:

private:
    static QList<QStandardItem *> createNode(const QString& itemValue, int itemID, SBID::sb_type type);
};

#endif // LEFTCOLUMNCHOOSER_H
