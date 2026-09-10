#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QMessageBox>

#include <atomic>

#include "painter.hpp"
#include "choosing.hpp"

class main_window : public QMainWindow
{
	Q_OBJECT
 public:
	main_window(QWidget *parent = nullptr);

	QWidget *central_widget;

	choosing *choose_port;

	QPushButton *button_read;
	QPushButton *button_cali;
	QPushButton *button_expo;

	painter *painter_;
	
 private:
	//QWidget *panel;
	QWidget *top;
	QWidget *bottom;
	QWidget *right_top;
	
	QVBoxLayout *main_layout;
	QHBoxLayout *bottom_layout;
	QHBoxLayout *top_layout;
	QVBoxLayout *right_buttons;
};
