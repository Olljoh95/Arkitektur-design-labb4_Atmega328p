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

#define EEPROM_WRITE_TO_ADDR 0x10

void main (void) {

	i2c_init();
	uart_init();

	sei();

	//uint8_t manufacturer_code = 0xFA;
	eeprom_write_byte(EEPROM_WRITE_TO_ADDR, 'O');
	eeprom_write_byte(EEPROM_WRITE_TO_ADDR+1, 'l');
	eeprom_write_byte(EEPROM_WRITE_TO_ADDR+2, 'l');
	eeprom_write_byte(EEPROM_WRITE_TO_ADDR+3, 'e');

	uint8_t counter = 0;

	i2c_meaningful_status(i2c_get_status()); //print out status of 

	while (1) {
		_delay_ms(100);

		printf_P(PSTR("%c"),eeprom_read_byte(EEPROM_WRITE_TO_ADDR+counter));
		counter++;
		if(counter == 4) {
			counter = 0;
			printf_P(PSTR("\r\n"));
		}

	}
}



