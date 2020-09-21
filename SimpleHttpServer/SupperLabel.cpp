#include "SupperLabel.h"

SupperLabel::SupperLabel(QWidget *parent)
	: QLabel(parent)
{
}

void SupperLabel::mousePressEvent(QMouseEvent* e)
{
    if (e->button() & Qt::LeftButton) {
        emit leftClicked();
        emit clicked();
    }
    else if (e->button() & Qt::RightButton)
    {
        emit rightClicked();
        emit clicked();
    }
    else if (e->button() & Qt::MiddleButton)
    {
        emit middleClicked();
        emit clicked();
    }
    QLabel::mousePressEvent(e);
}
SupperLabel::~SupperLabel()
{
}
