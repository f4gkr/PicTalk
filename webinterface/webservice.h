#ifndef WEBSERVICE_H
#define WEBSERVICE_H

#include <QObject>
#include <QDebug>
#include <QSemaphore>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "httprequesthandler.h"

class WebService : public HttpRequestHandler
{
    Q_OBJECT
public:
    explicit WebService( QObject *parent = 0);
    void service(HttpRequest& request, HttpResponse& response);


public slots:
    void reportStatus( bool on, qint64 frequency );

signals:
    void mturnOn();
    void mturnOff();
    void mtuneTo( qint64 frequency );


private:
    void repJson( HttpResponse& response, QJsonDocument &doc) ;

    QJsonObject * turnOn();
    QJsonObject * turnOff();
    QJsonObject * tuneTo(qint64 f);
    QJsonObject * getStatus();

    // status to be reported by json request
    bool radio_on ;
    qint64 frequency ;
    QSemaphore *sem ;
};

#endif // WEBSERVICE_H
