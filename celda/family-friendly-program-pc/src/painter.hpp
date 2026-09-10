#pragma once
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QWidget>
#include <QGraphicsView>
#include <thread>
#include <iostream>
#include <chrono>
#include <vector>

#include "connection.hpp"

class painter : public QWidget
{
	Q_OBJECT
 public:
	painter(QWidget *parent);

	serial_listener *listen;
	//QPainter painter_n;
	QPen pen;
	QBrush brush;
	
	bool drawing = false;

	float max_ = 10.0;
	float roof = 100.0;
	int steps_v = 10;
	int steps_h = 10;

	std::vector<int> numbers;

	void set_connect(serial_listener *listene);
	void start_drawing();
	void drawing_listen();

	void paintEvent(QPaintEvent *event) override;
	
 private:
	std::thread thread_draw;

	
};
