#include "MyCalibTools.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QtCore\qxmlstream.h>
#include <QtCore\QProcess>
#include <opencv2\core\mat.hpp>

static void saveXYZ(const char* filename, const cv::Mat& mat)
{
	const double max_z = 1.0e4;
	FILE* fp = fopen(filename, "wt");
	for (int y = 0; y < mat.rows; y++)
	{
		for (int x = 0; x < mat.cols; x++)
		{
			cv::Vec3f point = mat.at<cv::Vec3f>(y, x);
			if (fabs(point[2] - max_z) < FLT_EPSILON || fabs(point[2]) > max_z) continue;
			fprintf(fp, "%f %f %f\n", point[0], point[1], point[2]);
		}
	}
	fclose(fp);
}


MyCalibTools::MyCalibTools(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	this->move(1920, 0);
	this->setWindowState(Qt::WindowMaximized);
	m_stereo_finished = false;
	m_parent_path_ = ".";
	//on_actionLoadAndMatching_triggered();
}

void MyCalibTools::on_actionOpen_triggered()
{
	m_imagePath_ = QFileDialog::getOpenFileName(this, QStringLiteral("打开图片文件"), m_parent_path_, "Image(*.bmp *.png *.jpg)");
	if (m_imagePath_ == "")
	{
		QMessageBox::warning(this, QStringLiteral("未选择图片"), QStringLiteral("未选择图片"));
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	//记录上级目录
	QDir dir_temp(m_imagePath_);
	dir_temp.cdUp();
	m_parent_path_ = dir_temp.absolutePath();
	//////////////////////////////////////////////////////////////////////////
	m_image_source_ = cv::imread(std::string(m_imagePath_.toLocal8Bit()));
	cv::Mat m_image_gray_;
	cv::cvtColor(m_image_source_, m_image_gray_, CV_BGR2GRAY);
	GaussianBlur(m_image_gray_, m_image_gray_, cv::Size(3, 3), 0);
	cv::Mat m_image_canny_;
	cv::Canny(m_image_gray_, m_image_canny_, 30, 100);
	cv::imwrite("canny.bmp", m_image_canny_);
	std::vector <std::vector<cv::Point>> m_contours_;//边界
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
		qDebug() << QStringLiteral("去除边界");
	}
	int sum = 0;
	std::vector <std::vector<cv::Point>> contours_temp;//边界
	std::vector<cv::Point2f> centers;//圆心
	for (int i = 0; i < m_contours_.size(); ++i)
	{
		if (m_contours_[i].size() < 1000)
		{
			cv::Point2f center;
			if (isCircle(m_contours_[i], center, 15.0))
			{
				sum += m_contours_[i].size();
				contours_temp.push_back(m_contours_[i]);
				centers.push_back(center);
			}
		}
	}
	m_contours_ = contours_temp;

	double avg = sum / m_contours_.size();
	//遍历寻找大圆
	std::vector<int> idx_big;
	std::vector<cv::Point2f> centers_big;
	for (int i = 0; i < m_contours_.size(); ++i)
	{
		if (m_contours_[i].size() > avg*1.5)
		{

			idx_big.push_back(i);
			centers_big.push_back(centers[i]);
		}
	}
	if (centers_big.size() != 3)
	{
		QMessageBox::warning(this, QStringLiteral("大圆数目有误！"), QStringLiteral("大圆数目有误！大圆数目为：%1,请查看canny图").arg(centers_big.size()));
		ui.widget->setImage(Mat2QImage(m_image_canny_));
		return;
	}

	//通过两两之间连线的距离排序，求出大圆1
	std::vector<cv::Point2f> centers_big_two_max;
	cv::Point2f center_big_1;
	double dis_max = 0;
	for (int i = 0; i < centers_big.size(); i++)
	{
		for (int j = i + 1; j < centers_big.size(); j++)
		{
			double dis_big = GetDistance(centers_big[i], centers_big[j]);
			if (dis_big > dis_max)
			{
				centers_big_two_max.clear();
				centers_big_two_max.push_back(centers_big[i]);
				centers_big_two_max.push_back(centers_big[j]);
				center_big_1 = centers_big[3 - i - j];
				dis_max = dis_big;
			}
		}
	}
	//求另外两个大圆与大圆1的连线
	std::vector<cv::Point3f> line_big_123;
	line_big_123.push_back(cv::Point3f(centers_big_two_max[0].x - center_big_1.x,
		centers_big_two_max[0].y - center_big_1.y, 1));
	line_big_123.push_back(cv::Point3f(centers_big_two_max[1].x - center_big_1.x,
		centers_big_two_max[1].y - center_big_1.y, 1));
	cv::Mat cross_result = cv::Mat(line_big_123[0]).cross(cv::Mat(line_big_123[1]));

	float z = cross_result.at<float>(2);
	cv::Point2f center_big_2;
	cv::Point2f center_big_3;

	if (z > 0)//Z轴表示从第一个点到第二个点的朝向是
	{
		center_big_2 = centers_big_two_max[0];
		center_big_3 = centers_big_two_max[1];
	}
	else
	{
		center_big_3 = centers_big_two_max[0];
		center_big_2 = centers_big_two_max[1];
	}

	cv::Point2f center_23 = (center_big_2 + center_big_3) / 2;
	cv::Point2f center_4_guess = 2 * center_23 - center_big_1;
	cv::Point2f center_4 = GetDistanceMinFromCenters(center_4_guess, centers);

	//qDebug() << z;

#if 0 //画出大圆轮廓
	for (int i = 0; i < idx_big.size(); ++i)
	{
		cv::drawContours(m_image_source_, m_contours_, idx_big[i], cv::Scalar(0, 0, 255), 2);
	}
#endif
#if 0 //画出大圆序号
	cv::Point2f text_center_1 = cv::Point(center_big_1.x - 30, center_big_1.y);
	cv::Point2f text_center_2 = cv::Point(center_big_2.x - 30, center_big_2.y);
	cv::Point2f text_center_3 = cv::Point(center_big_3.x - 30, center_big_3.y);
	cv::putText(m_image_source_, "1", text_center_1, cv::FONT_HERSHEY_SCRIPT_SIMPLEX,
		3, cv::Scalar(0, 0, 255), 5);
	cv::putText(m_image_source_, "2", text_center_2, cv::FONT_HERSHEY_SCRIPT_SIMPLEX,
		3, cv::Scalar(0, 0, 255), 5);
	cv::putText(m_image_source_, "3", text_center_3, cv::FONT_HERSHEY_SCRIPT_SIMPLEX,
		3, cv::Scalar(0, 0, 255), 5);
#endif

	std::vector<cv::Point2f> centers_big_sorted;
	centers_big_sorted.push_back(center_big_1);
	centers_big_sorted.push_back(center_big_2);
	centers_big_sorted.push_back(center_big_3);
	centers_big_sorted.push_back(center_4);
	std::vector<cv::Point2f> centers_big_world;
	int d = 25;
	centers_big_world.push_back(cv::Point2f(-d, d));
	centers_big_world.push_back(cv::Point2f(-d, -d));
	centers_big_world.push_back(cv::Point2f(d, d));
	centers_big_world.push_back(cv::Point2f(d, -d));
	cv::Mat H = cv::findHomography(centers_big_sorted, centers_big_world);
	std::vector<cv::Point2f> centers_after_perspective;
	cv::perspectiveTransform(centers, centers_after_perspective, H);
	//构造圆心的标准坐标矩阵
	std::vector<cv::Point2f> centers_world;
	for (int i = 0; i < 7; ++i)
	{
		for (int j = 0; j < 7; ++j)
		{
			if (i % 2 == 1)
			{
				if (j % 2 == 1)
					centers_world.push_back(cv::Point2f((j - 3)*d, (i - 3)*d));
				else
					continue;
			}
			else
			{
				if (j % 2 == 0)
					centers_world.push_back(cv::Point2f((j - 3)*d, (i - 3)*d));
				else
					continue;
			}
		}
	}
	//排序
	std::vector<cv::Point2f> centers_sorted;
	for (int i = 0; i < centers_world.size(); ++i)
	{
		int idx = GetDistanceMinFromCentersIdx(centers_world[i], centers_after_perspective);
		centers_sorted.push_back(centers[idx]);
	}

#if 0 //打印序号
	for (int i = 0; i < centers_sorted.size(); ++i)
	{
		cv::putText(m_image_source_, std::to_string(i + 1), centers_sorted[i], cv::FONT_HERSHEY_SCRIPT_SIMPLEX,
			3, cv::Scalar(0, 0, 255), 5);
	}
#endif

	for (int i = 0; i < centers_sorted.size(); ++i)
	{
		cv::circle(m_image_source_, centers_sorted[i], 5, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);
	}

	cv::imwrite("center.bmp", m_image_source_);
	ui.widget->setImage(Mat2QImage(m_image_source_));

}

