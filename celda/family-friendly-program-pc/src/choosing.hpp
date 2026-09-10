#pragma once

#include <QWidget>
#include <QString>

#include <string>

class choosing : public QWidget
{
 Q_OBJECT
	
 public:
	choosing(QWidget *parent = nullptr);
	std::string title;
	std::vector<std::string> ports;
	std::string choosed_port;
	
 private:

	
};
