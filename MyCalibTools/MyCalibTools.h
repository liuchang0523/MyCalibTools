﻿#ifndef MyCalibTools_h__
#define MyCalibTools_h__


#include <QtWidgets/QMainWindow>
#include "ui_MyCalibTools.h"
#include <opencv2/opencv.hpp>
#include <QtCore\qxmlstream.h>

class MyCalibTools : public QMainWindow
{
	Q_OBJECT
public:
	MyCalibTools(QWidget *parent = Q_NULLPTR);

	private slots:
	void on_actionOpen_triggered();
	void on_actionOpenList_triggered();
	void on_actionCalibration_triggered();
	void on_actionStereoCalibrate_triggered();
	void on_actionMatching_triggered();
	void on_actionLoadAndMatching_triggered();
	void on_actionXML_triggered();

private:
	Ui::MyCalibToolsClass ui;
	//图片路径
	QString m_imagePath_;
	QStringList m_image_list_;
	QString m_parent_path_;//记录上一次打开图像的路径
	cv::Mat m_image_source_;

	//圆心坐标
	std::vector<std::vector<cv::Point2f>> m_points_on_image_;
	std::vector<std::vector<cv::Point3f>> m_points_on_world_;


	//标定参数
	cv::Mat m_matrix_;
	cv::Mat m_distortion_;
	std::vector<cv::Mat> m_R_;
	std::vector<cv::Mat> m_T_;
	std::vector<double> m_errors_;

	//立体视觉标定参数
	cv::Mat matrix_1;
	cv::Mat dist_1;
	cv::Mat matrix_2;
	cv::Mat dist_2;
	cv::Mat R, T, E, F;
	bool m_stereo_finished;

private:
	//Mat格式转QImage格式
	QImage Mat2QImage(const cv::Mat& mat);

	//Mat格式转到QSting格式，方便打印输出
	QString Mat2QString(const cv::Mat& mat);

	double GetDistance(const cv::Point2f &a, const cv::Point2f &b);

	//遍历获得距离点point最小的点
	cv::Point2f GetDistanceMinFromCenters(const cv::Point2f &point, const std::vector<cv::Point2f> &centers);

	//计算距离point最小的圆心，返回值为centers中的idx
	int GetDistanceMinFromCentersIdx(const cv::Point2f &point, const std::vector<cv::Point2f> &centers);

	//计算方差
	double ComputeVariance(const std::vector<cv::Point> &contours, const cv::Point2f &center);

	//判断是否为圆
	bool isCircle(const std::vector<cv::Point> &contours, cv::Point2f &center, double maxOffset);

	//从XML文件中读取Mat
	void XML2Mat(const QString &filename);

	//对应点搜索（旧）
	void findCorrespondence(const cv::Mat& imageL, const cv::Mat& imageR, const cv::Mat& PL, const cv::Mat& PR, cv::Mat& PU, cv::Mat& PP);

	//对应点搜索
	void MyFindCorrespondence(const cv::Mat& imageL, const cv::Mat& imageR, cv::Mat& disparity, cv::Mat& valid);

	void reconstruct3D(const cv::Mat& disparity, const cv::Mat &Q, const int &dis, const int &minDisparity, cv::Point2i &lefttop, const cv::Mat& Mask, cv::Mat& xyz);
	cv::Mat readFromXML(const QString &str, QXmlStreamReader &reader);
};





#endif // MyCalibTools_h__
