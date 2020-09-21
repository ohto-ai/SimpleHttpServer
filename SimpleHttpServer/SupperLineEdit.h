#pragma once

#include <QLineEdit>
#include <QMouseEvent>

class SupperLineEdit : public QLineEdit
{
	Q_OBJECT

protected:
	virtual void mousePressEvent(QMouseEvent* e) override;
public:
	SupperLineEdit(QWidget *parent);
	~SupperLineEdit();

signals:
	void clicked();
	void leftClicked();
	void rightClicked();
	void middleClicked();
};
