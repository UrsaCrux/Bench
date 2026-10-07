#pragma once
#include <CSerialPort/SerialPort.h>
#include <CSerialPort/SerialPortInfo.h>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>

class serial_listener : public itas109::CSerialPortListener {
public:
	bool new_read = false;
	std::string command_n, param_n;
	const char delim = '_';
	std::vector<std::string> commands_a;
	
	
	serial_listener(itas109::CSerialPort* port) : m_port(port) {
		commands_a = std::vector<std::string>();
	}

	void onReadEvent(const char* portname, unsigned int reafBufferlen) override;

private:
	itas109::CSerialPort* m_port;
	std::stringstream accumulator;
};

class connection
{
 public:
	itas109::CSerialPort sp;
	std::string exp_desc = "cp210x";
	std::string port_name_g;
	const char init_command = 192;
	std::thread thread_coms;
	
	serial_listener* listener;
	
	bool init_search();
	std::vector<std::string> get_ports();
	bool connect_port(std::string s);
	bool open_port();
	bool init_connection(serial_listener *n);
	void send_command(std::string command_name, std::string parameter);
	void sending_coms();
	connection();

 private:
	std::atomic<std::string*> comm_a, param_a;
	std::atomic_flag comm_flag = ATOMIC_FLAG_INIT;
	std::atomic_flag new_comm_flag = ATOMIC_FLAG_INIT;
	bool try_connect_esp();
	
};


