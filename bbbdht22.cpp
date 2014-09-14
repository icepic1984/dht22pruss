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
	pruss_wrapper(prussdrv_init,"prussdrv_init");
	pruss_wrapper(prussdrv_open,"prussdrv_open", PRU_EVTOUT_0);
	pruss_wrapper(prussdrv_open,"prussdrv_open", PRU_EVTOUT_1);
	pruss_wrapper(prussdrv_pruintc_init, "prussdrv_pruintc_init",
				  &pruss_intc_initdata);
	if (Pru::BPRU0 == pru_){
		pruss_wrapper(prussdrv_map_prumem,"prussdrv_map_prumem",
					  PRUSS0_PRU0_DATARAM, (void **)&data_);
	} else if (Pru::BPRU1 == pru_) {
		pruss_wrapper(prussdrv_map_prumem,"prussdrv_map_prumem",
					  PRUSS0_PRU1_DATARAM, (void **)&data_);
	}
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
		pruss_wrapper(prussdrv_pru_wait_event,"prussdrv_pru_wait_event",
					  PRU_EVTOUT_1);
		std::cout << "Terminating" << std::endl;
		pruss_wrapper(prussdrv_pru_clear_event,"prussdrv_pru_clear_event ev0",
					  PRU_EVTOUT_0,PRU0_ARM_INTERRUPT);
		pruss_wrapper(prussdrv_pru_clear_event,"prussdrv_pru_clear_event ev1",
					  PRU_EVTOUT_1,PRU0_ARM_INTERRUPT);
	}
	if(Pru::BPRU0 == pru_){
		pruss_wrapper(prussdrv_pru_disable,"prussdrv_pru_disable", PRU0);
	} else if (Pru::BPRU1 == pru_) {
		pruss_wrapper(prussdrv_pru_disable,"prussdrv_pru_disable", PRU1);
	}
}

void DHT22::start() 
{
	int ret = 0;
	if(Pru::BPRU0 == pru_){
		pruss_wrapper(prussdrv_exec_program,"prussdrv_exec_program",
					  PRU0,path_.c_str());
	} else if (Pru::BPRU1 == pru_) {
		pruss_wrapper(prussdrv_exec_program,"prussdrv_exec_program",
					  PRU1, path_.c_str());
	}
	halt_ = false;
	process_ = std::thread(&DHT22::run,this);	
	std::cout << "Thread started" << std::endl;
}

void DHT22::run()
{
	while(true) {
		 std::cout << "Wait for pru" << std::endl;
		 while(read_from_pru(STATUS) != 1)
		   std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		std::cout << "Cycle done" << std::endl;
		{
			std::lock_guard<std::mutex> lck(mtx_);
			int t_temp = read_from_pru(TEMP);
			int t_hum = read_from_pru(HUM);
			int t_errors = read_from_pru(ERROR);
			if(t_errors == errors_){
				cycles_ = read_from_pru(CYCLE);
				if(t_temp & 80000000)
				   temperature_ = static_cast<float>(t_temp)*-0.1f;
				else
				   temperature_ = static_cast<float>(t_temp) * 0.1f;
				humidity_ = static_cast<float>(t_hum)* 0.1f;
				write_to_pru(STATUS, 0);
			} 
			errors_ = t_errors;
		}	
		if(halt_ == true){
			write_to_pru(HALT,STOP);
			break;
		}
	}
}

unsigned int DHT22::read_from_pru(DataField field)
{return data_[field];}

void DHT22::write_to_pru(DataField field, unsigned int value)
{data_[field] = value;}

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


