#ifndef MyImageWidget_h__
#define MyImageWidget_h__


#include <QtWidgets/QWidget>
#include "ui_MyImageWidget.h"
#include <QMouseEvent>
#include <QRgb>
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>

class MyImageWidget : public QWidget
{
	Q_OBJECT

public:
	MyImageWidget(QWidget *parent = Q_NULLPTR);
	void setImage(const QImage &image);

protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void keyPressEvent(QKeyEvent *);
	void paintEvent(QPaintEvent* e);

private:
	Ui::MyImageWidgetClass ui;

	QPixmap m_srcImage;
	QPixmap m_tempPaintImage;
	QPoint m_positionStart, m_positionMoving, m_positionReleased;
	QVector<QLine> m_Box;
	QString Format2QString(const int&);
	void MyBox(const QPoint &a, const QPoint &b);
	bool m_keyPressed;
	QPoint caculatePosOnImage(const QPoint &posOnWidget);
};
#endif // MyImageWidget_h__
