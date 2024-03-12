#ifndef SBHTMLCHARTSALL_H
#define SBHTMLCHARTSALL_H

#include <QString>

class SBHtmlChartsAll
{
public:
    SBHtmlChartsAll();

    static QString retrieveAllCharts(const QChar& startsWith, qsizetype offset=0, qsizetype size=0);
};

#endif // SBHTMLCHARTSALL_H
