#include "MyCalibTools.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

MyCalibTools::MyCalibTools(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

void MyCalibTools::on_actionOpen_triggered()
{
	m_imagePath_ = QFileDialog::getOpenFileName(this, QStringLiteral("打开图片文件"), ".", "Image(*.bmp *.png *.jpg)");
	if (m_imagePath_ == "")
	{
		QMessageBox::warning(this, QStringLiteral("未选择图片"), QStringLiteral("未选择图片"));
		return;
	}
	m_imageSrc_ = cv::imread(std::string(m_imagePath_.toLocal8Bit()));
	cv::cvtColor(m_imageSrc_, m_imageSrc_, CV_BGR2GRAY);
	GaussianBlur(m_imageSrc_, m_imageSrc_, cv::Size(3, 3), 0);
	cv::Mat m_image_canny_;
	cv::Canny(m_imageSrc_, m_image_canny_, 100, 300);
	std::vector <std::vector<cv::Point>> m_contours_;//边界
	std::vector <cv::Vec4i> m_hierarchy_;//边界序号 **在这里没有用到
	//获取边界
	cv::findContours(m_image_canny_, m_contours_, cv::RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	//去掉外边界
	while (m_contours_.size() < 5)
	{
		for (int i = 0; i < m_contours_.size(); ++i)
		{
			cv::drawContours(m_image_canny_, m_contours_, i, cv::Scalar(0));
		}
		cv::findContours(m_image_canny_, m_contours_, cv::RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	}
	int sum = 0;
	int valid_num = 0;
	for (int i = 0; i < m_contours_.size(); ++i)
	{
		if (m_contours_[i].size() < 1000)
		{
			valid_num++;
			sum += m_contours_[i].size();
		}
	}
	double avg = sum / valid_num;
	//遍历寻找大圆
	std::vector<int> idx_big;
	for (int i = 0; i < m_contours_.size(); ++i)
	{
		if (m_contours_[i].size() > avg*1.5 && m_contours_[i].size() < 1000)
		{
			idx_big.push_back(i);
		}
	}
	for (int i = 0; i < idx_big.size(); ++i)
	{
		cv::drawContours(m_image_canny_, m_contours_, idx_big[i], cv::Scalar(125));
	}

	ui.widget->setImage(Mat2QImage(m_image_canny_));
}


QImage MyCalibTools::Mat2QImage(const cv::Mat& mat)
{
	// 8-bits unsigned, NO. OF CHANNELS = 1
	if (mat.type() == CV_8UC1)
	{
		QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
		// Set the color table (used to translate colour indexes to qRgb values)
		image.setColorCount(256);
		for (int i = 0; i < 256; i++)
		{
			image.setColor(i, qRgb(i, i, i));
		}
		// Copy input Mat
		uchar *pSrc = mat.data;
		for (int row = 0; row < mat.rows; row++)
		{
			uchar *pDest = image.scanLine(row);
			memcpy(pDest, pSrc, mat.cols);
			pSrc += mat.step;
		}
		return image;
	}
	// 8-bits unsigned, NO. OF CHANNELS = 3
	else if (mat.type() == CV_8UC3)
	{
		// Copy input Mat
		const uchar *pSrc = (const uchar*)mat.data;
		// Create QImage with same dimensions as input Mat
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
		return image.rgbSwapped();
	}
	else if (mat.type() == CV_8UC4)
	{
		qDebug() << "CV_8UC4";
		// Copy input Mat
		const uchar *pSrc = (const uchar*)mat.data;
		// Create QImage with same dimensions as input Mat
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
		return image.copy();
	}
	else
	{
		qDebug() << "ERROR: Mat could not be converted to QImage.";
		return QImage();
	}
}
