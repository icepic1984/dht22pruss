// PRU Driver for DHT22 temperature and humidity sensor
//
// Author: IcePic
// Email: icepic2009@googlemail.com
//
// Table and Section numbers refer to AM335x PRU-ICSS Ref. (AMPI) or
// to AM335x ARM Cortex-A8 Technical Reference Manual (AMTR)
//
// Code starts here:

// Offset of the start of the code in PRU memory
.origin 0 

// Include helper functions
#include "utils.hp"		

// When everything is setup properply C24 points to local memory
#define CONST_RAM C24
		
//Using pin number
#define PINNR 16

// Define low and high signals for input
#define LOW 0
#define HIGH 1

// waitForSignal need approx. 40 cycles
#define WFS 40
	
// Program entry point, used by debugger only
.entrypoint START 

START:
		// Always make sure to clear the STANDBY_INIT bit in the SYSCFG
		// register, otherwise the PRU will not be able to write outside the
		// PRU memory space and to the BeagleBone's pins. The following three
		// lines at the top of a PRU assembly file will accomplish this. LBCO
		// See Section 10.1.2 AMPI
		LBCO r0, C4, 4, 4
		CLR r0, r0, 4
		SBCO r0, C4, 4, 4

		//**********Initialize local memory**********
		// Set local register r0 to local mem adress of pru
		MOV r0, LOCAL_MEM
		// Set local register r1 to CTBIR0
		MOV r1, CTBIR0
		//Set CTBIR0 to address of local mem
		SBBO r0, r1, 0, 4
		// C24 aka CONST_RAM points to local memory
		// We are done
		//**********Done**********
	
		// When communication between MCU and DHT begins MCU will pull down
		// data bus for 1~10ms. Therefor, pull down and wait for 1ms.
		setPinMode PINNR, GPIO_OUTPUT
		digitalWrite PINNR, GPIO_LOW
		delayms 1 

		//Then MCU will pull up signal and wait for 20-40us. Therefore:
		//Pull up and wait for 40ms
		digitalWrite PINNR, GPIO_HIGH 
		delayus 40

		//When DHT detects signal, it will pull down the bus for 80us.
		setPinMode PINNR, GPIO_INPUT
		waitForSignal PINNR, HIGH

		//DHT22 pulls up bus for 80us for preparation to send data.
		waitForSignal PINNR, LOW 

		//DHT22 sends 40bits to mcu:
		//First 16Bit: Humidity
		//Second 16Bit: Temperature
		//Last 8Bit:  Checksum
		//Loop for 40 bits
		MOV r1, 0
		MOV r8, 0
		MOV r9,0
		SET r9.t31
READB:	
		//When DHT22 is sending data to mcu, every transmission begins
		//with a low signal which last for 50us
		waitForSignal PINNR, HIGH 

		//The following high-voltage signal decides if bit is 0 or 1.
		//Signal is high for 26-28us => 0
		//Signal is high for 70us    => 1
		//We measure, how long the signal is high. For more then 40us 
		//we set the current bit to 1.
		waitForSignal PINNR, LOW 
		//waitForSignal takes approx 40 cycles per loop
		//BBB runs with 200 instructions per us
		//245 loop runs are approx 50us = 245 * 40 / 200	
		QBLT BITSET, r6, 245
		


		//Increment counter
CONT:	ADD r1, r1, 1
		LSR r9, r9, 1
		// Loop until all bits are read
		QBGT READB, r1, 31
		
		SBCO r8, CONST_RAM, 4, 4
		//// tell host we are done, then halt
		MOV	R31.b0, PRU0_R31_VEC_VALID | SIGNUM
		HALT

BITSET:	OR r8, r8, r9
		JMP CONT
		
