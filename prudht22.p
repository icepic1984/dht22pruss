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

// Program entry point, used by debugger only
.entrypoint START 

// To signal the host that we are done, we set bit 5 in our R31
// simultaneously with putting the number of the signal we want
// into R31 bits 0-3. See 5.2.2.2 AMPI 
#define PRU0_R31_VEC_VALID (1<<5)
#define SIGNUM 3 // corresponds to PRU_EVTOUT_0

// Address of local memory, see 3.1.2 Table 5 AMPI 
#define LOCAL_MEM 0x00000000

// Adress of PRU Control Register (see 3.1.2 Table 5 AMPI) + Offset of
// CTBIR0 Register (0x20h, see 5.4.6 AMPI)
// This register is used to set the block indices which are used to
// modify entries
// 24 and 25 in the PRU Constant Table.
// Bits 0-7 => C24_BLK_INDEX
// Bits 16-23 => C25_BLK_INDEX
#define CTBIR0 0x22020

// When everything is setup properply C24 points to local memory
#define CONST_RAM C24
		
// Memory Maps for GPIO-Pins, see Section 2.1 Table 2-1 AMTR
#define GPIO0 0x44E07000		
#define GPIO1 0x4804C000
#define GPIO2 0x481AC000
#define GPIO3 0x481AE000

// GPIO Related register and addresses. See 25.4.1 Table 2504.1 Table 25-5 AMTR	
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194
#define GPIO_DIRECTION 0x134
#define GPIO_DIRECTION2 0x142	
#define GPIO_DATAIN 0x138
		
#define DELAY_SECONDS 5 // adjust this to experiment
#define CLOCK 200000000 // PRU is always clocked at 200MHz
#define CLOCKS_PER_LOOP 2 // loop contains two instructions, one clock each
#define DELAYCOUNT DELAY_SECONDS * CLOCK / CLOCKS_PER_LOOP

#define CYCLE_PER_SECOND 200000000 / CLOCKS_PER_LOOP
#define CYCLE_PER_MILLISECOND 200000 / CLOCKS_PER_LOOP
#define	CYCLE_PER_MICROSECOND 200 / CLOCKS_PER_LOOP

// Multiply op1 and op2. Result is stored in R26, R27.
// Multiplication is done using the MAC Unit.
// See 4.5.3.2 AMTR
// ONLY WORKS WITH PASM COMMUNITY BUILD
.macro MUL 
.mparam OP1, OP2
		MOV r25,0
		XOUT 0, R25,1
		MOV R28, OP1 
		MOV R29, OP2 
		XIN 0, R27,4
		XIN 0, R26,4
.endm	
		
.macro DLYS 

// Wait for sec seconds
// We are using only 32 Bits registers
// therefore we can only wait for up to 42 seconds
.mparam sec
		MUL sec, CYCLE_PER_SECOND
WAITS:	SUB r26, r26, 1     
		QBNE WAITS, r26, 0
.endm

// Wait for ms millisecond		
.macro DLYMS
.mparam ms
		MUL ms, CYCLE_PER_MILLISECOND
WAITMS:	SUB r26, r26, 1
		QBNE WAITMS, r26,0
.endm

// Wait for us microseconds
.macro DLYUS
.mparam us
		MUL us, CYCLE_PER_MICROSECOND
		MOV r23,26
WAITUS:	SUB r26, r26, 1
		QBNE WAITUS, r26, 0
.endm
		
		
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

		//Store data in local memory in order to read it from host
		MOV r0, 0xDEAD 
		SBCO r0, CONST_RAM, 4, 4
		SBCO r0, CONST_RAM, 0, 4

		// Set Pin 16 to input and write content of output register
		// to ram (for debugging).
		// 1 => INPUT
		// 0 => OUTPUT
		MOV r4, GPIO1 | GPIO_DIRECTION
		LBBO r5, r4, 0 , 4
		SET r5,16
		SBCO r5, CONST_RAM, 0, 4
		SBBO r5, r4, 0, 4
		SBCO r5, CONST_RAM, 4, 4
	
		// Set GPIO1[16] to high
		//MOV r4, GPIO1 | GPIO_SETDATAOUT
		//MOV r5, 1 <<16
		//SBBO r5, r4,0,4
		
	    // initialize loop counter
		MOV r1, DELAYCOUNT
		// wait for specified period of time
DELAY:
		SUB	r1, r1, 1     // decrement loop counter
	//	QBNE DELAY, r1, 0  // repeat loop unless zero

		MOV r4 , GPIO1 | GPIO_DATAIN
		LBBO r5, r4, 0, 4
		SBCO r5, CONST_RAM, 0 ,4
		
		// Set GPI01[16] to low 
        //MOV r4, GPIO1 | GPIO_CLEARDATAOUT
		//MOV r5, 1<<16
        //SBBO r5, r4, 0 ,4

		SBCO r23, CONST_RAM, 0, 4
		SBCO r27, CONST_RAM, 4, 4

		
		// tell host we are done, then halt
		MOV	R31.b0, PRU0_R31_VEC_VALID | SIGNUM
		HALT
