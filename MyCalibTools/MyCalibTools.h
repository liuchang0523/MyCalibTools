#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MyCalibTools.h"

class MyCalibTools : public QMainWindow
{
	Q_OBJECT

public:
	MyCalibTools(QWidget *parent = Q_NULLPTR);

private:
	Ui::MyCalibToolsClass ui;
};
