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

#define MAXLOOP 123	
#define CPUDONE 0
.struct dht22metadata
		.u32 mask
.ends
// Program entry point, used by debugger only
.entrypoint START 

START:
		// Always make sure to clear the STANDBY_INIT bit in the SYSCFG
		// register, otherwise the PRU will not be able to write outside the
		// PRU memory space and to the BeagleBones pins. The following three
		// lines at the top of a PRU assembly file will accomplish this. LBCO
		// See Section 10.1.2 AMPI
		LBCO r0, C4, 4, 4
		CLR r0, r0, 4
		SBCO r0, C4, 4, 4

		// Set local register r0 to local mem adress of pru
		MOV r0, LOCAL_MEM
		// Set local register r1 to CTBIR0
		MOV r1, CTBIR0
		//Set CTBIR0 to address of local mem
		SBBO r0, r1, 0, 4
		// C24 aka CONST_RAM points to local memory

		//Cycle and error counter
		MOV r18, 0
		MOV r19, 0
MAINLOOP:		
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
		//Initi Bitmask and counter
		MOV r11,  0x80000000
		MOV r12, 0
		//Result goes to r13
		MOV r13, 0
READDATA:	
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
		
DATA_CONTINUE:
		//Increment counter
		ADD r12, r12, 1
		//Shift mask one bit to the right
		LSR r11, r11, 1
		// Loop until all bits are read
		QBGT READDATA, r12, 32
		
		//Read checksum
		MOV r11, 128
		MOV r12, 0
		//Result goes to r14
		MOV r14,0

CHECKSUM:
		waitForSignal PINNR, HIGH 
		waitForSignal PINNR, LOW
		QBLT BITSET_CHECKSUM, r6, 245
		
CHECKSUM_CONTINUE:		
		ADD r12, r12, 1
		LSR r11, r11, 1
		QBGT CHECKSUM, r12, 8 

		//Add temperature and humidity and 8bit wise and compare to
		//checksum. Result goes to r15.
		MOV r15,0
		ADD r15.b0, r13.b0, r13.b1 
		ADD r15.b0, r15.b0, r13.b2
		ADD r15.b0, r15.b0, r13.b3

		//When checksum != sum of humidity + temperature
		//increase error counter and continue.
		QBEQ NOERROR, r15, r14
		ADD r19,r19, 1
NOERROR:		
		ADD r18,r18,1
		SBCO r13, CONST_RAM, 0, 2
		SBCO r13.w2, CONST_RAM, 4, 2
		SBCO r19, CONST_RAM, 8, 4
		SBCO r18, CONST_RAM, 12, 4		
		
		//Inform host that we are done with this cycle by
		//sending PRU_EVENT_0
		//PRU_EVENT_0 needs to be the first event.
		//Don't know why
		//MOV	R31.b0, PRU0_R31_VEC_VALID | 3
		//MOV	R31.b0, PRU0_R31_VEC_VALID | 3
		MOV r20, 1
		SBCO r20, CONST_RAM, 20, 4
WAITCPU:		
		LBCO r20, CONST_RAM, 20, 4
		QBNE WAITCPU, r20,CPUDONE

		//Wait for two seconds to ensure specified behaviour.
		delaysec 2 
		//Check if CPU send terminate symbol
		LBCO r20, CONST_RAM, 16, 4
		//Loop
		QBNE MAINLOOP, r20, MAXLOOP
	
QUIT:	
		//// tell host we are done, then halt
		MOV	R31.b0, PRU0_R31_VEC_VALID | 4 
		HALT
BITSET:
		OR r13, r13, r11 
		JMP DATA_CONTINUE

BITSET_CHECKSUM:
		OR r14, r14, r11
		JMP CHECKSUM_CONTINUE
		
