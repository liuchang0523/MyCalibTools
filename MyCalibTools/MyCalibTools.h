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
	void on_actionOpenList_triggered();
	void on_actionCalibration_triggered();

private:
	Ui::MyCalibToolsClass ui;
	//图片路径
	QString m_imagePath_;
	QStringList m_image_list_;
	QString m_parent_path_;//记录上一次打开图像的路径
	cv::Mat m_image_source_;
	int m_image_num;

	//圆心坐标
	std::vector<std::vector<cv::Point2f>> m_points_on_image_;
	std::vector<std::vector<cv::Point3f>> m_points_on_world_;

private:
	//Mat格式转QImage格式
	QImage Mat2QImage(const cv::Mat& mat);

	double GetDistance(const cv::Point2f &a, const cv::Point2f &b);

	//遍历获得距离点point最小的点
	cv::Point2f GetDistanceMinFromCenters(const cv::Point2f &point, const std::vector<cv::Point2f> &centers);

	//计算距离point最小的圆心，返回值为centers中的idx
	int GetDistanceMinFromCentersIdx(const cv::Point2f &point, const std::vector<cv::Point2f> &centers);

	//计算方差
	double ComputeVariance(const std::vector<cv::Point> &contours, const cv::Point2f &center);

	//判断是否为圆
	bool isCircle(const std::vector<cv::Point> &contours, cv::Point2f &center, double maxOffset);
};





#endif // MyCalibTools_h__
