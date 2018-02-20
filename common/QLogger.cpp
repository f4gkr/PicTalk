#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QTextStream>
#include "QLogger.h"

/****************************************************************************************
 ** QLogger is a library to register and print logs into a file.
 ** Copyright (C) 2013  Francesc Martinez <es.linkedin.com/in/cescmm/en>
 **
 ** This library is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License as published by the Free Software Foundation; either
 ** version 2.1 of the License, or (at your option) any later version.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/
#include <cstdarg>
#include <QStandardPaths>
namespace QLogger
{
    void QLog_Trace(const QString &module, const QString &message)
    {
        QLog_(module, TraceLevel, message);
    }

    void QLog_VATrace(const QString &module, const char* message,...)  {
        char newMessage[1024]; // **Make sure the buffer is large enough**
        va_list args;
        va_start(args, message);
        vsnprintf(newMessage, 1023, message, args);
        va_end( args );
        QLog_(module, TraceLevel, QString::fromLocal8Bit( newMessage ));
    }

    void QLog_Debug(const QString &module, const QString &message)
    {
        QLog_(module, DebugLevel, message);
    }

    void QLog_Info(const QString &module, const QString &message)
    {
        QLog_(module, InfoLevel, message);
    }

    void QLog_Warning(const QString &module, const QString &message)
    {
        QLog_(module, WarnLevel, message);
    }

    void QLog_Error(const QString &module, const QString &message)
    {
        QLog_(module, ErrorLevel, message);
    }

    void QLog_Fatal(const QString &module, const QString &message)
    {
        QLog_(module, FatalLevel, message);
    }

    void QLog_(const QString &module, LogLevel level, const QString &message)
    {
        QLoggerManager *manager = QLoggerManager::getInstance();

        QMutexLocker(&manager->mutex);

        QLoggerWriter *logWriter = manager->getLogWriter(module);

        if (logWriter and logWriter->getLevel() <= level) {
                logWriter->write(module,message);
        }

    }

    //QLoggerManager
    QLoggerManager * QLoggerManager::INSTANCE = NULL;

    QLoggerManager::QLoggerManager() : QThread(), mutex(QMutex::Recursive)
    {
        setObjectName("QLoggerManager");
        start();
    }

    QLoggerManager * QLoggerManager::getInstance()
    {
        if (!INSTANCE)
            INSTANCE = new QLoggerManager();

        return INSTANCE;
    }

    QString QLoggerManager::levelToText(const LogLevel &level)
    {
        switch (level)
        {
            case TraceLevel: return "Trace";
            case DebugLevel: return "Debug";
            case InfoLevel: return "Info";
            case WarnLevel: return "Warning";
            case ErrorLevel: return "Error";
            case FatalLevel: return "Fatal";
            default: return QString();
        }
    }

    bool QLoggerManager::addDestination(const QString &fileDest, const QString &module, LogLevel level)
    {
        QLoggerWriter *log;

        if (!moduleDest.contains(module))
        {
            log = new QLoggerWriter(fileDest,level);
            moduleDest.insert(module, log);
            return true;
        }

        return false;
    }

    bool QLoggerManager::addDestination(const QString &fileDest, const QStringList &modules, LogLevel level)
    {
        QLoggerWriter *log;
        foreach (QString module, modules)
        {
            if (!moduleDest.contains(module))
            {
                log = new QLoggerWriter(fileDest,level);
                moduleDest.insert(module, log);
                return true;
            }
        }
        return false;
    }

    void QLoggerManager::closeLogger()
    {
        exit(0);
        deleteLater();
    }

    QLoggerWriter::QLoggerWriter(const QString &fileDestination, LogLevel level)
    {
        m_fileDestination = fileDestination;
        m_level = level;
    }

    void QLoggerWriter::write(const QString &module, const QString &message)
    {
        QString _fileName = m_fileDestination;

        int MAX_SIZE = 1024 * 1024;

        QDir dir( QStandardPaths::writableLocation( QStandardPaths::HomeLocation) );
        if (!dir.exists("pictalk") )
            dir.mkdir("pictalk");

        QFile file(_fileName);
        QString toRemove = _fileName.section('.',-1);
        QString fileNameAux = _fileName.left(_fileName.size() - toRemove.size()-1);
        bool renamed = false;
        QString newName = fileNameAux + "_%1__%2.log";

        //Renomenem l'arxiu si està ple
        if (file.size() >= MAX_SIZE)
        {
            //Creem un fixer nou
            QDateTime currentTime = QDateTime::currentDateTime();
            newName = newName.arg(currentTime.date().toString("dd_MM_yy")).arg(currentTime.time().toString("hh_mm_ss"));
            renamed = file.rename(_fileName, newName);

        }
        _fileName = QStandardPaths::writableLocation( QStandardPaths::HomeLocation) + "/pictalk/" + _fileName ;
        file.setFileName(_fileName);
        if (file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append))
        {
            QTextStream out(&file);
            QString dtFormat = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss.zzz");

            if (renamed)
                out << QString("%1 - Previuous log %2\n").arg(dtFormat).arg(newName);

            QString logLevel = QLoggerManager::levelToText(m_level);
            QString text = QString("[%1] [%2] {%3} %4\n").arg(dtFormat).arg(logLevel).arg(module).arg(message);
            out << text;
            file.close();
        }
    }
}
