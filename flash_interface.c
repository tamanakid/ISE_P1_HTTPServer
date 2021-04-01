#include "lpc17xx.h"
#include "lpc17xx_iap.h"
#include "lpc17xx_libcfg.h"

#include "HTTP_Server.h"


/** The area will be erase and program */
#define FLASH_PROG_SECTOR_START       0x8000	/* Starting position for sector 8 */
#define FLASH_PROG_SECTOR_SIZE				0x1000	/* This accounts for 4kB: The entire sector 8 */
#define FLASH_SECTOR_8								0x8


/** The origin buffer on RAM */
#define BUFF_SIZE           				0x400	/* (1024) This accounts for 1kB: a fourth of the sector */



extern uint8_t net_mac_address	[6];
extern uint8_t net_ip_address		[4];

extern uint8_t leds_on;
extern bool leds_running;


uint32_t result[4];



void thread_flash (void const *argument) {
	int size = 10;
	bool initialized = false;
	uint8_t i, flash_buffer[12];
	int sector_number;
	IAP_STATUS_CODE status;
	
	while (1) {		
		/* Erase whole sector before overwriting */
		if (initialized == true) {
			sector_number = GetSecNum(0x00018000);
			flash_erase_sector(sector_number, sector_number);
		}
		
		/* IP Address */
		for (i = 0; i < sizeof(net_mac_address); i++) {
			flash_buffer[i] = net_mac_address[i];
		}
		
		/* MAC Address */
		for (i = 0; i < sizeof(net_ip_address); i++) {
			flash_buffer[i + 6] = net_ip_address[i];
		}
		
		/* LEDs Status */
		if (initialized == true) {
			flash_buffer[0x0A] = leds_running == false ? 0x10 : 0x00;
			flash_buffer[0x0A] = flash_buffer[0x0A] | leds_on;
		}
		
		// flash_write_array(0x00018000, flash_buffer);
		flash_write_bytes_tuple(0x00018000, flash_buffer, size);
		
		initialized = true;
		size = 11;
		osSignalWait(0x01, osWaitForever);
	}
}







/**
* @brief				Writes corresponding values into flash
 */
void flash_write_data () {
	uint8_t i, flash_buffer[12];
	int sector_number;
	IAP_STATUS_CODE status;
	
	/* Erase whole sector before overwriting */
	sector_number = GetSecNum(0x00007000);
	status = EraseSector(sector_number, sector_number);
	status = BlankCheckSector(sector_number , sector_number , &result[0], &result[1]);
	
	/* IP Address */
	for (i = 0; i < sizeof(net_mac_address); i++) {
		flash_buffer[i] = net_mac_address[i];
	}
	
	/* MAC Address */
	for (i = 0; i < sizeof(net_ip_address); i++) {
		flash_buffer[i + 6] = net_ip_address[i];
	}
	
	flash_buffer[0x0A] = leds_running == true ? 0x01 : 0x00;
	flash_buffer[0x0B] = leds_on;
	
	flash_write_array(0x00007000, flash_buffer);
	// flash_write_bytes_tuple(0x00004000, mac_ip_addresses, sizeof(mac_ip_addresses));
}



/**
 * @brief				Gets the flash sector range that matches the address range specified
 * @param[in]		FlashAddressRange *address_range - The address range (as input)
 * @param[in]		FlashSectorRange *sector_range - The resulting sector range (as output)
 */
void flash_get_sector_range (FlashAddressRange *address_range, FlashSectorRange *sector_range) {
	sector_range->start = GetSecNum(address_range->start);
	sector_range->end = GetSecNum(address_range->end);
}



/**
 * @brief				Clears all the bytes (to 0xFF) in the specified sector range
 * @param[in]		FlashSectorRange *sector_range - The sector range (as input)
 */
void flash_erase_sector (int start, int end) {	
	IAP_STATUS_CODE status = EraseSector(start, end); 
  if (status != CMD_SUCCESS) {
		while(1);
  }
	
	/* Check that sector is indeed blank */
	status = BlankCheckSector(start, end, &result[0], &result[1]);
	switch (status) {
		case CMD_SUCCESS:
			return;
		case SECTOR_NOT_BLANK:
			// uint32_t not_blank_offset = result[0];
			// uint32_t not_blank_contents = result[1];
			break;
		default:
			break;
	}
}



/**
 * @brief				Writes the given array into the flash's specified address (OVERWRITES REST OF DATA)
 * @param[in]		uint32_t address_start - The address to start writing to
 */
