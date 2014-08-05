#include <iostream>
#include <csignal>
#include "bbbdht22.hpp"

// We are using PRU0
//#define PRU_NUM 0
bool stop = false;

void handler(int param)
{stop = true;}

int main(int argc, char **argv) {

	std::signal(SIGINT, handler);
	std::string path = "prudht22.bin";

	DHT22 dht22(path);
	dht22.start();
	while(dht22.is_running()){
		if(stop)
		   dht22.halt();
		std::cout << "Temp: "<<dht22.temperature()
				  << " Hum: "<<dht22.humidity()
				  << " Cycle: "<<dht22.cycles()
				  << " Error: "<<dht22.error_rate()
				  << "Total Error: "<<dht22.errors()<< std::endl;
	}	
}
