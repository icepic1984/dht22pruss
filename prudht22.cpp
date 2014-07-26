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
// We are using PRU0
#define PRU_NUM 0


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
	
	//std::cout << "Bla" <<prussdrv_pru_wait_event (PRU_EVTOUT_0)<<std::endl;
	// Wait for PRU to halt
	std::cout << "Waiting for halt" << std::endl;
	
	std::cout << prussdrv_pru_wait_event(PRU_EVTOUT_1)<<std::endl;
	prussdrv_pru_clear_event (PRU_EVTOUT_1, PRU0_ARM_INTERRUPT);
	std::cout << prussdrv_pru_wait_event(PRU_EVTOUT_1) << std::endl;
	prussdrv_pru_clear_event (PRU_EVTOUT_1, PRU0_ARM_INTERRUPT);

	prussdrv_pru_wait_event (PRU_EVTOUT_0);
	std::cout << "Done" << std::endl;
	
	std::cout << "Temp: " <<data[0]<< std::endl;
	std::cout << "Humi: " <<data[1]<< std::endl;
	std::cout << "Check: " <<data[2]<< std::endl;
	std::cout << "Add: " <<data[3]<< std::endl;
	std::cout << "All: " <<data[4]<<std::endl;

	std::cout << "Temp0: " <<data[5]<< std::endl;
	std::cout << "Temp1: " <<data[6]<< std::endl;
	std::cout << "Humi0: " <<data[7]<< std::endl;
	std::cout << "Humi1: " <<data[8]<< std::endl;



	// Cleanup
	prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
	prussdrv_pru_disable (PRU_NUM);
	prussdrv_exit ();
}
