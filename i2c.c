#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <string.h>

#include "i2c.h"

void i2c_init(void) {
	/*Calculate a bit rate of 100kHz*/
	
	TWBR = 0x12; //Set bit rate divisiondivision value
	TWSR = 0x4; //Set bit rate prescale 
	/*16+2[TWBR] * [TWSR] = 160 */

	/*16 000 000 / 160 = 100 000 = 100kHz*/
	
	TWCR |= (1<<TWEN); //Set the third bit of the TWI Control Register  
}

void i2c_meaningful_status(uint8_t status) {
	switch (status) {
		case 0x08: // START transmitted, proceed to load SLA+W/R
			printf_P(PSTR("START\n"));
			break;
		case 0x10: // repeated START transmitted, proceed to load SLA+W/R
			printf_P(PSTR("RESTART\n"));
			break;
		case 0x38: // NAK or DATA ARBITRATION LOST
			printf_P(PSTR("NOARB/NAK\n"));
			break;
		// MASTER TRANSMIT
		case 0x18: // SLA+W transmitted, ACK received
			printf_P(PSTR("MT SLA+W, ACK\n"));
			break;
		case 0x20: // SLA+W transmitted, NAK received
			printf_P(PSTR("MT SLA+W, NAK\n"));
				break;
		case 0x28: // DATA transmitted, ACK received
			printf_P(PSTR("MT DATA+W, ACK\n"));
			break;
		case 0x30: // DATA transmitted, NAK received
			printf_P(PSTR("MT DATA+W, NAK\n"));
			break;
		// MASTER RECEIVE
		case 0x40: // SLA+R transmitted, ACK received
			printf_P(PSTR("MR SLA+R, ACK\n"));
			break;
		case 0x48: // SLA+R transmitted, NAK received
			printf_P(PSTR("MR SLA+R, NAK\n"));
			break;
		case 0x50: // DATA received, ACK sent
			printf_P(PSTR("MR DATA+R, ACK\n"));
			break;
		case 0x58: // DATA received, NAK sent
			printf_P(PSTR("MR DATA+R, NAK\n"));
			break;
		default:
			printf_P(PSTR("N/A %02X\n"), status);
			break;
	}
}

inline void i2c_start() {
	/* 
	The TWCR is used to control the operation of the TWI. 
	It is used to enable the TWI, to initiate a Master access 
	by applying a START condition to the bus, to generate a Receiver acknowledge,
	to generate a stop condition, 
	and to control halting of the bus while the data to be written to the bus are written to the TWDR.
	It also indicates a write collision if data is attempted written to TWDR while the register is inaccessible

	TWINT = 8th bit, TWI Interrupt flag. By setting it to 1 we clear it.

	TWSTA = 6th bit, TWI Start Condition. The application writes the 
	TWSTA bit to one when it desires to become a Master on the 2-wire Serial Bus.

	TWEN = Third bit, TWI Enable Bit. The TWEN bit enables TWI operation and activates the TWI interface.
	*/
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); //Send START condition

	/*Wait for TWINT Flag to be set.
	 This indicates that the START condition has been transmitted*/
	while(!(TWCR & (1<<TWINT))); 
}

inline void i2c_stop() {
	/*Writing the TWSTO bit to one in Master mode will generate a STOP
	condition on the 2-wire Serial Bus.
	When the STOP condition is executed on the bus, the TWSTO bit is cleared automatically.
	In Slave mode, setting the TWSTO bit can be used to recover from an error condition. */

	TWCR = (1<<TWINT)|(1<<TWEN) | (1<<TWSTO); //Transmit STOP condition

	while(TWCR & (1<<TWSTO)); //Wait for STOP condition to be executed

}

inline uint8_t i2c_get_status(void) {
	return (TWSR & 0xF8); //Check value of TWI Status register
}

inline void i2c_xmit_addr(uint8_t address, uint8_t rw) {
	TWDR = (address & 0xFE) | (rw & 0x1); //Load SLA + R/W into TWI Data Register (0 = write, 1 = read)
	TWCR = (1<<TWINT) | (1<<TWEN); //Clear TWINT flag,
	while(!(TWCR & (1<<TWINT)));
}

inline void i2c_xmit_byte(uint8_t data) {
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
}

inline uint8_t i2c_read_ACK() {
	// ...
}

inline uint8_t i2c_read_NAK() {
	// ...
}

inline void eeprom_wait_until_write_complete() {
	// ...
}

uint8_t eeprom_read_byte(uint8_t addr) {
	// ...
}

void eeprom_write_byte(uint8_t addr, uint8_t data) {
	// ...
}



void eeprom_write_page(uint8_t addr, uint8_t *data) {
	// ... (VG)
}

void eeprom_sequential_read(uint8_t *buf, uint8_t start_addr, uint8_t len) {
	// ... (VG)
}
