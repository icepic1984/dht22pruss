#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <iostream>
#include "bbbdht22.hpp"

DHT22::DHT22(const std::string& path) :
	halt_(true),
	path_(path),
	temperature_(0.0f),
	humidity_(0.0f),
	cycles_(0),
	errors_(0),
	data_(nullptr)
{
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	prussdrv_init ();
	prussdrv_open(PRU_EVTOUT_0);
	prussdrv_open(PRU_EVTOUT_1);
	prussdrv_pruintc_init(&pruss_intc_initdata);
	prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, (void **)&data_);
	//Flush memory
	std::cout << "Flash Memory" << std::endl;
	for(int i = 0; i < 100; ++i){
		data_[i] = 0;
	}
}

DHT22::~DHT22() 
{
	process_.join();
	std::cout << "Waiting for halt" << std::endl;
	prussdrv_pru_wait_event (PRU_EVTOUT_1);
	std::cout << "Terminating" << std::endl;
	prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
	prussdrv_pru_clear_event(PRU_EVTOUT_1, PRU0_ARM_INTERRUPT);
	prussdrv_pru_disable(0);
}

void DHT22::start() 
{
	std::cout << "Start Pru" << std::endl;
	prussdrv_exec_program (0, path_.c_str());
	std::cout << "Start Thread" << std::endl;
	halt_ = false;
	process_ = std::thread(&DHT22::run,this);	
}

void DHT22::run()
{
	while(true) {
		prussdrv_pru_wait_event(PRU_EVTOUT_0);
		prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
		//Read data from pru
		{
			std::lock_guard<std::mutex> lck(mtx_);
			int t_temp = data_[0];
			int t_hum = data_[1];
			errors_ = data_[2];
			cycles_ = data_[3];
			   
			if(t_temp & 80000000)
			   temperature_ = static_cast<float>(t_temp)*-0.1f;
			else
			   temperature_ = static_cast<float>(t_temp) * 0.1f;
			humidity_ = static_cast<float>(t_hum)* 0.1f;
		}	
		if(halt_ == true){
			data_[4] = STOP;
			break;
		}
	}
}

float DHT22::temperature() 
{
	std::lock_guard<std::mutex> lck(mtx_);
	return temperature_;
}
	
float DHT22::humidity() 
{
	std::lock_guard<std::mutex> lck(mtx_);
	return humidity_;
}

float DHT22::error_rate()
{
	std::lock_guard<std::mutex> lck(mtx_);
	return static_cast<float>(errors_) / static_cast<float>(cycles_);
}

int DHT22::errors()
{
	std::lock_guard<std::mutex> lck(mtx_);
	return errors_;
}

int DHT22::cycles() 
{
	std::lock_guard<std::mutex> lck(mtx_);
	return cycles_;
}

bool DHT22::is_running() 
{return !halt_;}

void DHT22::halt(){
	if(halt_ == true)
	   return;
	halt_ = true;	
}