void MyCalibTools::on_actionOpenList_triggered()
{
	m_image_list_ = QFileDialog::getOpenFileNames(this, QStringLiteral("打开图片文件"), m_parent_path_, "Image(*.bmp *.png *.jpg)");
	if (m_image_list_.size() == 0)
	{
		QMessageBox::warning(this, QStringLiteral("未选择图片"), QStringLiteral("未选择图片"));
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	//记录上级目录
	QDir dir_temp(m_image_list_[0]);
	dir_temp.cdUp();
	m_parent_path_ = dir_temp.absolutePath();
	//////////////////////////////////////////////////////////////////////////


	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	if (m_points_on_image_.size() > 0)
	{
		m_points_on_image_.clear();
		m_points_on_world_.clear();
	}
	for (int k = 0; k < m_image_list_.size(); ++k)
	{

		m_image_source_ = cv::imread(std::string(m_image_list_[k].toLocal8Bit()));
		cv::Mat m_image_gray_;
		cv::cvtColor(m_image_source_, m_image_gray_, CV_BGR2GRAY);
		GaussianBlur(m_image_gray_, m_image_gray_, cv::Size(3, 3), 0);
		cv::Mat m_image_canny_;
		cv::Canny(m_image_gray_, m_image_canny_, 30, 100);
		std::vector <std::vector<cv::Point>> m_contours_;//边界
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
		std::vector <std::vector<cv::Point>> contours_temp;//边界
		std::vector<cv::Point2f> centers;//圆心
		for (int i = 0; i < m_contours_.size(); ++i)
		{
			if (m_contours_[i].size() < 1000)
			{
				cv::Point2f center;
				if (isCircle(m_contours_[i], center, 15.0))//圆度阈值
				{
					sum += m_contours_[i].size();
					contours_temp.push_back(m_contours_[i]);
					centers.push_back(center);
				}
			}
		}
		m_contours_ = contours_temp;

		double avg = sum / m_contours_.size();
		//遍历寻找大圆
		std::vector<int> idx_big;
		std::vector<cv::Point2f> centers_big;
		for (int i = 0; i < m_contours_.size(); ++i)
		{
			if (m_contours_[i].size() > avg*1.5)
			{

				idx_big.push_back(i);
				centers_big.push_back(centers[i]);
			}
		}
		if (centers_big.size() != 3)
		{
			QMessageBox::warning(this, QStringLiteral("大圆数目有误！"), QStringLiteral("大圆数目有误！%1").arg(m_image_list_[k]));
			continue;
		}

		//通过两两之间连线的距离排序，求出大圆1
		std::vector<cv::Point2f> centers_big_two_max;
		cv::Point2f center_big_1;
		double dis_max = 0;
		for (int i = 0; i < centers_big.size(); i++)
		{
			for (int j = i + 1; j < centers_big.size(); j++)
			{
				double dis_big = GetDistance(centers_big[i], centers_big[j]);
				if (dis_big > dis_max)
				{
					centers_big_two_max.clear();
					centers_big_two_max.push_back(centers_big[i]);
					centers_big_two_max.push_back(centers_big[j]);
					center_big_1 = centers_big[3 - i - j];
					dis_max = dis_big;
				}
			}
		}
		//求另外两个大圆与大圆1的连线
		std::vector<cv::Point3f> line_big_123;
		line_big_123.push_back(cv::Point3f(centers_big_two_max[0].x - center_big_1.x,
			centers_big_two_max[0].y - center_big_1.y, 1));
		line_big_123.push_back(cv::Point3f(centers_big_two_max[1].x - center_big_1.x,
			centers_big_two_max[1].y - center_big_1.y, 1));
		cv::Mat cross_result = cv::Mat(line_big_123[0]).cross(cv::Mat(line_big_123[1]));

		float z = cross_result.at<float>(2);
		cv::Point2f center_big_2;
		cv::Point2f center_big_3;

		if (z > 0)//Z轴表示从第一个点到第二个点的朝向是
		{
			center_big_2 = centers_big_two_max[0];
			center_big_3 = centers_big_two_max[1];
		}
		else
		{
			center_big_3 = centers_big_two_max[0];
			center_big_2 = centers_big_two_max[1];
		}

		cv::Point2f center_23 = (center_big_2 + center_big_3) / 2;
		cv::Point2f center_4_guess = 2 * center_23 - center_big_1;
		cv::Point2f center_4 = GetDistanceMinFromCenters(center_4_guess, centers);
		for (int i = 0; i < idx_big.size(); ++i)
		{
			cv::drawContours(m_image_source_, m_contours_, idx_big[i], cv::Scalar(0, 0, 255), 2);
		}
		std::vector<cv::Point2f> centers_big_sorted;
		centers_big_sorted.push_back(center_big_1);
		centers_big_sorted.push_back(center_big_2);
		centers_big_sorted.push_back(center_big_3);
		centers_big_sorted.push_back(center_4);
		std::vector<cv::Point2f> centers_big_world;
		int d = 25;
		centers_big_world.push_back(cv::Point2f(-d, d));
		centers_big_world.push_back(cv::Point2f(-d, -d));
		centers_big_world.push_back(cv::Point2f(d, d));
		centers_big_world.push_back(cv::Point2f(d, -d));
		cv::Mat H = cv::findHomography(centers_big_sorted, centers_big_world);
		std::vector<cv::Point2f> centers_after_perspective;
		cv::perspectiveTransform(centers, centers_after_perspective, H);
		//构造圆心的标准坐标矩阵
		std::vector<cv::Point2f> centers_world;
		for (int i = 0; i < 7; ++i)
		{
			for (int j = 0; j < 7; ++j)
			{
				if (i % 2 == 1)
				{
					if (j % 2 == 1)
						centers_world.push_back(cv::Point2f((j - 3)*d, (i - 3)*d));
					else
						continue;
				}
				else
				{
					if (j % 2 == 0)
						centers_world.push_back(cv::Point2f((j - 3)*d, (i - 3)*d));
					else
						continue;
				}
			}
		}
		//排序
		std::vector<cv::Point2f> centers_sorted;
		for (int i = 0; i < centers_world.size(); ++i)
		{
			int idx = GetDistanceMinFromCentersIdx(centers_world[i], centers_after_perspective);
			centers_sorted.push_back(centers[idx]);
		}

		for (int i = 0; i < centers_sorted.size(); ++i)
		{
			cv::putText(m_image_source_, std::to_string(i + 1), centers_sorted[i], cv::FONT_HERSHEY_SCRIPT_SIMPLEX,
				3, cv::Scalar(0, 0, 255), 5);
		}
		std::vector<cv::Point3f> centers_world_3d;
		for (int i = 0; i < centers_world.size(); ++i)
		{
			centers_world_3d.push_back(cv::Point3f(centers_world[i].x, centers_world[i].y, 0));
		}

		ui.widget->setImage(Mat2QImage(m_image_source_));
		QApplication::processEvents();
		m_points_on_image_.push_back(centers_sorted);
		m_points_on_world_.push_back(centers_world_3d);
	}
	QApplication::restoreOverrideCursor();
}

void MyCalibTools::on_actionCalibration_triggered()
{
	// 	if (m_points_on_image_.size() < 5)
	// 	{
	// 		return;
	// 	}
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	m_matrix_ = cv::Mat(3, 3, CV_32FC1, cv::Scalar::all(0));
	m_distortion_ = cv::Mat(1, 5, CV_32FC1, cv::Scalar::all(0));
	double e = cv::calibrateCamera(m_points_on_world_, m_points_on_image_,
		m_image_source_.size(), m_matrix_, m_distortion_, m_R_, m_T_,
		cv::noArray(), cv::noArray(), m_errors_, 0);
	qDebug() << QStringLiteral("标定完成！");
	qDebug() << QStringLiteral("重投影误差为%1").arg(e);
	std::cout << "相机内参矩阵：" << std::endl;
	std::cout << m_matrix_ << std::endl;
	std::cout << "畸变系数：" << std::endl;
	std::cout << m_distortion_ << std::endl;
	for (int i = 0; i < m_errors_.size(); ++i)
	{
		std::cout << "第" << i + 1 << "副图像的重投影误差为:" << m_errors_[i] << std::endl;
	}
	QApplication::restoreOverrideCursor();
}


void MyCalibTools::on_actionStereoCalibrate_triggered()
{
#if 1 // 打开图片进行标定
	std::vector<std::vector<cv::Point2f>> points_on_image_1;
	std::vector<std::vector<cv::Point2f>> points_on_image_2;
	std::vector<std::vector<cv::Point3f>> points_on_world;
	on_actionOpenList_triggered();
	points_on_image_1 = m_points_on_image_;
	//左相机单目标定
	on_actionCalibration_triggered();
	matrix_1 = m_matrix_;
	dist_1 = m_distortion_;

	on_actionOpenList_triggered();
	points_on_image_2 = m_points_on_image_;
	points_on_world = m_points_on_world_;
	on_actionCalibration_triggered();
	if (points_on_image_1.size() != points_on_image_2.size())
	{
		QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("图片数目不相等！"));
		return;
	}

	matrix_2 = m_matrix_;
	dist_2 = m_distortion_;

	std::vector<double> errors;
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	double e = cv::stereoCalibrate(points_on_world, points_on_image_1, points_on_image_2,
		matrix_1, dist_1, matrix_2, dist_2, m_image_source_.size(),
		R, T, E, F, CV_CALIB_FIX_INTRINSIC);//使用单目标定的内参和畸变
	std::cout << std::endl << std::endl;
	qDebug() << QStringLiteral("双目标定的重投影误差为%1").arg(e);
	std::cout << "R:" << std::endl << R << std::endl;
	std::cout << "T:" << std::endl << T << std::endl;

	//////////////////////////////////////////////////////////////////////////
	//写入xml
	QFile file("StereoCalibrateResult.xml");
	if (!file.open(QFile::WriteOnly | QFile::Text))
	{
		qDebug() << "Error: cannot open file";
		return;
	}
	QXmlStreamWriter stream(&file);
	stream.setAutoFormatting(true);
	stream.writeStartDocument();
	stream.writeStartElement("calibration_result");
	stream.writeTextElement("matrix_1", Mat2QString(matrix_1));
	stream.writeTextElement("distortion_1", Mat2QString(dist_1));
	stream.writeTextElement("matrix_2", Mat2QString(matrix_2));
	stream.writeTextElement("distortion_2", Mat2QString(dist_2));
	stream.writeTextElement("R", Mat2QString(R));
	stream.writeTextElement("T", Mat2QString(T));
	stream.writeTextElement("e", QString::number(e, 'g', 10));
	stream.writeEndElement();
	stream.writeEndElement();
	file.close();

	QApplication::restoreOverrideCursor();
	//标定成功
	m_stereo_finished = true;
#else
	m_image_source_ = cv::imread("L1.bmp");
	XML2Mat("StereoCalibrateResult.xml");
#endif



	//////////////////////////////////////////////////////////////////////////
	//标定结果验证
// 	cv::Mat lines_1, lines_2;
// 	cv::undistortPoints(points_on_image_1[0], points_on_image_1[0], matrix_1, dist_1, cv::noArray(), matrix_1);
// 	cv::undistortPoints(points_on_image_2[0], points_on_image_2[0], matrix_2, dist_2, cv::noArray(), matrix_2);
// 	cv::computeCorrespondEpilines(points_on_image_1[0], 1, F, lines_1);
// 	cv::computeCorrespondEpilines(points_on_image_2[0], 2, F, lines_2);
// 	double avg_err = 0;
	// 	for (int i = 0; i < points_on_image_1[0].size(); ++i)
	// 	{
	// 		double err = fabs(points_on_image_1[0][i].x)
	// 	}
	cv::Mat R_1(3, 3, CV_64F), R_2(3, 3, CV_64F);
	cv::Mat P_1(3, 4, CV_64F), P_2(3, 4, CV_64F);
	cv::Mat Q(4, 4, CV_64F);
	cv::Size new_size(m_image_source_.size().width * 2, m_image_source_.size().height * 2);
	cv::Rect roi1;
	cv::Rect roi2;
	//极限矫正
	cv::stereoRectify(matrix_1, dist_1, matrix_2, dist_2, m_image_source_.size(),
		R, T, R_1, R_2, P_1, P_2, Q, cv::CALIB_ZERO_DISPARITY, 1, new_size, &roi1, &roi2);

	//计算矫正后的Map
	cv::Mat mx1(m_image_source_.size(), CV_32F);
	cv::Mat my1(m_image_source_.size(), CV_32F);
	cv::Mat mx2(m_image_source_.size(), CV_32F);
	cv::Mat my2(m_image_source_.size(), CV_32F);

	cv::initUndistortRectifyMap(matrix_1, dist_1, R_1, P_1, new_size, CV_32FC1, mx1, my1);
	cv::initUndistortRectifyMap(matrix_2, dist_2, R_2, P_2, new_size, CV_32FC1, mx2, my2);

	//读取图片
	cv::Mat imageL = cv::imread("L2.bmp");
	cv::Mat imageR = cv::imread("R2.bmp");
	cv::Mat imageLr, imageRr;
	cv::remap(imageL, imageLr, mx1, my1, cv::INTER_CUBIC);
	cv::remap(imageR, imageRr, mx2, my2, cv::INTER_CUBIC);
	//cv::rectangle(imageLr, roi1, cv::Scalar(0, 0, 255), 3);
	//cv::rectangle(imageRr, roi2, cv::Scalar(0, 0, 255), 3);

	//提取公共有效区域
	int y_max = cv::max(roi1.y, roi2.y);
	int width_min = cv::min(roi1.width, roi2.width);
	int height_min = cv::min(roi1.y + roi1.height - y_max,
		roi2.y + roi2.height - y_max);
	cv::Rect roi1r(roi1.x, y_max, width_min, height_min);
	cv::Rect roi2r(roi2.x, y_max, width_min, height_min);

	cv::rectangle(imageLr, roi1r, cv::Scalar(0, 0, 255), 5);
	cv::rectangle(imageRr, roi2r, cv::Scalar(0, 0, 255), 5);

	cv::imwrite("imageLr.bmp", imageLr);
	cv::imwrite("imageRr.bmp", imageRr);

	cv::Mat imageLr_cut, imageRr_cut;
	//裁剪公共有效区域
	imageLr_cut = imageLr(roi1r);
	imageRr_cut = imageRr(roi2r);

	//cv::imwrite("L1r.bmp", imageLr_cut);
	//cv::imwrite("R1r.bmp", imageRr_cut);

	QMessageBox::about(this, QStringLiteral("极线矫正完毕！"), QStringLiteral("极线矫正完毕！"));
	//////////////////////////////////////////////////////////////////////////
}


