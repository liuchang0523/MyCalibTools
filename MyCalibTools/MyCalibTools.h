#ifndef MyCalibTools_h__
#define MyCalibTools_h__


#include <QtWidgets/QMainWindow>
#include "ui_MyCalibTools.h"
#include <opencv2/opencv.hpp>

class MyCalibTools : public QMainWindow
{
	Q_OBJECT
public:
	MyCalibTools(QWidget *parent = Q_NULLPTR);

	private slots:
	void on_actionOpen_triggered();

	QImage Mat2QImage(const cv::Mat& mat);
private:
	Ui::MyCalibToolsClass ui;
	//图片路径
	QString m_imagePath_;
	cv::Mat m_imageSrc_;
};
#endif // MyCalibTools_h__
