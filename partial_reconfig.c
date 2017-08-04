#include "xparameters.h"
#include "xbram.h"
#include <stdio.h>
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include <xparameters.h>
#include <xil_io.h>
#include <xstatus.h>
#include <stdlib.h>
#include "xhwicap_i.h"
#include "xhwicap.h"
#include "xuartlite_l.h"
#include "xuartlite_i.h"

/*
 *The following constants define BRAM ID and buffer size
 */
#define BRAM_DEVICE_ID XPAR_BRAM_0_DEVICE_ID
#define RECIEVE_BUFFER_SIZE 32

/*
 * The following constants define the commands which may be sent to the EEPROM
 * device that are used for this sw application.
 */
#define WRITE_STATUS_CMD	1
#define WRITE_CMD				2
#define READ_CMD				3
#define WRITE_DISABLE_CMD	4
#define READ_STATUS_CMD		5
#define WRITE_ENABLE_CMD	6
#define BULK_ERASE_CMD		0xC7
#define SECTOR_ERASE_CMD	0xD8

/*
 * The following constants define the offsets within a EepromBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the SPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 		/* EEPROM instruction */
#define ADDRESS_MSB_OFFSET	1 		/* MSB of address to read or write */
#define ADDRESS_LSB_OFFSET	2 		/* LSB of address to read or write */
#define DATA_OFFSET			4
#define WRITE_DATA_OFFSET	4  	/* Start of data to write to the EEPROM */
#define READ_DATA_OFFSET	6  	/* Start of data read from the EEPROM */


/*
 *COMMANDS FOR ICAP CONTROL
 */
#define ICAP_SELECT 1
#define ICAP_WRITE  2



XBram Bram; /*the object for bram device*/
static XHwIcap  HwIcap_SLR0;	/* The instance of the HWICAP device */
static XHwIcap  *curIcap;		/* ICAP currently being used */
static XUartLite Uart; /* The instance of the UART device */

int initIp(XHwIcap_Config *ConfigPtr0, XUartLite_Config *UARTConfig, XBram_Config *Bram_ConfigPtr){

	int Status;

		//initialize bram comfig
		Bram_ConfigPtr = XBram_LookupConfig(BRAM_DEVICE_ID);
		if (Bram_ConfigPtr == (XBram_Config *) NULL) {
			return XST_FAILURE;
		}

		Status = XBram_CfgInitialize(&Bram, Bram_ConfigPtr,Bram_ConfigPtr->CtrlBaseAddress);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	    //Get Uart cfg
		UARTConfig = XUartLite_LookupConfig(XPAR_UARTLITE_0_DEVICE_ID);
		if (UARTConfig == NULL) {
			return XST_FAILURE;
		}
		//Initialize UART with config and check for success
		Status = XUartLite_CfgInitialize(&Uart, UARTConfig,UARTConfig->RegBaseAddr);
		if (Status != XST_SUCCESS) {
			return 2;
		}

	   //Get ICAP cfg
		ConfigPtr0 = XHwIcap_LookupConfig(XPAR_AXI_HWICAP_0_DEVICE_ID);

		if (ConfigPtr0 == NULL) {
			return XST_FAILURE;
		}
		Status = XHwIcap_CfgInitialize(&HwIcap_SLR0, ConfigPtr0,ConfigPtr0->BaseAddress);
		if (Status != XST_SUCCESS) {
			return 3;
		}
		return XST_SUCCESS;
}

