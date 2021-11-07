#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <string.h>

#include "i2c.h"

#define F_CPU 16000000
#define ADDRESS 0xA0
#define WRITE 0 //  10100000
#define READ 1 // 10100001

void i2c_init(void) {
	TWSR = 0; // Prescaler=1
	TWBR = ((F_CPU/100000)-16)/(2*1);  // =72  100KHZ=100000
	TWCR = 1 << TWEN; // enable I2C
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
	TWCR=(1<<TWINT) | (1<<TWSTA) | (1<<TWEN);  // begin
	while(!(TWCR & (1<<TWINT))); // Wait till complete TWDR byte transmitted
}

inline void i2c_stop() {
	TWCR=(1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}

inline uint8_t i2c_get_status(void) {
	uint8_t status;
    //mask status
    status = TWSR & 0xF8;
    return status;
}

inline void i2c_xmit_addr(uint8_t address, uint8_t rw) {
	TWDR = address | rw;
	TWCR = (1<<TWINT)|(1<<TWEN);

	while(!(TWCR & (1<<TWINT)));
}

inline void i2c_xmit_byte(uint8_t data) {
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
}

inline uint8_t i2c_read_ACK() {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while(!(TWCR & (1<<TWINT)));
	return TWDR; //return the data that was just received from slave
}

inline uint8_t i2c_read_NAK() {
	TWCR = (1<<TWINT)|(1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	return TWDR;
}

inline void eeprom_wait_until_write_complete() {
	while(i2c_get_status() != 0x18){
		i2c_start();
		TWDR = (0xA0 | 0);
		TWCR = (1<<TWINT)|(1<<TWEN);
		while(!(TWCR & (1<<TWINT)));
	}
}

uint8_t eeprom_read_byte(uint8_t addr) {
	uint8_t data = 0;
	i2c_start();
	i2c_xmit_addr(ADDRESS, WRITE);
	i2c_xmit_byte(addr);
	i2c_start();
	i2c_xmit_addr(ADDRESS, READ);
	data = i2c_read_NAK(); 
	i2c_stop();
	return data;
}

void eeprom_write_byte(uint8_t addr, uint8_t data) {
	i2c_start();
	// uint8_t status = i2c_get_status();
	// char buffer_status[20];
	// uart_putstr("status: ");
	// itoa(status,buffer_status,16); 
	// uart_putstr(buffer_status);
	// uart_putstr("\r\n");
	i2c_xmit_addr(ADDRESS, WRITE);
	i2c_xmit_byte(addr);
	i2c_xmit_byte(data);
	i2c_stop();
	eeprom_wait_until_write_complete();
}



void eeprom_write_page(uint8_t addr, uint8_t *data) {
	i2c_start();
	i2c_xmit_addr(ADDRESS, WRITE);	
	i2c_xmit_byte(addr);
	int count = sizeof(data)%8 == 0 ? sizeof(data) : sizeof(data)+1;
	int byte = 8;
	for(int i = 0; i<count; i++){
		for(int j = 0; j<byte; j++){
			i2c_xmit_byte(data[j*i+j*8]);
		} 		
	}
	i2c_stop();
	eeprom_wait_until_write_complete();
}

void eeprom_sequential_read(uint8_t *buffer, uint8_t start_addr, uint8_t len) {
	i2c_start();
	i2c_xmit_addr(ADDRESS, WRITE);	
	i2c_xmit_byte(start_addr);
	i2c_start();
	i2c_xmit_addr(ADDRESS, READ);
	for(int i = 0; i < (len-1); i++){
		buffer[i] = i2c_read_ACK();
	}
	buffer[len-1] = i2c_read_NAK();
	i2c_stop();
}
