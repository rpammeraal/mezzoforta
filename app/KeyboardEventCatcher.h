#ifndef KEYBOARDEVENTCATCHER_H
#define KEYBOARDEVENTCATCHER_H

#include <QObject>
#include <QAbstractNativeEventFilter>

//class KeyboardEventCatcher : public QAbstractNativeEventFilter, public QObject
class KeyboardEventCatcher : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    KeyboardEventCatcher();
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result);

signals:
    void songNext();
    void songPlayPauseToggle();
    void songPrevious();
    void volumeLouder();
    void volumeMuteToggle();
    void volumeSofter();

protected:
    friend class Context;
    void doInit();	//	Init done by Context::

private:
    void _init();
};

#endif // KEYBOARDEVENTCATCHER_H
