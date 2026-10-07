#include "main_window.hpp"

#include "settings.h"



main_window::main_window(connection &con, int *sett, QWidget *parent) : QMainWindow(parent)
{
	this->settle = sett;
	
	central_widget = new QWidget(this);
	setCentralWidget(central_widget);

	choose_port = new choosing(con, this);
	painter_ = new painter(this);

	choose_port->paint = painter_;
	choose_port->allow_start = std::bind(&main_window::activate_s, this);

	top = new QWidget(this);
	bottom = new QWidget(this);
	right_top = new QWidget(this);

	main_layout = new QVBoxLayout(central_widget);
	main_layout->addWidget(top);
	main_layout->addWidget(bottom);
	
	top_layout = new QHBoxLayout(top);
	bottom_layout = new QHBoxLayout(bottom);
	right_buttons = new QVBoxLayout(right_top);

	painter_->setStyleSheet(
			     "background-color: black;"\
			     "border: 6px solid;"\
			     "border-radius: 5px;"\
			     "border-color: #222222 #666666 #666666 #444444;"
			     );
	painter_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	button_read = new QPushButton(B_SENS, this);
	button_cali = new QPushButton(B_CALI, this);
	button_expo = new QPushButton(B_EXPO, this);

	button_read->setEnabled(false);
	button_cali->setEnabled(false);
	button_expo->setEnabled(false);
	
	top_layout->addWidget(painter_);
	top_layout->addWidget(right_top);
	bottom_layout->addWidget(choose_port);
	bottom_layout->addWidget(button_cali);
	bottom_layout->addWidget(button_read);
	right_buttons->addWidget(button_expo);

}

void main_window::activate_s(){
	button_read->setEnabled(true);
	return;
}

void main_window::activate_e(){
	button_expo->setEnabled(true);
}
