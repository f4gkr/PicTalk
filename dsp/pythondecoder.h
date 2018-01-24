#ifndef PYTHONDECODER_H
#define PYTHONDECODER_H


#include <QObject>
#include <QThread>
#include <stdio.h>

class ZmqPython : public QThread
{
    Q_OBJECT
public:
    explicit ZmqPython(QObject *parent=0);

signals:
    void message( QString msg );

private:
    void run();
};

class PythonDecoder : public QThread
{
    Q_OBJECT
public:
    explicit PythonDecoder(QObject *parent = nullptr);

signals:
    void pythonStarts();
    void pythonEnds();

public slots:
private:
    QString mPythonScript ;
    FILE *code ;
    ZmqPython* zmq ;

    void run();
};

#endif // PYTHONDECODER_H