void MyCalibTools::on_actionMatching_triggered()
{
	//读取极线矫正后图像
	cv::Mat img1 = cv::imread("L1r.bmp", 0);
	cv::Mat img2 = cv::imread("R1r.bmp", 0);

	cv::Mat disparity;
#if 0 //BM算法
	cv::Ptr<cv::StereoBM> bm = cv::StereoBM::create(256, 11);
	bm->setPreFilterType(CV_STEREO_BM_NORMALIZED_RESPONSE);
	bm->setPreFilterSize(9);
	bm->setPreFilterCap(31);
	bm->setUniquenessRatio(1);
	bm->setMinDisparity(-256);
	bm->compute(img1, img2, disparity);
#endif
	cv::Mat valid;
	MyFindCorrespondence(img1, img2, disparity, valid);

	// 	cv::Ptr<cv::StereoSGBM> sgbm = cv::StereoSGBM::create(0, 128, 11, 8 * 11 * 11, 32 * 11 * 11);
	// 	sgbm->setDisp12MaxDiff(1);
	// 	sgbm->setUniquenessRatio(10);
	// 	sgbm->setSpeckleWindowSize(100);
	// 	sgbm->setSpeckleRange(32);
	// 	sgbm->setMode(cv::StereoSGBM::MODE_SGBM);
	// 	sgbm->compute(img1, img2, disparity);
	valid.convertTo(valid, CV_8U, 255);
	disparity.convertTo(disparity, CV_32F, 1.0 / 16);//转换为可视化的视差图
	cv::normalize(disparity, disparity, 0, 256, cv::NORM_MINMAX);
}

