#include <iostream>
#include <vector>
#include <thread>

#include <QApplication>
#include <QMessageBox>
#include <QString>


#include "settings.h"
#include "main_window.hpp"
#include "connection.hpp"
#include "painter.hpp"

serial_listener *listener;
painter *painter_;
connection con;
main_window *window;

void reset_esp(connection& c){
	c.sp.setDtr(false);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	c.sp.setDtr(true);
	c.sp.setRts(true);
	return;

}

void warn_send(std::string mes, std::string title = "Alerta"){
	QMessageBox::warning(
			     &*window->central_widget,
			     QString::fromStdString(title),
			     QString::fromStdString(mes)
			     );
	return;
}

void choose_port(){
	std::cout << "- Eligiendo puerto" << std::endl;
	return;
}

void start_plot(){
	con.send_command("mode", "1");
	//std::cout << "empezando" << std::endl;
	painter_->start_drawing();
}

void init_connect(){
	if (!con.init_search()){
		std::cerr << "- No tienes conectada la celda!" << std::endl;
		choose_port();
		return;
	}
	else{
		std::cout << "- Conectado a la celda!" << std::endl;
	}


	listener = new serial_listener(&con.sp);
	con.sp.connectReadEvent(listener);
	
	painter_->set_connect(listener);
	
	if (!con.open_port()){
		std::cerr << "No se puede abrir el puerto " << con.port_name_g << std::endl;
		//warn_send("No se pudo abrir el puerto (o te equivocaste de puerto), da los permisos necesarios o intenta cerrando algun monitor serial (luego vuelve a ejecutar el programa)");
		choose_port();
	}
	else{
		std::cout << "- Puerto " << con.port_name_g << " abierto!" << std::endl;
	}

	reset_esp(con);

	
	if (!con.init_connection(listener)){
		std::cerr << "No se pudo iniciar una conexion con la celda!" << std::endl;
		return;//mientras
	}
	else{
		std::cout << "- Conexion iniciada con la celda!" << std::endl;
	}
	return;
}

int main(int argc, char *argv[]){
	QApplication app(argc, argv);
	//connection con;
        window = new main_window();
		
	std::cout << "- Using godfucking Qt5 library for shitgraphics!" << std::endl;
	std::cout << "- CSerialPort Library version: " << con.sp.getVersion() << std::endl;
	
	//painter_->set_connect(listener);
	
	window->connect(window->button_read, &QPushButton::clicked, start_plot);
	
	window->resize(W_WIDTH, W_HEIGHT);
	window->setWindowTitle(W_NAME);

	window->setAttribute(Qt::WA_DeleteOnClose);

	window->show();

	painter_ = window->painter_;

	std::thread thread_conn(init_connect);
	thread_conn.detach();

	return app.exec();
}
