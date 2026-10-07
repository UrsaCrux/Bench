#include "choosing.hpp"



choosing::choosing(connection &con, QWidget *parent) : QWidget(parent)
{
	title = "Elige un puerto";

	layout = new QVBoxLayout(this);
	desplegable = new QComboBox(this);
	
	button = new QPushButton(B_CONE, this);

	layout->addWidget(desplegable);
	layout->addWidget(button);

	conek = &con;

	this->connect(button, &QPushButton::clicked, this, &choosing::connect_esp);

	this->add_items();
}

void choosing::connect_esp(){
	
	std::string port_name = desplegable->currentText().toStdString();
	
	if (conek->connect_port(port_name)) {
		std::cout << "Connected to port: " << port_name << std::endl;
		allow_start();
	}
	else{
		std::cout << "Cannot connect to port: " << port_name << std::endl;
		QMessageBox::warning(
			     nullptr,
			     QString("Alerta"),
			     QString("No se pudo conectar a la celda, intente otro puerto")
			     );
		return;
	}

	//si se conecto:
	
	
	
}

void choosing::add_items(){
	std::vector<std::string> ports = conek->get_ports();

	for (auto i : ports){
		desplegable->addItem(QString::fromStdString(i));
	}
}
