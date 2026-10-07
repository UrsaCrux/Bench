
void reset_esp(connection& c){
	c.sp.setDtr(false);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	c.sp.setDtr(true);
	c.sp.setRts(true);
	return;

}


void choose_port(){
	std::cout << "- Eligiendo puerto" << std::endl;
	return;
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
		std::cerr << "No se puede abrir el puerto "
			  << con.port_name_g << std::endl;
		choose_port();
	}
	else{
		std::cout << "- Puerto " << con.port_name_g << " abierto!" << std::endl;
	}

	reset_esp(con);

	
	if (!con.init_connection(listener)){
		std::cerr << "No se pudo iniciar una conexion con la celda!" << std::endl;
		warn_send("No se pudo iniciar una conexion con la celda, inteta otro puerto.");
		return;
	}
	else{
		std::cout << "- Conexion iniciada con la celda!" << std::endl;
	}
	return;
}
