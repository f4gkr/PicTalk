#include "gkpushbutton.h"

gkPushButton::gkPushButton(QWidget *parent) :
    QToolButton(parent)
{
    m_on = false ;
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

}


gkPushButton::gkPushButton(const QString &text, QWidget *parent) :
    QToolButton(parent)
{
    setText(text+" ");
    setStatus(false);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

void gkPushButton::setStatus(bool on) {
    m_on = on ;
    if( m_on ) {
        QPixmap pixmap(":/buttons/on");
        setIconSize(pixmap.rect().size());
        setIcon( QIcon( pixmap )) ;
        return ;
    }
    QPixmap pixmap(":/buttons/off");
    setIconSize(pixmap.rect().size());
    setIcon( QIcon( pixmap )) ;
    return ;
}

