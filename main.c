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

#define ADDRESS 0xA0
#define WRITE 0 //  10100000
#define READ 1 // 10100001

char name[] = "Julia";
uint8_t address = 0x10;

char sentences[] = "Hello World!";
uint8_t address_del3 = 0x50;  //address_del3 % 8 == 0
uint8_t buffer[30];

void main (void) {

	i2c_init();
	uart_init();

	sei();

	// for(int i = 0; i<sizeof(name); i++){
	// 	eeprom_write_byte((address+i),name[i]);
	// }
	// uint8_t data = eeprom_read_byte(address);

	eeprom_write_page(address_del3, sentences);
	uint8_t data = eeprom_read_byte(address_del3);


	while (1) {
		// uart_putstr(data);
		// uart_putstr("\r\n");
		// _delay_ms(5000);

		eeprom_sequential_read(buffer, address_del3, sizeof(sentences));
		uart_putstr(*buffer);
		uart_putstr("\r\n");
		_delay_ms(5000);

	}
}

