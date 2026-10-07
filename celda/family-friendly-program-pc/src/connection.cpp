#include "connection.hpp"



connection::connection()
{
	comm_a.store(new std::string(""));
	param_a.store(new std::string(""));
}

bool connection::init_search() {
	std::vector<itas109::SerialPortInfo> ports = itas109::CSerialPortInfo::availablePortInfos();
	if (ports.empty()){
		return false;
	}

	bool enc = false;
	std::string port_name = "";
	
	for (const auto& i : ports) {
		//std::cout << i.description << std::endl;
		if (exp_desc.compare(0, 6, i.description, 0, 6) == 0){
			enc = true;
			port_name = i.portName;
			port_name_g = port_name;
		}
	}

	if (enc){
		sp.init(port_name.c_str(), 115200, itas109::ParityNone, itas109::DataBits8, itas109::StopOne);
	}

	return enc;
	
}

void connection::sending_coms(){
	comm_flag.test_and_set();
	while(comm_flag.test()){
		new_comm_flag.wait(false);

		std::string data_ = *(comm_a.load());
		data_ += "_";
		data_ += *(param_a.load());
		sp.writeData(data_.c_str(), data_.size());
		
		new_comm_flag.clear();
	}
}

void connection::send_command(std::string command_name, std::string parameter){
	std::string *new_comm = new std::string(command_name);
	std::string *new_param = new std::string(parameter);
	
	std::string *old_comm = comm_a.exchange(new_comm);
	std::string *old_param = param_a.exchange(new_param);

	delete old_comm;
	delete old_param;

	new_comm_flag.test_and_set();
	new_comm_flag.notify_one();
}

bool connection::connect_port(std::string s){
	sp.init(s.c_str(), 115200, itas109::ParityNone, itas109::DataBits8, itas109::StopOne);

	return try_connect_esp();
}

bool connection::try_connect_esp(){
	listener = new serial_listener(&sp);
	sp.connectReadEvent(listener);

	this->open_port();

	sp.setDtr(false);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	sp.setDtr(true);
	sp.setRts(true);

	int t_c = 0;
	int max_t = 10;
	
	while (true){
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		t_c++;

		if (t_c > max_t)
			return false;
		
		if (!listener->new_read)
			continue;

		if (listener->command_n != "init")
			continue;

		char n2 = 192;
		sp.writeData(&n2, 1);

		thread_coms = std::thread(&connection::sending_coms, this);
		thread_coms.detach();

		std::cout << "- Thread sending coms detached!" << std::endl;

		break;
		
	}

	
	
	return true;
}

bool connection::open_port(){
	return sp.open();
}

bool connection::init_connection(serial_listener *n){


        while (true){
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (n->new_read){

			if (n->command_n == "init"){
				char n2 = 192;
				sp.writeData(&n2, 1);

				thread_coms = std::thread(&connection::sending_coms, this);
				thread_coms.detach();
				std::cout << "- thread sending coms detached!" << std::endl;
		
				int inn = 0;
				while(inn < 1000) {
					inn++;
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					if (n->new_read){
						if (n->command_n == "init" && n->param_n == "oo"){
					//send_command("mode", "1");
					/*std::string data_ = "mode_1";
					  sp.writeData(data_.c_str(), data_.size());*/
							return true;	
						}
					}
				}
			}

		}	
	}

	

	return false;
}


std::vector<std::string> connection::get_ports(){
	std::vector<std::string> ret_ports = std::vector<std::string>();
	std::vector<itas109::SerialPortInfo> ports = itas109::CSerialPortInfo::availablePortInfos();

	for (const auto& i : ports){
		ret_ports.push_back(i.portName);
	}

	return ret_ports;
}


//------------------------------------

void serial_listener::onReadEvent(const char* portname, unsigned int readBufferLen){
	if (readBufferLen > 0){
		char* data_buff = new char[readBufferLen];
		int bytes_read = m_port->readData(data_buff, readBufferLen);

		if (bytes_read > 0){
			//			data_buff[bytes_read] = '\0';
			//accumulator << data_buff;

			std::stringstream ss(data_buff);
			std::string t;

			//std::cout << data_buff << std::endl;

			std::getline(ss, t, delim);
			command_n = t;

			if (std::getline(ss, t, '\n')){
				param_n = t.substr(0, t.size()-1);
				new_read = true;
				std::cout << command_n << " : " << param_n << std::endl;
			}
		        
		}

		/*std::string m_c;
		while (std::getline(accumulator, m_c, '\n')){
			std::cout << m_c << std::endl;
			}*/

		

		delete[] data_buff;
	}
}


