/**
 * 
 * Sistemas Basados en Microprocesador
 * Fichero de funcionalidad del LCD
 * 
 * Autores:
 * Nicolas Villanueva
 * Vanesa Marcano
 * 
 * Profesor:
 * Fernando Pescador
 * 
 * Otonho 2019
 * 
 * */
#include "GPIO_LPC17XX.h"
#include "Driver_SPI.h"
#include "Arial12x12.h"
#include "lcd.h"


// CMSIS Driver reference declaration
extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

// Information to display in the screen
unsigned char buffer[512];
// Variables to confirm line writing
bool activeL1 = 0;
bool activeL2 = 0;




/* Funcion de inicializacion del LCD */
void init_lcd() {
	// 1. Inicializaci?n y configuraci?n del driver SPI para gestionar el LCD
	SPIdrv->Initialize(NULL); 				// Initialize the SPI driver
	SPIdrv->PowerControl(ARM_POWER_FULL); 	// Power up the SPI peripheral
	// Configure the SPI to Master, 8-bit mode @20000 kBits/sec
	SPIdrv->Control(
		ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8),
		20000000
	);

	/* 2. Configurar los pines de IO que sean necesarios y programar su valor por defecto */
	GPIO_SetDir(PORT_SPI, PIN_SPI_RESET, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_SPI, PIN_SPI_A0, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_SPI, PIN_SPI_CS, GPIO_DIR_OUTPUT);
	GPIO_PinWrite(PORT_SPI, PIN_SPI_RESET, 1);
	GPIO_PinWrite(PORT_SPI, PIN_SPI_A0, 1); /* Da igual */
	GPIO_PinWrite(PORT_SPI, PIN_SPI_CS, 1);

	/* 3. Generar la senhal de reset  */
	GPIO_PinWrite(PORT_SPI, PIN_SPI_RESET, 0);
	retardo(10000); /* 1 ms */
	GPIO_PinWrite(PORT_SPI, PIN_SPI_RESET, 1);
	retardo(10000); /* 1 ms */
}



/* Funcion de configuracion en el reset del LCD */
void reset_lcd() {
	wr_cmd(0xAE); // Display off
	wr_cmd(0xA2); // Fija el valor de la relaci?n de la tensi?n de polarizaci?n del LCD a 1/9 
	wr_cmd(0xA0); // El direccionamiento de la RAM de datos del display es la normal
	wr_cmd(0xC8); // El scan en las salidas COM es el normal
	wr_cmd(0x22); // Fija la relaci?n de resistencias interna a 2 
	wr_cmd(0x2F); // Power on
	wr_cmd(0x40); // Display empieza en la l?nea 0
	wr_cmd(0xAF); // Display ON
	wr_cmd(0x81); // Contraste
	wr_cmd(0x17); // Valor Contraste
	wr_cmd(0xA4); // Display all points normal
	wr_cmd(0xA6); // LCD Display normal
}



/* Funcion de escritura al LCD */
void copy_to_lcd() {
	int i;
	wr_cmd(0x00);      // 4 bits de la parte baja de la direccion a 0
	wr_cmd(0x10);      // 4 bits de la parte alta de la direccion a 0
	wr_cmd(0xB0);      // P?gina 0
	if (activeL1 == 1) {
		for(i=0; i<128; i++) {
			wr_data(buffer[i]);
		}
    }
	

	wr_cmd(0x00);      // 4 bits de la parte baja de la direccion a 0
	wr_cmd(0x10);      // 4 bits de la parte alta de la direccion a 0
	wr_cmd(0xB1);      // Pagina 1
	if (activeL1 == 1) {
		for(i=128; i<256; i++) {
			wr_data(buffer[i]);
		}
	}

	wr_cmd(0x00);       
	wr_cmd(0x10);      
	wr_cmd(0xB2);      //Pagina 2
	if (activeL2 == 1) {
		for(i=256; i<384; i++) {
			wr_data(buffer[i]);
		}
	}
	
	wr_cmd(0x00);       
	wr_cmd(0x10);       
	wr_cmd(0xB3);      // Pagina 3
	if (activeL2 == 1) {	
		for(i=384; i<512 ;i++){
			wr_data(buffer[i]);
		}
	}
}