void MyCalibTools::on_actionLoadAndMatching_triggered()
{

	m_image_list_ = QFileDialog::getOpenFileNames(this, QStringLiteral("打开两张图片文件，先L后R"), m_parent_path_, "Image(*.bmp *.png *.jpg)");
	if (m_image_list_.size() != 2)
	{
		QMessageBox::warning(this, QStringLiteral("打开图片数目有误"), QStringLiteral("打开图片数目有误"));
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	//记录上级目录
	QDir dir_temp(m_image_list_[0]);
	dir_temp.cdUp();
	m_parent_path_ = dir_temp.absolutePath();
	//////////////////////////////////////////////////////////////////////////

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	//极线矫正
	m_image_source_ = cv::imread("L1.bmp");
	XML2Mat("StereoCalibrateResult.xml");
	cv::Mat R_1(3, 3, CV_64F), R_2(3, 3, CV_64F);
	cv::Mat P_1(3, 4, CV_64F), P_2(3, 4, CV_64F);
	cv::Mat Q(4, 4, CV_64F);
	cv::Size new_size(m_image_source_.size().width * 2, m_image_source_.size().height * 2);
	//cv::Size new_size = m_image_source_.size();
	cv::Rect roi1;
	cv::Rect roi2;
	//极限矫正
	cv::stereoRectify(matrix_1, dist_1, matrix_2, dist_2, m_image_source_.size(),
		R, T, R_1, R_2, P_1, P_2, Q, cv::CALIB_ZERO_DISPARITY, -1, new_size, &roi1, &roi2);

	//计算矫正后的Map
	cv::Mat mx1(m_image_source_.size(), CV_32F);
	cv::Mat my1(m_image_source_.size(), CV_32F);
	cv::Mat mx2(m_image_source_.size(), CV_32F);
	cv::Mat my2(m_image_source_.size(), CV_32F);
	cv::initUndistortRectifyMap(matrix_1, dist_1, R_1, P_1, new_size, CV_32FC1, mx1, my1);
	cv::initUndistortRectifyMap(matrix_2, dist_2, R_2, P_2, new_size, CV_32FC1, mx2, my2);

	//读取图片
	cv::Mat imageL = cv::imread(std::string(m_image_list_[0].toLocal8Bit()));
	cv::Mat imageR = cv::imread(std::string(m_image_list_[1].toLocal8Bit()));
	cv::Mat imageLr, imageRr;
	cv::remap(imageL, imageLr, mx1, my1, cv::INTER_CUBIC);
	cv::remap(imageR, imageRr, mx2, my2, cv::INTER_CUBIC);
	//cv::rectangle(imageLr, roi1, cv::Scalar(0, 0, 255), 3);
	//cv::rectangle(imageRr, roi2, cv::Scalar(0, 0, 255), 3);

	//提取公共有效区域
	int y_max = cv::max(roi1.y, roi2.y);
	int width_min = cv::min(roi1.width, roi2.width);
	int height_min = cv::min(roi1.y + roi1.height - y_max,
		roi2.y + roi2.height - y_max);
	cv::Rect roi1r(roi1.x, y_max, width_min, height_min);
	cv::Rect roi2r(roi2.x, y_max, width_min, height_min);

	int dis = roi1.x - roi2.x;

	//cv::rectangle(imageLr, roi1r, cv::Scalar(0, 255, 0), 3);
	//cv::rectangle(imageRr, roi2r, cv::Scalar(0, 255, 0), 3);

	cv::Mat imageLr_cut, imageRr_cut;
	//裁剪公共有效区域
	imageLr_cut = imageLr(roi1r);
	imageRr_cut = imageRr(roi2r);

	cv::imwrite("imageLr.bmp", imageLr_cut);
	cv::imwrite("imageRr.bmp", imageRr_cut);

	//////////////////////////////////////////////////////////////////////////

	//读取极线矫正后图像
	cv::Mat img1;
	cv::Mat img2;
	cv::cvtColor(imageLr_cut, img1, CV_BGR2GRAY);
	cv::cvtColor(imageRr_cut, img2, CV_BGR2GRAY);


	cv::Mat disparity;
	cv::Mat disparity_2;
#if 0 //BM算法
	int minDisparity = -128;
	//第一遍，从右边图像中找左边图像的对应点
	cv::Ptr<cv::StereoBM> bm = cv::StereoBM::create(14 * 16, 21);
	bm->setPreFilterType(CV_STEREO_BM_BASIC);
	bm->setMinDisparity(minDisparity);
	bm->compute(img1, img2, disparity);
	//第二遍，从左边图像中找右边图像的对应点
	bm->setMinDisparity(-(14 * 16 + minDisparity));
	bm->compute(img2, img1, disparity_2);
	disparity.convertTo(disparity, CV_32F);
	disparity_2.convertTo(disparity_2, CV_32F);
	//遍历
	cv::Mat valid(disparity.size(), CV_8U, cv::Scalar(0));
	for (int i = 0; i < disparity.rows; ++i)
	{
		float* ptr = disparity.ptr<float>(i);
		float* ptr_2 = disparity_2.ptr<float>(i);
		uchar* v_ptr = valid.ptr<uchar>(i);
		for (int j = 0; j < disparity.cols; ++j)
		{
			if (ptr[j] <= -128 * 16 || ptr[j] >= 1536 || _isnanf(ptr[j]))
			{
				continue;
			}
			float left_right = ptr[j] / 16.0;
			float right_left = ptr_2[j - int(left_right)];
			if (abs(left_right - right_left < 1.0))
			{
				v_ptr[j] = 255;
			}
		}
	}
#endif
	cv::Mat valid;
	MyFindCorrespondence(img1, img2, disparity, valid);

	disparity.convertTo(disparity, CV_32F);
	cv::Mat xyz;
	reconstruct3D(disparity, Q, dis * 16, -128, roi1r.tl(), valid, xyz);
	saveXYZ("result.xyz", xyz);

	//disparity.convertTo(disparity, CV_32F, 1.0 / 16);//转换为可视化的视差图	
	//cv::Mat disparity_all(new_size, disparity.type());

// 	cv::Mat PL(img1.size(), CV_8UC1, cv::Scalar(1));
// 	cv::Mat PR(img2.size(), CV_8UC1, cv::Scalar(1));
// 	cv::Mat PP(img2.size(), CV_8UC1, cv::Scalar(0));
// 	cv::Mat PU;
// 	findCorrespondence(img1, img2, PL, PR, PU, PP);

	//cv::Mat xyz;

	//cv::reprojectImageTo3D(disparity, xyz, Q, false);
	//disparity.convertTo(disparity, CV_32F);//转换为可视化的视差图
	//reconstruct3D(disparity, xyz, Q);
	//saveXYZ("result.xyz", xyz);

	disparity.convertTo(disparity, CV_32F);//转换为可视化的视差图
	//cv::Mat disparity_cut(disparity(roi1r));
	cv::normalize(disparity, disparity, 0, 1, cv::NORM_MINMAX);
	cv::imwrite("disparity.bmp", disparity);




	disparity.convertTo(disparity, CV_8UC1, 255);
	ui.widget->setImage(Mat2QImage(disparity));
	QApplication::restoreOverrideCursor();
}

void MyCalibTools::on_actionXML_triggered()
{
	QProcess::execute("explorer StereoCalibrateResult.xml");
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


QString MyCalibTools::Mat2QString(const cv::Mat& mat)
{
	QString str = "\n";
	for (int i = 0; i < mat.rows; ++i)
	{
		for (int j = 0; j < mat.cols; ++j)
		{
			str += QString::number(mat.at<double>(i, j), 'f', 10);//10位有效数字
			if (j != mat.cols - 1)
			{
				str += " ";
			}
		}
		str += "\n";
	}
	return str;
}

double MyCalibTools::GetDistance(const cv::Point2f &a, const cv::Point2f &b)
{
	//计算距离
	double distance;
	distance = powf((a.x - b.x), 2) + powf((a.y - b.y), 2);
	distance = sqrtf(distance);
	return distance;
}

cv::Point2f MyCalibTools::GetDistanceMinFromCenters(const cv::Point2f &point, const std::vector<cv::Point2f> &centers)
{
	double dis_temp = 9999;
	cv::Point2f result;
	for (int i = 0; i < centers.size(); i++)
	{
		double dis = GetDistance(point, centers[i]);
		if (dis < dis_temp)
		{
			result = centers[i];
			dis_temp = dis;
		}
	}
	return result;
}

int MyCalibTools::GetDistanceMinFromCentersIdx(const cv::Point2f &point, const std::vector<cv::Point2f> &centers)
{
	double dis_temp = 9999;
	int num = 0;
	for (int i = 0; i < centers.size(); i++)
	{
		double dis = GetDistance(point, centers[i]);
		if (dis < dis_temp)
		{
			num = i;
			dis_temp = dis;
		}
	}
	return num;
}

double MyCalibTools::ComputeVariance(const std::vector<cv::Point> &contours, const cv::Point2f &center)
{
	int n = contours.size();
	std::vector<double> a(n);

	double aver, s;
	double sum = 0, e = 0;

	for (int i = 0; i < n; i++)
	{
		a[i] = GetDistance(contours[i], center);
		sum += a[i];
	}
	aver = sum / n;
	for (int i = 0; i < n; i++)
		e += (a[i] - aver) * (a[i] - aver);
	e /= n;
	s = sqrt(e);
	return e;
}

bool MyCalibTools::isCircle(const std::vector<cv::Point> &contours, cv::Point2f &center, double maxOffset)
{
	//判断是否为圆
	cv::RotatedRect _box;
	if (contours.size() > 5)
	{
		//////////////////////////////////////////////////////////////////////////
		float radius;
		minEnclosingCircle(contours, center, radius);//得到最小外接圆圆心和半径
		_box = fitEllipse(contours);
		center = _box.center;
		//////////////////////////////////////////////////////////////////////////
		double offset = ComputeVariance(contours, center);
		if (offset < maxOffset)
		{
			return true;
		}
		else
		{
			//qDebug() << QStringLiteral("大于阈值offset：%1").arg(offset);
		}
	}
	return false;
}

cv::Mat MyCalibTools::readFromXML(const QString &str, QXmlStreamReader &reader)
{
	std::vector<double> vec;
	cv::Mat result;
	// 如果没有读到文档结尾，而且没有出现错误
	while (!reader.atEnd()) {
		// 读取下一个记号，它返回记号的类型
		QXmlStreamReader::TokenType type = reader.readNext();
		if (type == QXmlStreamReader::StartElement) {
			//qDebug() << "<" << reader.name() << ">";
			if (reader.name() == str)
			{
				type = reader.readNext();
				int row;
				if (type == QXmlStreamReader::Characters
					&& !reader.isWhitespace())
				{
					//qDebug() << reader.text();
					QString text(reader.text().toUtf8());
					row = text.count("\n") - 1;
					int start = 1;
					int index = text.indexOf("\n", start);
					while (index > 0)
					{
						QString temp_row(text.mid(start, index - start));
						//	qDebug() << temp_row;
						int col = temp_row.count(" ") + 1;
						QStringList list = temp_row.split(" ");
						for (int i = 0; i < list.size(); ++i)
						{
							vec.push_back(list.at(i).toDouble());
						}
						start = index + 1;
						index = text.indexOf("\n", start);
					}
				}
				result = cv::Mat(vec);
				result = result.reshape(0, row);
				result = result.clone();//深拷贝
				return result;
			}
		}
	}
	// 如果读取过程中出现错误，那么输出错误信息
	if (reader.hasError()) {
		qDebug() << "error: " << reader.errorString();
	}

	return result;
}

void MyCalibTools::XML2Mat(const QString &filename)
{
	QFile file(filename);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		qDebug() << "Error: cannot open file";
		return;
	}
	QXmlStreamReader reader;
	// 设置文件，这时会将流设置为初始状态
	reader.setDevice(&file);
	matrix_1 = readFromXML("matrix_1", reader);
	dist_1 = readFromXML("distortion_1", reader);
	matrix_2 = readFromXML("matrix_2", reader);
	dist_2 = readFromXML("distortion_2", reader);
	R = readFromXML("R", reader);
	T = readFromXML("T", reader);
	file.close();
}

void MyCalibTools::findCorrespondence(const cv::Mat& imageL, const cv::Mat& imageR, const cv::Mat& PL, const cv::Mat& PR, cv::Mat& PU, cv::Mat& PP)
{
	int row = imageL.rows;
	int col = imageL.cols;

	CV_Assert(row > 0 && col > 0);
	CV_Assert(imageL.rows == imageR.rows && imageL.cols == imageR.cols);
	CV_Assert(PL.rows == imageL.rows && PL.cols == imageL.cols);
	CV_Assert(PL.rows == PR.rows && PL.cols == PR.cols);

	if (PU.cols != col || PU.rows != row)
	{
		PU = cv::Mat::zeros(row, col, CV_32S);
	}

	if (PP.cols != col || PP.rows != row)
	{
		PP = cv::Mat::zeros(row, col, CV_8U);
	}

	int mCHW = 6;
	int mCHW2 = 13;
	int mStep = 1;
	int mMinDisparity = -128;
	int mMaxDisparity = 128;
	float mNCCThresh = 0.7;


	int startRow = mCHW;
	int endRow = row - mCHW;
	int startCol = mCHW;
	int endCol = col - mCHW;

	double* averageDiffL = new double[mCHW2*mCHW2];

	for (int i = startRow; i < endRow; i += mStep)
	{
		for (int j = startCol; j < endCol; j += mStep)
		{
			//left image
			if (PL.type() == CV_8U)
			{
				if (PL.at<uint8_t>(i, j) == 0)
				{
					continue;
				}
			}
			else if (PL.type() == CV_32S)
			{
				if (PL.at<int32_t>(i, j) == 0)
				{
					continue;
				}
			}

			double averageL = 0.0;

			double averageDiffSquareL = 0.0;

			for (int k = i - mCHW; k <= i + mCHW; k++)
			{
				for (int n = j - mCHW; n <= j + mCHW; n++)
				{
					if (imageL.type() == CV_8U)
					{
						averageL += imageL.at<uint8_t>(k, n);
					}
					else if (imageL.type() == CV_32F)
					{
						averageL += imageL.at<float>(k, n);
					}
				}
			}

			averageL /= mCHW2 * mCHW2;

			for (int k = i - mCHW; k <= i + mCHW; k++)
			{
				for (int n = j - mCHW; n <= j + mCHW; n++)
				{
					int m = k - (i - mCHW);
					int l = n - (j - mCHW);
					int index = m * mCHW2 + l;

					double diff = 0.0;
					if (imageL.type() == CV_8U)
					{
						diff = imageL.at<uint8_t>(k, n) - averageL;
					}
					else if (imageL.type() == CV_32F)
					{
						diff = imageL.at<float>(k, n) - averageL;
					}

					averageDiffL[index] = diff;

					averageDiffSquareL += diff * diff;
				}
			}

			//right image
			int32_t minR = j - mMaxDisparity;
			int32_t maxR = j - mMinDisparity;

			minR = minR < mCHW ? mCHW : minR;
			maxR = maxR > col - mCHW - 1 ? col - mCHW - 1 : maxR;

			double ZNCC = 0.0;
			int currentUR = 0;

			for (int loop = minR; loop < maxR; loop++)
			{
				int uR = loop;
				if (PR.type() == CV_8U)
				{
					if (PR.at<uint8_t>(i, uR) == 0)
					{
						continue;
					}
				}
				else if (PR.type() == CV_32S)
				{
					if (PR.at<int32_t>(i, uR) == 0)
					{
						continue;
					}
				}

				if (imageL.type() == CV_8U)//灰度约束
				{
					if (abs(imageL.at<uint8_t>(i, j) - imageR.at<uint8_t>(i, uR)) > 25)
						continue;
				}
				else if (imageL.type() == CV_32F)
				{
					if (abs(imageL.at<float>(i, j) - imageR.at<float>(i, uR)) > 25)
						continue;
				}

				double averageR = 0.0;

				double averageDiffSquareR = 0.0;

				for (int k = i - mCHW; k <= i + mCHW; k++)
				{
					for (int n = uR - mCHW; n <= uR + mCHW; n++)
					{
						if (imageR.type() == CV_8U)
						{
							averageR += imageR.at<uint8_t>(k, n);
						}
						else if (imageR.type() == CV_32F)
						{
							averageR += imageR.at<float>(k, n);
						}
					}
				}

				averageR /= mCHW2 * mCHW2;

				double averDiffSum = 0.0;

				for (int k = i - mCHW; k <= i + mCHW; k++)
				{
					for (int n = uR - mCHW; n <= uR + mCHW; n++)
					{
						int m = k - (i - mCHW);
						int l = n - (uR - mCHW);
						int index = m * mCHW2 + l;

						double diff = 0.0;
						if (imageR.type() == CV_8U)
						{
							diff = imageR.at<uint8_t>(k, n) - averageR;
						}
						else if (imageR.type() == CV_32F)
						{
							diff = imageR.at<float>(k, n) - averageR;
						}

						averDiffSum += averageDiffL[index] * diff;

						averageDiffSquareR += diff * diff;
					}
				}

				if (averageDiffSquareL < 1e-6 || averageDiffSquareR < 1e-6)
					continue;

				double zncc = averDiffSum / sqrt(averageDiffSquareL*averageDiffSquareR);

				if (zncc > ZNCC)
				{
					ZNCC = zncc;
					currentUR = uR;
				}
			}

			if (ZNCC > mNCCThresh)
			{
				if (PU.type() == CV_32S)
				{
					PU.at<int32_t>(i, j) = j - currentUR;
				}
				else if (PU.type() == CV_32F)
				{
					PU.at<float>(i, j) = j - currentUR;
				}

				if (PP.type() == CV_8U)
				{
					PP.at<uint8_t>(i, j) = 1;
				}
				else if (PP.type() == CV_32S)
				{
					PP.at<int32_t>(i, j) = 1;
				}
			}


		}
		//std::cout << i << std::endl;
	}

	delete[] averageDiffL;
}


void MyCalibTools::MyFindCorrespondence(const cv::Mat& imageL, const cv::Mat& imageR, cv::Mat& disparity, cv::Mat& valid)
{
	int row = imageL.rows;
	int col = imageL.cols;

	CV_Assert(row > 0 && col > 0);
	CV_Assert(imageL.rows == imageR.rows && imageL.cols == imageR.cols);

	//初始化视差图和模版
	disparity = cv::Mat(row, col, CV_32S, cv::Scalar(-129));
	valid = cv::Mat::zeros(row, col, CV_8U);

	//设置窗口大小和最大最小视差
	int window = 21;
	CV_Assert(window % 2 == 1 && window > 5);
	//设置搜索步长
	int step = 4;
	int min_disparity = -128;
	int max_disparity = 128;
	double ZNCC_threshold = 0.7;
	double* averageDiffL = new double[window*window];
	int half_window = (window - 1) / 2;
	//循环遍历
	for (int i = half_window; i < (row - half_window); i += step)
	{
		for (int j = half_window; j < (col - half_window); j += step)
		{
			//左图像，**此处可加模版**
			double avgL = 0.0;
			double avg_diff_squareL = 0.0;
			for (int m = i - half_window; m <= i + half_window; ++m)
			{
				const uchar* ptr = imageL.ptr<uchar>(m);
				for (int n = j - half_window; n <= j + half_window; ++n)
				{
					avgL += ptr[n];
				}
			}
			//窗口像素均值
			avgL = avgL / (window*window);

			for (int m = i - half_window; m <= i + half_window; ++m)
			{
				const uchar* ptr = imageL.ptr<uchar>(m);
				for (int n = j - half_window; n <= j + half_window; ++n)
				{
					int index = (m - (i - half_window))*window + n - (j - half_window);
					double diff = ptr[n] - avgL;
					averageDiffL[index] = diff;
					avg_diff_squareL += diff * diff;
				}
			}

			//右图像
			int minR = j + min_disparity;
			int maxR = j + max_disparity;

			minR = minR < half_window ? half_window : minR;
			maxR = maxR > col - half_window - 1 ? col - half_window - 1 : maxR;

			double ZNCC = 0.0;
			int correspond = 0;

			for (int r = minR; r < maxR; ++r)
			{
				//灰度约束
				if (abs(imageL.at<uchar>(i, j) - imageR.at<uchar>(i, r)) > 25)
				{
					continue;
				}

				double avgR = 0.0;
				double avg_diff_squareR = 0.0;

				for (int m = i - half_window; m <= i + half_window; ++m)
				{
					const uchar* ptr = imageL.ptr<uchar>(m);
					for (int n = r - half_window; n <= r + half_window; ++n)
					{
						avgR += ptr[n];
					}
				}

				avgR = avgR / (window*window);

				double averDiffSum = 0.0;

				for (int m = i - half_window; m <= i + half_window; ++m)
				{
					const uchar* ptr = imageL.ptr<uchar>(m);
					for (int n = r - half_window; n <= r + half_window; ++n)
					{
						int index = (m - (i - half_window))*window + n - (r - half_window);
						double diff = ptr[n] - avgR;
						averDiffSum += averageDiffL[index] * diff;
						avg_diff_squareR += diff * diff;
					}
				}

				if (avg_diff_squareL < 1e-6 || avg_diff_squareR < 1e-6)
				{
					continue;
				}

				double zncc_temp = averDiffSum / sqrt(avg_diff_squareL*avg_diff_squareR);

				if (zncc_temp > ZNCC)
				{
					ZNCC = zncc_temp;
					correspond = r;
				}
			}

			if (ZNCC > ZNCC_threshold)
			{
				disparity.at<int32_t>(i, j) = j - correspond;
				valid.at<uchar>(i, j) = 1;
			}
		}
		std::cout << i << std::endl;
	}
	delete[] averageDiffL;
}

void MyCalibTools::reconstruct3D(const cv::Mat& disparty, const cv::Mat &Q,
	const int &dis, const int &minDisparity, cv::Point2i &lefttop, const cv::Mat& Mask, cv::Mat& xyz)
{
	xyz.create(disparty.size(), CV_32FC3);
	if (disparty.type() == CV_32F)
	{
		for (int i = 0; i < disparty.rows; i += 1)
		{
			for (int j = 0; j < disparty.cols; j += 1)
			{

				if (disparty.at<float>(i, j) <= minDisparity || !Mask.at<uchar>(i, j))
				{
					xyz.at<cv::Vec3f>(i, j)[0] = 1.0e4;
					xyz.at<cv::Vec3f>(i, j)[1] = 1.0e4;
					xyz.at<cv::Vec3f>(i, j)[2] = 1.0e4;
					continue;
				}
				double w = Q.at<double>(3, 2)*(disparty.at<float>(i, j) * 16 + dis) + Q.at<double>(3, 3);
				double x = j + lefttop.x + Q.at<double>(0, 3);
				double y = i + lefttop.y + Q.at<double>(1, 3);
				double z = Q.at<double>(2, 3);

				xyz.at<cv::Vec3f>(i, j)[0] = x / w;
				xyz.at<cv::Vec3f>(i, j)[1] = y / w;
				xyz.at<cv::Vec3f>(i, j)[2] = z / w;
			}
		}
	}
	else
	{
		std::cout << "The type of disparty must be CV_32F" << std::endl;
	}
}
