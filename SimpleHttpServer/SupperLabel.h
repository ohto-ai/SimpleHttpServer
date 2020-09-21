#pragma once

#include <QLabel>
#include <QMouseEvent>

class SupperLabel : public QLabel
{
	Q_OBJECT
protected:
	virtual void mousePressEvent(QMouseEvent* e) override;

public:
	SupperLabel(QWidget *parent);
	~SupperLabel();
signals:
	void clicked();
	void leftClicked();
	void rightClicked();
	void middleClicked();
};
