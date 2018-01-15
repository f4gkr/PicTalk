#ifndef TUNINGPOLICY_H
#define TUNINGPOLICY_H

#include <QObject>

class TuningPolicy : public QObject
{
    Q_OBJECT
public:
    explicit TuningPolicy();

    qint64 rx_hardware_frequency ;
    int channelizer_offset ;
};

#endif // TUNINGPOLICY_H
