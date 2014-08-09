#include <iostream>
#include <csignal>
#include <sstream>
#include <boost/asio.hpp>
#include "bbbdht22.hpp"
#include "server.hpp"

using boost::asio::ip::tcp;

bool stop = false;

void handler(int param)
{stop = true;}

int main(int argc, char **argv) {

	std::signal(SIGINT, handler);
	std::signal(SIGTERM, handler);
	std::string path = "prudht22.bin";

 	DHT22Ptr_t dht22(new DHT22(path,Pru::BPRU0));
	boost::asio::io_service io_service;
	Server server(dht22,io_service);
 	dht22->start();
	std::thread st([&io_service](){io_service.run();});
	
	 while(dht22->is_running()) {
		 if(stop)
			dht22->halt();
		 std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	 
	 }
	 io_service.stop();
	 st.join();
}


