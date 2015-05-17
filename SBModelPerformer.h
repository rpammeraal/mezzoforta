#ifndef SBMODELPERFORMER_H
#define SBMODELPERFORMER_H


class SBModelPerformer
{
public:
    static SBID getDetail(const SBID& id);
    static SBModelSonglist* getAllAlbums(const SBID& id);
    static SBModelSonglist* getAllCharts(const SBID& id);
    static SBModelSonglist* getAllSongs(const SBID& id);
};

#endif // SBMODELPERFORMER_H
