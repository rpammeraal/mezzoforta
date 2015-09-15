#ifndef SBTABSONGEDIT_H
#define SBTABSONGEDIT_H

#include "SBTab.h"

class SBTabSongEdit : public SBTab
{
    Q_OBJECT

public:
    SBTabSongEdit();

    virtual void handleEnterKey() const;	//	return 1 when tab can be closed
    virtual bool hasEdits() const;
    virtual SBID populate(const SBID& id);

public slots:
    virtual void save() const;	//	return 1 if tab can be closed
};

#endif // SBTABSONGEDIT_H