void flash_write_array (uint32_t address_start, uint8_t *array) {
	// int sector_number;
	IAP_STATUS_CODE status;
	uint8_t *ptr_dest = (uint8_t*)(address_start);
	
	/* Minimum write value is 256B */
	/* CopyRAM2Flash includes PrepareSector call */
	status =  CopyRAM2Flash(ptr_dest, array, IAP_WRITE_256);
	if (status != CMD_SUCCESS) {
		while(1);
	}
}




/**
 * @brief				Reads the flash's content at a given address
 * @param[in]		uint32_t address - Address to start read from
 * @param[in]		uint8_t *dest - Destination (array) to write read values into
 * @param[in]		int size - size of dest array.
 */
void flash_read_array (uint32_t address, uint8_t *dest, int size) {
	int i;
	uint8_t *ptr_address;
	
	/* Read directly from memory address */
	for (i = 0; i < size; i++) {
		ptr_address = (uint8_t*)(address + i);
		dest[i] = *ptr_address;
	}
}



/**
 * @brief				Writes a single byte to the flash
 * @param[in]		uint32_t byte_address - The address to write the byte into
 * @param[in]		uint8_t value - The value to be written into the byte address
 */
void flash_write_byte (uint32_t byte_address, uint8_t value) {
	int i;
	uint8_t sector_number;
	uint8_t *ptr_address;
	IAP_STATUS_CODE status;
	uint8_t overwrite_array[IAP_WRITE_256] = { 0 };
	
	/* Get the 256B "sub-sector" start address where byte_address belongs to */
	uint32_t subsector_start_address = byte_address & 0xFFFFFF00;
	
	/** Read entire sector */
	for (i = 0; i < sizeof(overwrite_array); i++) {
		ptr_address = (uint8_t*)(subsector_start_address + i);
		if ((uint32_t)ptr_address == byte_address) {
			overwrite_array[i] = value;
		} else {
			overwrite_array[i] = *ptr_address;
		}
	}
	
	/* Erase whole sector before overwriting */
	sector_number = GetSecNum(byte_address);
	status = EraseSector(sector_number, sector_number);
	status = BlankCheckSector(sector_number , sector_number , &result[0], &result[1]);
	
	/* Write to the flash with the 256 bytes from the overwrite array */
	ptr_address = (uint8_t*)(subsector_start_address);
	status =  CopyRAM2Flash(ptr_address, overwrite_array, IAP_WRITE_256);
	if (status != CMD_SUCCESS) {
		while(1);
	}
	status = Compare(ptr_address, overwrite_array, IAP_WRITE_256);
}



/**
 * USE WITH CAUTION! Make sure all the values are to be written to the same sub-sector.
 * @brief				Writes an array of bytes to the flash adjacently
 * @param[in]		uint32_t start_write_address - The address where the array's first element is to be written
 * @param[in]		uint8_t values[] - The array of bytes to be written
 * @param[in]		uint8_t size - The amount of bytes to be written from the array
 */
void flash_write_bytes_tuple (uint32_t start_write_address, uint8_t values[], uint8_t size) {
	int i, overwrite_count;
	uint8_t sector_number;
	uint8_t *ptr_address;
	IAP_STATUS_CODE status;
	uint8_t overwrite_array[IAP_WRITE_256] = { 0 };
	
	/* Get the 256B "sub-sector" start address where the stream's address belongs to */
	uint32_t subsector_start_address = start_write_address & 0xFFFFFF00;
	
	/** Read entire sector */
	overwrite_count = 0;
	for (i = 0; i < sizeof(overwrite_array); i++) {
		ptr_address = (uint8_t*)(subsector_start_address + i);
		if ((uint32_t)ptr_address >= start_write_address && (uint32_t)ptr_address < start_write_address + size) {
			overwrite_array[i] = values[overwrite_count];
			overwrite_count++;
		} else {
			overwrite_array[i] = *ptr_address;
		}
	}
	
	/* Erase whole sector before overwriting */
	sector_number = GetSecNum(start_write_address);
	status = EraseSector(sector_number, sector_number);
	status = BlankCheckSector(sector_number , sector_number , &result[0], &result[1]);
	
	/* Write to the flash with the 256 bytes from the overwrite array */
	ptr_address = (uint8_t*)(subsector_start_address);
	status =  CopyRAM2Flash(ptr_address, overwrite_array, IAP_WRITE_256);
	if (status != CMD_SUCCESS) {
		while(1);
	}
	status = Compare(ptr_address, overwrite_array, IAP_WRITE_256);
}