int recPartial(XHwIcap *icapTarget,u32 numBytes,XBram_Config *Bram_ConfigPtr){

	u32 i=0, word;
	int Status;
	u8 byte0, byte1,byte2,byte3; //Bytes to hold received bitstream
	//Loop until end of bitstream

	volatile u32 *LocalAddr = (volatile u32 *)(Bram_ConfigPtr->MemBaseAddress);
	*LocalAddr = 1;

	u32 bram_offset=Bram_ConfigPtr->MemBaseAddress;
	u32 loop=0;

while(i<(numBytes/4)) {
		//Here microblaze receives 4 bytes from UART terminal and combines one word by shifting

		byte0 = XUartLite_RecvByte(XPAR_UARTLITE_0_BASEADDR ); //MSB
		byte1  = XUartLite_RecvByte(XPAR_UARTLITE_0_BASEADDR );
		byte2  = XUartLite_RecvByte(XPAR_UARTLITE_0_BASEADDR );
		byte3 = XUartLite_RecvByte(XPAR_UARTLITE_0_BASEADDR ); //LSB

		word = ((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | (byte3)); //Bit shift bytes to reassemble 32 bit word

		//Here stores words in BRAM and sent words to FPGA through ICAP
		u32 newword=*(volatile u32 *)(Bram_ConfigPtr->MemBaseAddress);//newword is the flag stored in BRAM baseaddress
																	  //if newword is 0, load 20 words into ICAP, else, continue load one word into BRAM
		if(newword){
		volatile u32 *LocalAddr = (volatile u32 *)(bram_offset+4);
		*LocalAddr = word;
		bram_offset+=4;
		loop++;

		if(loop==20){//if 20 words are stored in BRAM, then sent to ICAP
			volatile u32 *LocalAddr = (volatile u32 *)(Bram_ConfigPtr->MemBaseAddress);
			*LocalAddr = 0; //raise flag to be ready to load wirds to ICAP
		}

		}

		newword=*(volatile u32 *)(Bram_ConfigPtr->MemBaseAddress);

		//sent 20 words to ICAP
		if(!newword){
			loop=0;
			bram_offset=Bram_ConfigPtr->MemBaseAddress;
			for(u32 times=0;times<20;times++){
				u32 newwordd=*(volatile u32 *)(bram_offset+4+4*times);
				Status = XHwIcap_DeviceWrite(icapTarget, &newwordd, 1);
					if (Status != XST_SUCCESS)
						{
						//Error writing to ICAP
							return XST_FAILURE;
						}
			}
			volatile u32 *LocalAddr = (volatile u32 *)(Bram_ConfigPtr->MemBaseAddress);
			*LocalAddr = 1;//pull down flag to continue load word into BRAM
		}

		i++;
}

//here is check corner case, load remain words with less than 20 in BRAM to ICAP
if(loop!=0){
	bram_offset=Bram_ConfigPtr->MemBaseAddress;
	for(u32 times=0;times<loop;times++){
		u32 newworddd=*(volatile u32 *)(bram_offset+4+4*times);
		Status = XHwIcap_DeviceWrite(icapTarget, &newworddd, 1);
			if (Status != XST_SUCCESS)
				{
			//Error writing to ICAP
				return XST_FAILURE;
				}
	}

}
	xil_printf("\r\n\r\nPartial reconfiguration success\r\n\r\n");
	return XST_SUCCESS;
}

u32 readIdCode(XHwIcap *icapTarget){
	int Status;
	u32 dev_id;

	Status = XHwIcap_GetConfigReg(icapTarget, XHI_IDCODE, &dev_id);
	if (Status != XST_SUCCESS) {
		icapTarget->IsReady = 0;
		return XST_FAILURE;
	}

	return dev_id;
}

void menu(){

	xil_printf("\r\n          PARTIAL RECONFIGURATION OPTIONS MENU\r\n");
	xil_printf("    1: Set bitstream size\n\r");
	xil_printf("    2: Send bitstream\n\r");
	xil_printf("    3: Read device id from currently selected ICAP\n\r");
	xil_printf("    4: Print BRAM memory \n\r");
	xil_printf("    5: Exit \n\r");
	xil_printf("    Enter: Refresh menu and do nothing\n\r");
	print("\n\r> ");
}

int main()
{

	XHwIcap_Config *ConfigPtr0;		//Config pointer for ICAP
	XUartLite_Config *UARTConfig;   //Config pointer for UART
	XBram_Config *Bram_ConfigPtr;   //Config pointer for Bram
	curIcap = &HwIcap_SLR0;
	char size [10];
	int Status;
	u32 partialSize; //size in bytes of partial to be received
	int charCount =0; //Count of characters in bitstream size
	u8 Exit=0,operation =0;
	char key =0;			//hold trigger byte
	char READY_BYTE = 'y';	//char to send after getting trigger byte from PC
	u32 dev_id;  //device id code and word to be written to ICAP

	u32 i = 0;

	xil_printf("\r\nInitializing...\n");
	while(i<10000000){
		i++;
	}

	Status = initIp(ConfigPtr0, UARTConfig,Bram_ConfigPtr);

	if (Status != XST_SUCCESS) {
		xil_printf("\r\nInitialization failure. Returned error code %d.",Status);
		return XST_FAILURE;
	}
	xil_printf("\r\nInitialization complete");


	u32 Addr = Bram_ConfigPtr->MemBaseAddress;
	while(Exit != 1) {
		menu();
		operation = XUartLite_RecvByte(XPAR_UARTLITE_0_BASEADDR);

		switch (operation) {
			case '1':
				xil_printf("\r\n\r\nPlease enter size of bitstream in bytes and then hit return: ");
				charCount =0;
				key ='0';
				while(key != '\r'){
					key = XUartLite_RecvByte(XPAR_UARTLITE_0_BASEADDR);
					if(key != '\r'){
						size[charCount] = key;
					}
					xil_printf("%c",key);
					charCount++;
				}
				xil_printf("\n");
				partialSize = atoi(size);
				xil_printf("\r\n Partial bitstream size set as %d Bytes \n\r\n\r",partialSize);
				break;
			case '2':
				if(partialSize > 1){
					xil_printf("\r\n\r\nSend partial bitstream file in binary format '*.bin' :");
					xil_printf("\r\nWARNING: Do not type any more characters in the terminal until transfer is complete. These will be sent to ICAP and may cause FPGA damage.");
					xil_printf("\r\nAfter reconfigration finished, hit Enter to refresh menu. \r\n\r\n");
					Status = recPartial(curIcap,partialSize,Bram_ConfigPtr);
					if (Status != XST_SUCCESS) {
						xil_printf("\r\nRecongifuration failure. Returned error code %d.",Status);
						return XST_FAILURE;
					}
					partialSize=0;
				} else {
					xil_printf("\r\n######### WARNING: PLEASE ENTER A BITSTREAM SIZE BEFORE ATTEMPTING TO SEND A PARTIAL #########\r\n\r\n");
				}
				break;
			case '3':
				dev_id = readIdCode(curIcap);
				xil_printf("\r\n\nDevice0 IDcode is %lx \r\n", dev_id);
				break;
			case '4':
				for (Addr = Bram_ConfigPtr->MemBaseAddress; Addr < Bram_ConfigPtr->MemBaseAddress+4*22; Addr+=0x4) {
				xil_printf("addr %08x value %08x \n\r", Addr, XBram_In32(Addr));}
				break;
			case '5':
				Exit = 1;
				xil_printf("\n\r Exiting application!\n\r Send ready byte 'y' to PC signaling it to send first byte of bitstream.\n\r");
				break;
		}
	}

		//Send ready byte to PC signaling it to send first byte of bitstream.
		XUartLite_SendByte(XPAR_UARTLITE_0_BASEADDR, READY_BYTE);

    return 0;
}