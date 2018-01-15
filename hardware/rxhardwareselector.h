#ifndef RXHARDWARESELECTOR_H
#define RXHARDWARESELECTOR_H

#include <QObject>
#include "hardware/rxdevice.h"


class RxHardwareSelector : public QObject
{
    Q_OBJECT
public:

    explicit RxHardwareSelector(QObject *parent = 0);
    RxDevice *getReceiver() ;

signals:

public slots:
};

#endif // RXHARDWARESELECTOR_H
