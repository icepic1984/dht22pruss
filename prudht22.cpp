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
// We are using PRU0

#define PRU_NUM 0

void test() 
{
	std::cout<<"Sart"<<std::endl;
	
	int c = 0;
	while(true) {
		prussdrv_pru_wait_event(PRU_EVTOUT_1);
		prussdrv_pru_clear_event(PRU_EVTOUT_1, PRU0_ARM_INTERRUPT);
		c++;
		std::cout << "Counter: "<<c << std::endl;
	}
	std::cout<<"End"<<std::endl;

}

int main(int argc, char **argv) {

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
	unsigned int *data;
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
	int i = 0;
	while(true) {
		std::cout <<prussdrv_pru_wait_event(PRU_EVTOUT_0)<<std::endl;
		prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
		std::cout << "Temperature: " <<data[0]<< std::endl;
		std::cout << "Humidity: " <<data[1]<< std::endl;
		if(i == 9){
			data[2] = 123;
			break;
		}
		i++;	
	}

	std::cout << "Waiting for halt" << std::endl;
	std::cout << prussdrv_pru_wait_event (PRU_EVTOUT_1)<<std::endl;

	// Cleanup
	prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
	prussdrv_pru_clear_event (PRU_EVTOUT_1, PRU0_ARM_INTERRUPT);
	prussdrv_pru_disable (PRU_NUM);
	prussdrv_exit ();
}
