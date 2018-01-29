#include "MyImageWidget.h"
#include <QDebug>
MyImageWidget::MyImageWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_keyPressed = false;
	this->setFocusPolicy(Qt::StrongFocus);//响应键盘事件
}

void MyImageWidget::setImage(const QImage &image)
{
	m_srcImage = QPixmap::fromImage(image);
	QString fmtSizeString;
	fmtSizeString = QString::number(image.size().width()) + QStringLiteral(" × ")
		+ QString::number(image.size().height()) + "    " + Format2QString(image.format());
	ui.labelInformation->setText(fmtSizeString);
	ui.labelImage->setPixmap(m_srcImage);
}

void MyImageWidget::mousePressEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
	{ui.labelImage->setPixmap(m_srcImage);
	m_keyPressed = true;
	if (ui.labelImage->pixmap()->size() == m_srcImage.size())
	{
		QPoint posOnImage = caculatePosOnImage(e->pos());
		m_positionStart = posOnImage;
	}
	break; }
	case Qt::RightButton:
	{QPoint t_posOnImage = caculatePosOnImage(e->pos());
	if (!t_posOnImage.isNull())
	{
		QRgb pixColor = ui.labelImage->pixmap()->toImage().pixel(t_posOnImage);
		QString posStr = QString("(%1,%2)").arg(t_posOnImage.x()).arg(t_posOnImage.y());

		QString colorStr = QString("R:%1,G:%2,B:%3     Gray:%4").arg(qRed(pixColor)).arg(qGreen(pixColor)).arg(qBlue(pixColor)).arg(qGray(pixColor));
		ui.labelStatus->setText(posStr + "   " + colorStr);
	}
	break; }
	default:
		ui.labelImage->setPixmap(m_srcImage);
		break;
	}
}

void MyImageWidget::mouseMoveEvent(QMouseEvent *e)
{
	if (m_keyPressed)
	{
		QPoint posOnImage = caculatePosOnImage(e->pos());
		if (!posOnImage.isNull())
		{
			m_positionMoving = posOnImage;
			MyBox(m_positionStart, m_positionMoving);
			ui.labelImage->setPixmap(m_tempPaintImage);
		}
	}

}

void MyImageWidget::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
	{
		QPoint posOnImage = caculatePosOnImage(e->pos());
		if (!posOnImage.isNull())
		{
			m_positionReleased = posOnImage;
			//////////////////////////////////////////////////////////////////////////
			//判断选框大小大于10个像素
			if ((m_positionReleased.x() - m_positionStart.x()) >= 10 &&
				(m_positionReleased.y() - m_positionStart.y()) >= 10)
				//////////////////////////////////////////////////////////////////////////
			{
				MyBox(m_positionStart, m_positionReleased);
				QPixmap RoiPix = m_srcImage.copy(QRect(m_positionStart, m_positionReleased));
				ui.labelImage->setPixmap(RoiPix.scaled(RoiPix.size().expandedTo(ui.labelImage->size()), Qt::KeepAspectRatio, Qt::FastTransformation));
			}


		}
		m_keyPressed = false;
	}
}

//===============================================================

// @brief:  新加空格恢复原图操作
// 2017.11.29

//===============================================================
void MyImageWidget::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_Space:
	{ui.labelImage->setPixmap(m_srcImage);
	break; }
	default:
		break;
	}
}

void MyImageWidget::paintEvent(QPaintEvent* e)
{
	m_tempPaintImage = m_srcImage.copy();
	if (!m_tempPaintImage.isNull())
	{
		QPainter painter(&m_tempPaintImage);
		painter.setBrush(Qt::red);
		painter.setPen(Qt::red);
		painter.drawLines(m_Box);
	}

}

QString MyImageWidget::Format2QString(const int &format)
{
	switch (format)
	{
	case QImage::Format_Grayscale8:
		return QStringLiteral("灰度图像");
	case QImage::Format_RGB16:
	case QImage::Format_RGB30:
	case QImage::Format_RGB32:
	case QImage::Format_RGB444:
	case QImage::Format_RGB555:
	case QImage::Format_RGB666:
	case QImage::Format_RGB888:
		return QStringLiteral("RGB图像");
	case QImage::Format_BGR30:
		return QStringLiteral("BGR图像");
	default:
		return QStringLiteral("未知格式图像");
		break;
	}
}

void MyImageWidget::MyBox(const QPoint &a, const QPoint &b)
{
	m_Box.clear();
	int __xLeft = a.x();
	int __yUp = a.y();
	int __xRight = b.x();
	int __yDown = b.y();
	QPoint __leftUp(__xLeft, __yUp);
	QPoint __leftDown(__xLeft, __yDown);
	QPoint __rightUp(__xRight, __yUp);
	QPoint __rightDown(__xRight, __yDown);
	m_Box.push_back(QLine(__leftUp, __leftDown));
	m_Box.push_back(QLine(__leftUp, __rightUp));
	m_Box.push_back(QLine(__leftDown, __rightDown));
	m_Box.push_back(QLine(__rightUp, __rightDown));
}

QPoint MyImageWidget::caculatePosOnImage(const QPoint &posOnWidget)
{
	int xoffset = (ui.labelImage->contentsRect().width() - ui.labelImage->pixmap()->rect().width()) / 2;
	int yoffset = (ui.labelImage->contentsRect().height() - ui.labelImage->pixmap()->rect().height()) / 2;
	QPoint posOnImage = ui.labelImage->mapFrom(this, posOnWidget) - QPoint(xoffset, yoffset);//ImageLabel坐标
	if (posOnImage.x() >= 0 && posOnImage.y() >= 0 &&
		posOnImage.x() <= ui.labelImage->pixmap()->rect().width() &&
		posOnImage.y() <= ui.labelImage->pixmap()->rect().height()
		)
	{
		return posOnImage;
	}
	return QPoint();
}
