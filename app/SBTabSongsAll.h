#ifndef SBTABSONGSALL_H
#define SBTABSONGSALL_H

#include "SBTab.h"

class SBTabSongsAll : public SBTab
{
    Q_OBJECT

public:
    SBTabSongsAll();

    //	Virtual
    virtual bool handleEscapeKey() const;	//	return 1 when currentTab can be closed
    virtual SBID populate(const SBID& id);
};

#endif // SBTABSONGSALL_H
