#ifndef SBTABSONGSALL_H
#define SBTABSONGSALL_H

#include "SBTab.h"

class SBTabSongsAll : public SBTab
{
    Q_OBJECT

public:
    SBTabSongsAll();

    //	Virtual
    virtual bool handleEscapeKey();
    virtual SBID populate(const SBID& id);
};

#endif // SBTABSONGSALL_H
