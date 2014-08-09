#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <sstream>
#include <iostream>
#include "bbbdht22.hpp"

bool DHT22::constructed_ = false;

DHT22::DHT22(const std::string& path, const Pru& pru) :
	halt_(true),
	path_(path),
	pru_(pru),
	temperature_(0.0f),
	humidity_(0.0f),
	cycles_(0),
	errors_(0),
	data_(nullptr)
{
	if(constructed_) throw std::runtime_error("Already constructed");
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	int ret = 0;
	ret = prussdrv_init();
	if(ret ==  -1) throw std::runtime_error("prussdrv_init failed");
	ret = prussdrv_open(PRU_EVTOUT_0);
	if(ret ==  -1) throw std::runtime_error("prussdrv_open failed");
	ret = prussdrv_open(PRU_EVTOUT_1);
	if(ret ==  -1) throw std::runtime_error("prussdrv_open failed");
	ret = prussdrv_pruintc_init(&pruss_intc_initdata);
	if(ret == -1) throw std::runtime_error("prussdrv_pruintc_init failed");
	if (Pru::BPRU0 == pru_){
			ret = prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, (void **)&data_);
	} else if (Pru::BPRU1 == pru_) {
			ret = prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, (void **)&data_);
	}
	if(ret == -1) throw std::runtime_error("prussdrv_map_prumem failed");
	for(int i = 0; i < 100; ++i){
		data_[i] = 0;
	}
	constructed_ = true;
}

DHT22::~DHT22() 
{
	if(process_.joinable()) {
		process_.join();
		std::cout << "Waiting for halt" << std::endl;
		prussdrv_pru_wait_event (PRU_EVTOUT_1);
		std::cout << "Terminating" << std::endl;
		prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
		prussdrv_pru_clear_event(PRU_EVTOUT_1, PRU0_ARM_INTERRUPT);
	}
	if(Pru::BPRU0 == pru_){
		prussdrv_pru_disable(PRU0);
	} else if (Pru::BPRU1 == pru_) {
		prussdrv_pru_disable(PRU1);
	}
}

void DHT22::start() 
{
	int ret = 0;
	if(Pru::BPRU0 == pru_){
		ret = prussdrv_exec_program (PRU0, path_.c_str());
	} else if (Pru::BPRU1 == pru_) {
		ret = prussdrv_exec_program (PRU1, path_.c_str());
	}
	if(ret == -1) 
		throw std::runtime_error("prussdrv_exec_program failed");
		
	halt_ = false;
	process_ = std::thread(&DHT22::run,this);	
	std::cout << "Thread started" << std::endl;
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

std::string DHT22::message() 
{
	std::stringstream ss;
	ss << "Hum: "<< humidity()<<" Temp: "<<temperature()
	   << " Total Errors: "<<errors() 
	   << " Error Rate: "<<error_rate()
	   << " Cycle: "<<cycles()<<std::endl;
	return ss.str();
}

bool DHT22::is_running() 
{return !halt_;}

void DHT22::halt(){
	if(halt_ == true)
	   return;
	halt_ = true;	
}


