#include "SupperLineEdit.h"

SupperLineEdit::SupperLineEdit(QWidget *parent)
	: QLineEdit(parent)
{
}
void SupperLineEdit::mousePressEvent(QMouseEvent* e)
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
    QLineEdit::mousePressEvent(e);
}
SupperLineEdit::~SupperLineEdit()
{
}
