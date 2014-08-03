#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <thread>
#include <future>
#include <mutex>   
#include <csignal>

// We are using PRU0


#define PRU_NUM 0

std::mutex mtx;
std::atomic<bool> stop;
unsigned int *data;

float temperature;
float humidity;
int error;

void test() 
{
	while(true) {
		prussdrv_pru_wait_event(PRU_EVTOUT_0);
		prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
		{
			std::lock_guard<std::mutex> lck(mtx);
			int t_temp = data[0];
			int t_hum = data[1];
			error = data[2];
			if(error)
			   break;
			if(t_temp & 80000000)
				temperature = static_cast<float>(t_temp)*-0.1f;
			else
			   temperature = static_cast<float>(t_temp) * 0.1f;
			humidity = static_cast<float>(t_hum)* 0.1f;
		}	
		if(stop == true){
			data[3] = 123;
			break;
		}
	}
}

void handler(int param)
{stop = true;}

int main(int argc, char **argv) {

	stop = false;
	temperature = 0.0f;
	humidity = 0.0f;
	error = 0;
	
	std::signal(SIGINT, handler);
	
	std::string path = "prudht22.bin";
	// Interrupt 
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

	std::cout << "Starting example" << std::endl;
   
	// Initialize the PRU
	prussdrv_init ();
	

	// Open PRU interrupt
	unsigned int ret = prussdrv_open(PRU_EVTOUT_0);
	if (ret) {
		std::cout << "prussdrv_open failed" << std::endl;
		return -1;
	}

	ret = prussdrv_open(PRU_EVTOUT_1);
	if (ret) {
		std::cout << "prussdrv_open failed" << std::endl;
		return -1;
	}


	// Initialize interrupt 
	prussdrv_pruintc_init(&pruss_intc_initdata);

	// Intialize ram
	std::cout << "Initialize Ram" << std::endl;

	if( PRU_NUM == 0 ) 
	   prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, (void **)&data);
	else
	   prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, (void **)&data);
 
	// Flush memory
	for(int i = 0; i < 100; ++i){
		data[i] = 0;
	}
	
	
	// Executing program
	std::cout << "Executing program" << std::endl;
	prussdrv_exec_program (PRU_NUM, "prudht22.bin");
	
	// Wait for PRU to halt
	std::thread processing(test);
	
	while(true){
		
		if((stop == true) || (error)) break;
		{
			std::lock_guard<std::mutex> lck(mtx);
			std::cout << "Temp: "<<temperature << std::endl;
			std::cout << "Humi: "<<humidity << std::endl;
		}	
		std::chrono::milliseconds dura( 2000 );
		std::this_thread::sleep_for( dura );
	}		   
	processing.join();

	// int i = 0;
	// while(true) {
	// 	std::cout <<prussdrv_pru_wait_event(PRU_EVTOUT_0)<<std::endl;
	// 	prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
	// 	std::cout << "Temperature: " <<data[0]<< std::endl;
	// 	std::cout << "Humidity: " <<data[1]<< std::endl;
	// 	if(i == 9){
	// 		data[2] = 123;
	// 		break;
	// 	}
	// 	i++;	
	// }
	
	std::cout << "Waiting for halt" << std::endl;
	std::cout << prussdrv_pru_wait_event (PRU_EVTOUT_1)<<std::endl;

	// Cleanup
	prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
	prussdrv_pru_clear_event (PRU_EVTOUT_1, PRU0_ARM_INTERRUPT);
	prussdrv_pru_disable (PRU_NUM);
	prussdrv_exit ();
}
