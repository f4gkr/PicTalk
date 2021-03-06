#include "webservice.h"
#include <QDebug>


WebService::WebService(QObject *parent):
    HttpRequestHandler(parent)
{
    sem = new QSemaphore(1);
    radio_on = false ;
    frequency = 0 ;
}

void WebService::repJson( HttpResponse& response, QJsonDocument &doc) {
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Cache-Control", "no-cache, no-store, must-revalidate" );
    response.setHeader("Pragma", "no-cache");
    response.setHeader("Expires", 0);
    response.write(doc.toJson(), true);
}

void WebService::reportStatus( bool on, qint64 frequency ) {
    sem->acquire(1);
    this->radio_on = on ;
    this->frequency = frequency ;
    sem->release(1);
}


void WebService::service(HttpRequest &request, HttpResponse &response) {
    QByteArray path=request.getPath();

    qDebug() << "WebService::service for " << path ;

    QStringList parts = QString(path).split("/");
    int L = parts.size() ;

    if( L == 2 ) {
        if( "start" == parts[1] ) {
            QJsonObject *reply = turnOn();
            QJsonDocument doc(*reply);
            repJson( response, doc);
            delete reply ;
            return ;
        }

        if( "stop" == parts[1] ) {
            emit turnOn();
            QJsonObject *reply = turnOff();
            QJsonDocument doc(*reply);
            repJson( response, doc);
            delete reply ;
            return ;
        }

        if( "status" == parts[1] ) {
            QJsonObject *reply = getStatus();
            QJsonDocument doc(*reply);
            repJson( response, doc);
            delete reply ;
            return ;
        }

        qDebug() << "parts[1]" << parts[1];
    }

    if( L == 3 ) {
        if( "tune" == parts[1] ) {
            if( parts[2] != NULL ) {
                QString f = parts[2] ;
                if( f.toDouble() > 0 ) {
                    qint64 tuneRequest = (qint64)(f.toDouble() * 1e6 );
                    QJsonObject *reply = tuneTo(tuneRequest);
                    QJsonDocument doc(*reply);
                    repJson( response, doc);
                    delete reply ;
                    return ;
                }
            }
        }
    }

    response.setStatus( 200, "Ok");
    response.write("not found", true);
    return ;
}

QJsonObject * WebService::turnOn() {
    emit mturnOn();
    QJsonObject *ss = new QJsonObject();
    ss->insert("request", QJsonValue("start")) ;
    ss->insert("status", QJsonValue("ok"));

    return(ss);
}

QJsonObject * WebService::turnOff() {
    emit mturnOn();
    QJsonObject *ss = new QJsonObject();
    ss->insert("request", QJsonValue("stop")) ;
    ss->insert("status", QJsonValue("ok"));
    return(ss);
}

QJsonObject * WebService::tuneTo(qint64 f) {
    emit mtuneTo(f);
    QJsonObject *ss = new QJsonObject();
    ss->insert("request", QJsonValue("tune"));
    ss->insert("frequency", QJsonValue( QString::number( ((double)f)/1e6,'f',6) ));
    ss->insert("status", QJsonValue("ok"));
    return(ss);
}

QJsonObject * WebService::getStatus() {
    QJsonObject *ss = new QJsonObject();
    ss->insert("request", QJsonValue("status"));
    ss->insert("status", QJsonValue("ok"));
    sem->acquire(1);
    ss->insert("rx_frequency", QJsonValue( QString::number( ((double)frequency)/1e6,'f',6) ));
    ss->insert("radio_on", QJsonValue( radio_on ? QString("on"):QString("off") ));
    sem->release(1);
    return(ss);
}
