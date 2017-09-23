#ifndef COMPLETERFACTORY_H
#define COMPLETERFACTORY_H

#include <QAbstractTableModel>
#include <QObject>

class QCompleter;
class QSqlQueryModel;
class QString;


class CompleterFactory : public QObject
{
    Q_OBJECT

public:
    QCompleter* getCompleterAll();
    QCompleter* getCompleterAlbum();
    QCompleter* getCompleterPerformer();
    QCompleter* getCompleterPlaylist();
    QCompleter* getCompleterSong();

private:
    static QCompleter* _createCompleter(QString& query);
    static QCompleter* _instantiateCompleter(QAbstractTableModel* atm);
};

#endif // COMPLETERFACTORY_H
