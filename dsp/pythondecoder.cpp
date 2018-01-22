#include <cmath>
#include <Python.h>
#include "pythondecoder.h"
#include <QByteArray>
#include <QDebug>
PythonDecoder::PythonDecoder(QString scriptName, QObject *parent) : QThread(parent)
{
    mPythonScript = scriptName ;
    QString pythonHome = "Python3.6" ;
    wchar_t array[1024];
    pythonHome.toWCharArray(array);
    Py_SetPythonHome( array) ;
    Py_Initialize();
    PyRun_SimpleString("from time import time,ctime\n"
                         "print 'Today is',ctime(time())\n");
       //cerr << Py_GetPath() << endl;
    Py_Finalize();
}

void PythonDecoder::run() {
    PyObject *pName, *pModule, *pFunc;
    PyObject *pValue;

    QByteArray d = mPythonScript.toLocal8Bit() ;
    QString pythonHome = "C:\\python3.6" ;
    wchar_t array[1024];
    pythonHome.toWCharArray(array);
    Py_SetPythonHome( array) ;

    QString pythonPath = "'C:\\Python36\\python36.zip', 'C:\\Python36\\DLLs', 'C:\\Python36\\lib', 'C:\\Python36', 'C:\\Python36\\lib\\site-packages'" ;
    wchar_t array2[1024];
    pythonPath.toWCharArray(array2);
    Py_SetPath(array2);

    Py_InitializeEx(0);
    pName = PyUnicode_DecodeFSDefault( (const char *)d.data() );
    if( pName == NULL ) {
        qDebug() << "cannot find " << mPythonScript ;
        return ;
    }
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule == NULL) {
        qDebug() << "error  " << mPythonScript << "PyImport_Import() -> null" ;
        return ;
    }
    pFunc = PyObject_GetAttrString(pModule, (const char *)"decoderMain");
    if ((pFunc==NULL) || PyCallable_Check(pFunc)) {
        qDebug() << "error  " << mPythonScript << "PyCallable_Check()" ;
        return ;
    }

    pValue = PyObject_CallObject(pFunc, NULL);

    if (pValue != NULL) {
        printf("Result of call: %ld\n", PyLong_AsLong(pValue));
    }
    else {
        Py_DECREF(pFunc);
        Py_DECREF(pModule);
        PyErr_Print();
        fprintf(stderr,"Call failed\n");
        return ;
    }
    Py_XDECREF(pFunc);
    Py_DECREF(pModule);
    Py_FinalizeEx();
}
