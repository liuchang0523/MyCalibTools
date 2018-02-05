#include "MyCalibTools.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QtCore\qxmlstream.h>
#include <QtCore\QProcess>

MyCalibTools::MyCalibTools(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	this->move(1920, 0);
	this->setWindowState(Qt::WindowMaximized);
	m_parent_path_ = ".";
	m_image_num = 0;
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
	cv::Canny(m_image_gray_, m_image_canny_, 100, 300);
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
		cv::circle(m_image_source_, centers_sorted[i], 3, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);
	}

	cv::imwrite("center.bmp", m_image_source_);

	// 	std::string image_name = std::to_string(m_image_num);
	// 	image_name += ".bmp";
	// 	cv::imwrite(image_name, m_image_source_);
	//m_image_num++;
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
		cv::Canny(m_image_gray_, m_image_canny_, 100, 300);
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
				if (isCircle(m_contours_[i], center, 30.0))//圆度阈值
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
			QMessageBox::warning(this, QStringLiteral("大圆数目有误！"), QStringLiteral("大圆数目有误！%1").arg(k));
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
	qDebug() << "finished";
}

void MyCalibTools::on_actionCalibration_triggered()
{
	if (m_points_on_image_.size() < 5)
	{
		return;
	}
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
	std::vector<std::vector<cv::Point2f>> points_on_image_1;
	std::vector<std::vector<cv::Point2f>> points_on_image_2;
	std::vector<std::vector<cv::Point3f>> points_on_world;
	on_actionOpenList_triggered();
	points_on_image_1 = m_points_on_image_;
	//左相机单目标定
	on_actionCalibration_triggered();
	cv::Mat matrix_1 = m_matrix_;
	cv::Mat dist_1 = m_distortion_;

	on_actionOpenList_triggered();
	points_on_image_2 = m_points_on_image_;
	points_on_world = m_points_on_world_;
	on_actionCalibration_triggered();
	if (points_on_image_1.size() != points_on_image_2.size())
	{
		QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("图片数目不相等！"));
		return;
	}

	cv::Mat matrix_2 = m_matrix_;
	cv::Mat dist_2 = m_distortion_;

	cv::Mat R, T, E, F;
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
	qDebug() << Mat2QString(R);
	QFile file("StereoCalibrateResult.xml");
	if (!file.open(QFile::WriteOnly | QFile::Text))
	{
		qDebug() << "Error: cannot open file";
		return;
	}
	QXmlStreamWriter stream(&file);
	stream.setAutoFormatting(true);
	stream.writeStartDocument();
	stream.writeTextElement("matrix_1", Mat2QString(matrix_1));
	stream.writeTextElement("distortion_1", Mat2QString(dist_1));
	stream.writeTextElement("matrix_2", Mat2QString(matrix_2));
	stream.writeTextElement("distortion_2", Mat2QString(dist_2));
	stream.writeTextElement("R", Mat2QString(R));
	stream.writeTextElement("T", Mat2QString(T));
	stream.writeTextElement("e", QString::number(e, 'g', 10));
	stream.writeEndElement();
	file.close();

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