// Funcion de retardo: la unidad minima de division es ~10us
void retardo(int ticks) {
	int i;
	for (i = 0; i < US10_TICKS*ticks; i++);
}


// Funcion de escritura de datos (A0 = 1)
void wr_data(unsigned char cmd) {
	uint32_t a0 = 1;
	do_write(cmd, a0);
}

// Funcion de escritura de datos (A0 = 0)
void wr_cmd(unsigned char cmd) {
	uint32_t a0 = 0;
	do_write(cmd, a0);
}


// Funcion generica de escritura
void do_write(unsigned char info, uint32_t a0) {
	GPIO_PinWrite(PORT_SPI, PIN_SPI_CS, 0);
	GPIO_PinWrite(PORT_SPI, PIN_SPI_A0, a0);

	SPIdrv->Send(&info, sizeof(info));

	GPIO_PinWrite(PORT_SPI, PIN_SPI_CS, 1);
}


// Escribir letra en la linea superior (paginas 0 y 1)
uint8_t escribe_letra_L1(uint8_t letter, uint8_t posL1) {
	uint8_t i, valor1, valor2;
	uint16_t comienzo = 0;
	
	activeL1 = 1;
	comienzo = 25* (letter - ' ');
	
	for (i = 0; i < 12; i++) {
		valor1 = Arial12x12[comienzo + i*2 + 1];
		valor2 = Arial12x12[comienzo + i*2 + 2];
		
		if (i + posL1 > 127) {
			break;
		}
		
		buffer[i + posL1] = valor1;
		buffer[i + 128 + posL1] = valor2;
	}
	return posL1 + Arial12x12[comienzo];
}



// Escribir letra en la linea inferior (paginas 2 y 3)
uint8_t escribe_letra_L2(uint8_t letter, uint8_t posL2) {
  uint8_t i, valor1, valor2;
  uint16_t comienzo = 0;

  activeL2 = 1;
  comienzo = 25* (letter - ' ');

  for (i = 0; i < 12; i++) {
    valor1 = Arial12x12[comienzo + i*2 + 1];
    valor2 = Arial12x12[comienzo + i*2 + 2];
    
		if (i + posL2 > 127) {
			break;
		}

    buffer[256 + i + posL2] = valor1;
    buffer[384 + i + posL2] = valor2;
  }
  return posL2 + Arial12x12[comienzo];
}



void escribe_frase_L1(char* frase, int fraseLength) {
	uint8_t posL1 = 0;
	uint8_t letra;
	int ch;
  bool found_endline = false;

  for (ch = 0; ch < fraseLength; ch++) {
		// letra = space_fill(frase[ch]);
		// letra = (frase[ch] == 0x00 || frase[ch] > 0x7F) ? 0x20 : frase[ch];
    if (found_endline) {
      letra = 0x20;
		} else if (frase[ch] > 0x7F) {
			letra = 0x7F;
    } else if (frase[ch] < 0x20) {
      letra = 0x20;
      found_endline = (frase[ch] == 0x00);
    } else {
      letra = frase[ch];
    }
		// letra = frase[ch];
		posL1 = escribe_letra_L1(letra, posL1);
	}
}



void escribe_frase_L2(char* frase, int fraseLength) {
	uint8_t posL2 = 0;
	uint8_t letra;
	int ch;
  bool found_endline = false;

  for (ch = 0; ch < fraseLength; ch++) {
		// letra = space_fill(frase[ch]);
		// letra = (frase[ch] == 0x00 || frase[ch] > 0x7F) ? 0x20 : frase[ch];
    if (found_endline) {
      letra = 0x20;
    } else if (frase[ch] > 0x7F) {
			letra = 0x7F;
    } else if (frase[ch] < 0x20) {
      letra = 0x20;
      found_endline = (frase[ch] == 0x00);
    } else {
      letra = frase[ch];
    }
		// letra = frase[ch];
		posL2 = escribe_letra_L2(letra, posL2);
  }
}



uint8_t space_fill(char letter) {
	if (letter < 0x20) {
		return 0x20;
	} else {
		return letter;
	}
}
