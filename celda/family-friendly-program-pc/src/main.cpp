#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <filesystem>
#include <fstream>

#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <ctime>

#include "settings.h"
#include "main_window.hpp"
#include "connection.hpp"
#include "painter.hpp"

serial_listener *listener;
painter *painter_;
connection con;
main_window *window;

bool connected = false;

int* settle = new int(0);

std::vector<int> *numbers = nullptr;


void warn_send(std::string mes, std::string title = "Alerta"){
	QMessageBox::warning(
			     nullptr,
			     QString::fromStdString(title),
			     QString::fromStdString(mes)
			     );
	
	return;
}

void info_send(std::string mes, std::string title = "Info"){
	QMessageBox::information(
				 nullptr,
				 QString::fromStdString(title),
				 QString::fromStdString(mes)
				 );
}

void export_numbers_to_csv(std::vector<int> n){
	try{
		
	std::filesystem::path dir_p = O_PATH;

	std::time_t now = std::time(nullptr);
	std::tm* local_time = std::localtime(&now);

	if (!std::filesystem::exists(dir_p)){
		std::filesystem::create_directories(dir_p);
		std::cout << "- Directorio " << dir_p << " creado!" << std::endl;
	}

	char buffer_t[80];
	std::strftime(buffer_t, sizeof(buffer_t), "output-%d-%m-(%H:%M:%S).csv", local_time);
	std::filesystem::path file_p = dir_p / buffer_t;
	std::ofstream file(file_p);

	if (!file.is_open()){
		std::cout << "- Fallo al crear el archivo!" << std::endl;
		return;
	}

	//titulos de columnas:
	file << "N muestra" << "," << "Newton" << "," << "Tiempo" << "\n";
	
	for (int i = 0; i < numbers->size(); i++){
		file << i << "," << numbers->at(i) << "," << "---" << "\n";
	}

	file.close();
	
	std::cout << "- Se creó el archivo csv: " << file_p << std::endl;
	info_send(std::string("Se creó exitosamente el archivo: ") + buffer_t);
	}
	catch(const std::filesystem::filesystem_error& e){
		std::cerr << "- Filesystem error: " << e.what() << std::endl;
	}
	
	return;
}

void connection_export(){
	std::thread ex_tr(export_numbers_to_csv, *numbers);

	ex_tr.detach();
}

void activate_export(){
	window->activate_e();
}

void deactivate_export(){
	window->button_expo->setEnabled(false);
}

void start_plot(){
	if (painter_->drawing.test()){
		painter_->drawing.clear();
		window->button_read->setText(B_SENS);
		activate_export();
		return;
	}

	deactivate_export();
	painter_->set_connect(con.listener);
	con.send_command("mode", "1");
	painter_->start_drawing();

	numbers = &painter_->numbers;

	window->button_read->setText(B_STOP);
}

void stop_plot(){
	if (painter_->drawing.test())
		painter_->drawing.clear();
	return;
}

int main(int argc, char *argv[]){
	QApplication app(argc, argv);
	
        window = new main_window(con, settle);
		
	std::cout << "- Using godfucking Qt5 library for shitgraphics!" << std::endl;
	std::cout << "- CSerialPort Library version: " << con.sp.getVersion() << std::endl;
		
	window->connect(window->button_read, &QPushButton::clicked, start_plot);
	window->connect(window->button_expo, &QPushButton::clicked, connection_export);
	
	window->resize(W_WIDTH, W_HEIGHT);
	window->setWindowTitle(W_NAME);

	window->setAttribute(Qt::WA_DeleteOnClose);

	window->show();

	painter_ = window->painter_;

	return app.exec();
}
