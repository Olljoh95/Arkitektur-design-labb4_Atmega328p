#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <string.h>

#include "i2c.h"

#define EEPROM_CONTROL_BYTE 0xA0 //0b1010 0000

/*I2C Read/Write flags*/
#define I2C_R	1
#define I2C_W	0

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

	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN); //Transmit STOP condition

	while((TWCR & (1<<TWSTO))); //Wait for STOP condition to be executed

}

inline uint8_t i2c_get_status(void) {
	uint8_t status = TWSR & 0xF8; //Check value of TWI Status register, load into variable
	return status; //return variable
}

inline void i2c_xmit_addr(uint8_t address, uint8_t rw) {
	TWDR = (address & 0xFE) | (rw & 0x1); //Load SLA + R/W into TWI Data Register (0 = write, 1 = read)
	TWCR = (1<<TWINT) | (1<<TWEN); //Clear TWINT flag to start transmission of address
	while(!(TWCR & (1<<TWINT))); //Wait for TWINT Flag to be set.

inline void i2c_xmit_byte(uint8_t data) {
	TWDR = data; //Load data in to TWI Data Register
	TWCR = (1<<TWINT) | (1<<TWEN); //Clear TWINT flag to start transmission of data
	while(!(TWCR & (1<<TWINT))); //Wait for TWINT Flag to be set.
}


//Function not used and therefore commented out

inline uint8_t i2c_read_ACK() {
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA); //Data byte will be received and ACK will be returned
	while(!(TWCR & (1<<TWINT))); //Wait for TWINT Flag to be set.
	return TWDR;		//Return byte in TWI Data Register
}


inline uint8_t i2c_read_NAK() {
	TWCR = (1<<TWINT) | (1<<TWEN); //Data byte will be received and NOT ACK will be returned
	while(!(TWCR & (1<<TWINT)));  //Wait for TWINT Flag to be set.
	return TWDR;				//Return byte in TWI Data Register
}

inline void eeprom_wait_until_write_complete() {
	/*0x18 = SLA+W transmitted, ACK received*/
	while(i2c_get_status() != 0x18) {	//While master is not transmiting
		i2c_start();		//Send start condition
		i2c_xmit_addr(EEPROM_CONTROL_BYTE,I2C_W);	//Send control byte and write-mode bit
	}
}

uint8_t eeprom_read_byte(uint8_t addr) {
	i2c_start();				//Send start condition
	i2c_xmit_addr(EEPROM_CONTROL_BYTE,I2C_W);	//Send control byte and write-mode bit
	i2c_xmit_byte(addr);		//Send address as byte
	i2c_start();				//send Start condition
	i2c_xmit_addr(EEPROM_CONTROL_BYTE, I2C_R);	//Send control byte and read-mode bit
	uint8_t received = i2c_read_NAK();  //Load data into variable
	i2c_stop();					//Send stop condition
	return received;			//return data

}

void eeprom_write_byte(uint8_t addr, uint8_t data) {
	i2c_start();			//send start condition
	i2c_xmit_addr(EEPROM_CONTROL_BYTE, I2C_W);	//Send control byte and write-mode bit
	i2c_xmit_byte(addr);		//send address as byte
	i2c_xmit_byte(data);		//send data as byte
	i2c_stop();					//send stop condition
	eeprom_wait_until_write_complete(); //wait for status change
}
