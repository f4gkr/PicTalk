#ifndef AIRSPYWIDGET_H
#define AIRSPYWIDGET_H

#include <QWidget>
#include "ui/gkdial.h"
#include "hardware/airspy/airspy.h"

class AirspyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AirspyWidget(struct airspy_device* device,
                          QWidget *parent = nullptr);

signals:

public slots:
    void SLOT_setLNAGain(int g) ;
    void SLOT_setVGAGain(int g) ;
    void SLOT_setMIXGain(int g) ;

private:
    struct airspy_device* m_device ;
    gkDial *lnaGain ;
    gkDial *vgaGain ;
    gkDial *mixerGain ;
};

#endif // AIRSPYWIDGET_H
