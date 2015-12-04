#ifndef COMPLETERFACTORY_H
#define COMPLETERFACTORY_H

class QCompleter;
class QSqlQueryModel;
class QString;

class CompleterFactory
{
public:
    static QCompleter* getCompleterAll();
    static QCompleter* getCompleterAlbum();
    static QCompleter* getCompleterPerformer();
    static QCompleter* getCompleterPlaylist();
    static QCompleter* getCompleterSong();

private:
    static QCompleter* createCompleter(QString& query);
};

#endif // COMPLETERFACTORY_H
