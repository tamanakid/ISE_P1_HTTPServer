#include "lpc17xx.h"
#include "lpc17xx_iap.h"
#include "lpc17xx_libcfg.h"

#include "HTTP_Server.h"



extern uint8_t net_mac_address	[6];
extern uint8_t net_ip_address		[4];

extern uint8_t leds_on;
extern bool leds_running;


uint32_t result[4];


/**
 * @brief		Thread: Flash Interfacing - Retrieves arguments & writes to flash
 */
void thread_flash (void const *argument) {
	int sector_number;
	uint8_t i, flash_buffer[12];
	
	while (1) {		
		/* Erase whole sector before overwriting */
		sector_number = GetSecNum(FLASH_SECTOR_11_START);
		flash_erase_sector(sector_number, sector_number);
		
		/* IP Address */
		for (i = 0; i < sizeof(net_mac_address); i++) {
			flash_buffer[i] = net_mac_address[i];
		}
		
		/* MAC Address */
		for (i = 0; i < sizeof(net_ip_address); i++) {
			flash_buffer[i + 6] = net_ip_address[i];
		}
		
		/* LEDs Status */
		flash_buffer[0x0A] = leds_running == false ? 0x10 : 0x00;
		flash_buffer[0x0A] = flash_buffer[0x0A] | leds_on;
		
		/* Write array into flash */
		flash_write_array(FLASH_SECTOR_11_START, flash_buffer, FLASH_WRITE_SIZE);
		// flash_overwrite_array (FLASH_SECTOR_11_START, flash_buffer);

		osSignalWait(0x01, osWaitForever);
	}
}



/**
 * @brief				Gets the flash sector range that matches the address range specified
 * @param[in]		FlashAddressRange *address_range - The address range
 * @param[out]	FlashSectorRange *sector_range - The resulting sector range
 */
void flash_get_sector_range (FlashAddressRange *address_range, FlashSectorRange *sector_range) {
	sector_range->start = GetSecNum(address_range->start);
	sector_range->end = GetSecNum(address_range->end);
}



/**
 * @brief				Clears all the bytes (to 0xFF) in the specified sector range
 * @param[in]		int sector_start - First sector of range
 * @param[in]		int sector_end - Last sector of range
 */
void flash_erase_sector (int sector_start, int sector_end) {	
	IAP_STATUS_CODE status = EraseSector(sector_start, sector_end); 
  if (status != CMD_SUCCESS) {
		while(1);
  }
	
	/* Check that sector is indeed blank */
	status = BlankCheckSector(sector_start, sector_end, &result[0], &result[1]);
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
 * @brief				Reads flash's content at given address
 * @param[in]		uint32_t address - Address to start read at
 * @param[in]		uint8_t *dest_array - Destination (array) to write read values into
 * @param[in]		int size - size of dest array.
 */
void flash_read_array (uint32_t address_start, uint8_t *dest_array, int size) {
	int i;
	uint8_t *ptr_address;
	
	/* Read directly from memory address */
	for (i = 0; i < size; i++) {
		ptr_address = (uint8_t*)(address_start + i);
		dest_array[i] = *ptr_address;
	}
}



/**
 * @brief				Writes given array into flash's specified address (OVERWRITES REST OF DATA)
 * @param[in]		uint32_t address_start - The address to start writing to
 * @param[in]		uint8_t *array - Array to write into flash
 */
void flash_overwrite_array (uint32_t address_start, uint8_t *src_array) {
	IAP_STATUS_CODE status;
	uint8_t *ptr_dest = (uint8_t*)(address_start);
	
	/* Minimum write value is 256B */
	status =  CopyRAM2Flash(ptr_dest, src_array, IAP_WRITE_256); /* Includes PrepareSector call */
	if (status != CMD_SUCCESS) {
		while(1);
	}
}



/**
 * @brief				Writes single byte to the flash without overwriting rest of subsector
 * @param[in]		uint32_t byte_address - The address to write the byte into
 * @param[in]		uint8_t value - The value to be written into the byte address
 */
void flash_write_byte (uint32_t address, uint8_t value) {
	int i;
	uint8_t sector_number;
	uint8_t *ptr_address;
	IAP_STATUS_CODE status;
	uint8_t write_array[IAP_WRITE_256] = { 0 };
	
	/* Get the 256B "sub-sector" start address where byte_address belongs to */
	uint32_t subsector_start_address = address & 0xFFFFFF00;
	
	/** Read entire sector */
	for (i = 0; i < sizeof(write_array); i++) {
		ptr_address = (uint8_t*)(subsector_start_address + i);
		if ((uint32_t)ptr_address == address) {
			write_array[i] = value;
		} else {
			write_array[i] = *ptr_address;
		}
	}
	
	/* Erase whole sector before writing */
	sector_number = GetSecNum(address);
	status = EraseSector(sector_number, sector_number);
	status = BlankCheckSector(sector_number , sector_number , &result[0], &result[1]);
	
	/* Write to the flash with the 256 bytes from the write array */
	ptr_address = (uint8_t*)(subsector_start_address);
	status =  CopyRAM2Flash(ptr_address, write_array, IAP_WRITE_256);
	if (status != CMD_SUCCESS) {
		while(1);
	}
	status = Compare(ptr_address, write_array, IAP_WRITE_256);
}



/**
 * USE WITH CAUTION! Make sure all the values are to be written to the same sub-sector.
 * @brief				Writes an array of bytes to the flash adjacently
 * @param[in]		uint32_t start_write_address - The address where the array's first element is to be written
 * @param[in]		uint8_t values[] - The array of bytes to be written
 * @param[in]		uint8_t size - The amount of bytes to be written from the array
 */
void flash_write_array (uint32_t start_write_address, uint8_t src_array[], uint8_t size) {
	int i, overwrite_count;
	uint8_t sector_number;
	uint8_t *ptr_address;
	IAP_STATUS_CODE status;
	uint8_t write_array[IAP_WRITE_256] = { 0 };
	
	/* Get the 256B "sub-sector" start address where the stream's address belongs to */
	uint32_t subsector_start_address = start_write_address & 0xFFFFFF00;
	
	/** Read entire sector */
	overwrite_count = 0;
	for (i = 0; i < sizeof(write_array); i++) {
		ptr_address = (uint8_t*)(subsector_start_address + i);
		if ((uint32_t)ptr_address >= start_write_address && (uint32_t)ptr_address < start_write_address + size) {
			write_array[i] = src_array[overwrite_count];
			overwrite_count++;
		} else {
			write_array[i] = *ptr_address;
		}
	}
	
	/* Erase whole sector before writing */
	sector_number = GetSecNum(start_write_address);
	status = EraseSector(sector_number, sector_number);
	status = BlankCheckSector(sector_number , sector_number , &result[0], &result[1]);
	
	/* Write to the flash with the 256 bytes from the write array */
	ptr_address = (uint8_t*)(subsector_start_address);
	status =  CopyRAM2Flash(ptr_address, write_array, IAP_WRITE_256);
	if (status != CMD_SUCCESS) {
		while(1);
	}
	status = Compare(ptr_address, write_array, IAP_WRITE_256);
}
