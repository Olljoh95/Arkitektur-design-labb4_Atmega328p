#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "serial.h"
#include "timer.h"


void main (void) {

	i2c_init();
	uart_init();

	sei();

	uint8_t manufacurer_code = 0xFA;
	uint8_t start_addr = 0x10;

	


	while (1) {

	}
}

