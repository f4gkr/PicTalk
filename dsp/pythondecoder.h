#ifndef PYTHONDECODER_H
#define PYTHONDECODER_H


#include <QObject>
#include <QThread>

class PythonDecoder : public QThread
{
    Q_OBJECT
public:
    explicit PythonDecoder(QString scriptName, QObject *parent = nullptr);
    void run();
signals:

public slots:
private:
    QString mPythonScript ;

};

#endif // PYTHONDECODER_H
