// PRU Driver for DHT22 temperature and humidity sensor
//
// Author: IcePic
// Email: icepic2009@googlemail.com
//
// Table and Section numbers refer to AM335x PRU-ICSS Ref.
//
// Code starts here:

// Offset of the start of the code in PRU memory
.origin 0 

// Program entry point, used by debugger only
.entrypoint START 

// To signal the host that we are done, we set bit 5 in our R31
// simultaneously with putting the number of the signal we want
// into R31 bits 0-3. See 5.2.2.2 
#define PRU0_R31_VEC_VALID (1<<5)
#define SIGNUM 3 // corresponds to PRU_EVTOUT_0

// Address of local memory, see 3.1.2 Table 5 
#define LOCAL_MEM 0x00000000

// Adress of PRU Control Register (see 3.1.2 Table 5) + Offset of
// CTBIR0 Register (0x20h, see 5.4.6)
// This register is used to set the block indices which are used to
//	modify entries
// 24 and 25 in the PRU Constant Table.
// Bits 0-7 => C24_BLK_INDEX
// Bits 16-23 => C25_BLK_INDEX
#define CTBIR0 0x22020

// When everything is setup properply C24 points to local memory
#define CONST_RAM C24
		
#define DELAY_SECONDS 5 // adjust this to experiment
#define CLOCK 200000000 // PRU is always clocked at 200MHz
#define CLOCKS_PER_LOOP 2 // loop contains two instructions, one clock each
#define DELAYCOUNT DELAY_SECONDS * CLOCK / CLOCKS_PER_LOOP

		
START:
		//**********Initialize local memory**********
		// Set local register r0 to local mem adress of pru
		MOV r0, LOCAL_MEM
		// Set local register r1 to CTBIR0
		MOV r1, CTBIR0
		//Set CTBIR0 to address of local mem
		SBBO r0, r1, 0, 4
		// C24 aka CONST_RAM points to local memory
		// We are done

		//Store data in local memory in order to read it from host
		MOV r0, 0xDEAD 
		SBCO r0, CONST_RAM, 4, 4
		SBCO r0, CONST_RAM, 0, 4
		
		// initialize loop counter
		MOV r1, DELAYCOUNT
		//MOV r30, 1<<5
		// wait for specified period of time
DELAY:
		SUB	r1, r1, 1     // decrement loop counter
		QBNE DELAY, r1, 0  // repeat loop unless zero
		//MOV r30, 0
		WBS r31.t16
		// tell host we are done, then halt
		MOV	R31.b0, PRU0_R31_VEC_VALID | SIGNUM
		HALT
