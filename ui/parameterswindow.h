#ifndef PARAMETERSWINDOW_H
#define PARAMETERSWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>

class ParametersWindow : public QDialog
{
    Q_OBJECT
public:
    explicit ParametersWindow(QWidget *parent = nullptr);

signals:

public slots:
private slots:
    void SLOT_save();
private:
    QLineEdit *sCallSign ;
    QLineEdit *sLatitude;
    QLineEdit *sLongitude;
    bool mOK ;
};

#endif // PARAMETERSWINDOW_H
