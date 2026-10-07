#pragma once

#include <QWidget>
#include <QString>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>

#include <iostream>
#include <string>
#include <functional>

#include "settings.h"
#include "connection.hpp"
#include "painter.hpp"

class choosing : public QWidget
{
 Q_OBJECT
	
 public:
	choosing(connection &con, QWidget *parent = nullptr);

	void connect_esp();
	void add_items();

	std::function<void()> allow_start;
	
	QVBoxLayout *layout;
	QComboBox *desplegable;
	QPushButton *button;

	connection *conek;
	painter* paint;
	
	std::string title;
	std::vector<std::string> ports;
	std::string choosed_port;
	
 private:

	
};
