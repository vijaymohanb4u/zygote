//****************************************************************************
//**
//**    pit.cpp
//**		8253 Programmable Interval Timer
//**
//****************************************************************************

//============================================================================
//    IMPLEMENTATION HEADERS
//============================================================================

#include "../../include/hw_init.h"
#include "idt.h"
#include "pit.h"
#include "pic.h"

//============================================================================
//    IMPLEMENTATION PRIVATE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================

//-----------------------------------------------
//	Controller Registers
//-----------------------------------------------

#define		I86_PIT_REG_COUNTER0		0x40
#define		I86_PIT_REG_COUNTER1		0x41
#define		I86_PIT_REG_COUNTER2		0x42
#define		I86_PIT_REG_COMMAND			0x43

//! Global Tick count
static volatile uint32_t _pit_ticks=0;

//! Test if pit is initialized
static uint8_t	_pit_bIsInit=0;

//============================================================================
//    INTERFACE DATA
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTION PROTOTYPES
//============================================================================

//! pit timer interrupt handler
void i86_pit_irq ();

//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTIONS
//============================================================================

//!	pit timer interrupt handler
void i86_pit_irq () {

	asm ("add $28, %esp");
	asm ("pusha");

	//! increment tick count
	_pit_ticks++;
                //GotoXY (0,22);
                //Printf ("Current tick count: %i", get_tick_count());
	//! tell hal we are done
	interruptdone(0);

	asm ("popa");
	asm ("iret");
}

//============================================================================
//    INTERFACE FUNCTIONS
//============================================================================

//! Sets new pit tick count and returns prev. value
uint32_t i86_pit_set_tick_count (uint32_t i) {

	uint32_t ret = _pit_ticks;
	_pit_ticks = i;
	return ret;
}


//! returns current tick count
uint32_t i86_pit_get_tick_count () {

	return _pit_ticks;
}


//! send command to pic
void i86_pit_send_command (uint8_t cmd) {

	outportb (I86_PIT_REG_COMMAND, cmd);
}


//! send data to a counter
void i86_pit_send_data (uint16_t data, uint8_t counter) {

	uint8_t	port= (counter==I86_PIT_OCW_COUNTER_0) ? I86_PIT_REG_COUNTER0 :
		((counter==I86_PIT_OCW_COUNTER_1) ? I86_PIT_REG_COUNTER1 : I86_PIT_REG_COUNTER2);

	outportb (port, (uint8_t)data);
}


//! read data from counter
uint8_t i86_pit_read_data (uint16_t counter) {

	uint8_t	port= (counter==I86_PIT_OCW_COUNTER_0) ? I86_PIT_REG_COUNTER0 :
		((counter==I86_PIT_OCW_COUNTER_1) ? I86_PIT_REG_COUNTER1 : I86_PIT_REG_COUNTER2);

	return inportb (port);
}


//! starts a counter
void i86_pit_start_counter (uint32_t freq, uint8_t counter, uint8_t mode) {

	uint16_t divisor;
	uint8_t ocw;
	if (freq==0)
		return;

	divisor = (uint16_t)(1193181 / (uint16_t)freq);

	//! send operational command
	ocw=0;
	ocw = (ocw & ~I86_PIT_OCW_MASK_MODE) | mode;
	ocw = (ocw & ~I86_PIT_OCW_MASK_RL) | I86_PIT_OCW_RL_DATA;
	ocw = (ocw & ~I86_PIT_OCW_MASK_COUNTER) | counter;
	i86_pit_send_command (ocw);

	//! set frequency rate
	i86_pit_send_data (divisor & 0xff, 0);
	i86_pit_send_data ((divisor >> 8) & 0xff, 0);

	//! reset tick count
	_pit_ticks=0;
}


//! initialize minidriver
void i86_pit_initialize () {

	//! Install our interrupt handler (irq 0 uses interrupt 32)
	setvect (32, i86_pit_irq);

	//! we are initialized
	_pit_bIsInit = 1;
}


//! test if pit interface is initialized
uint8_t i86_pit_is_initialized () {

	return _pit_bIsInit;
}

//============================================================================
//    INTERFACE CLASS BODIES
//============================================================================
//****************************************************************************
//**
//**    END[pit.c]
//**
//****************************************************************************
