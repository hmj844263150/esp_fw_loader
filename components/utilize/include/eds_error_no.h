#ifndef __EDS_ERROR_NO_H__
#define __EDS_ERROR_NO_H__

/*
 * error code 
 * A-BB-CCC 
 * A	: error grade
 * BB	: error module
 * CCC	: error type
 */

typedef enum{
	SUCCESS = 0x0,
	ERR_DRIVER = 0x01,
	ERR_MODULE = 0x02,
	ERR_UART_CONN = 0x03,
	ERR_FW_DOWNLOAD = 0x04,
	ERR_MODULE_TEST,
	ERR_SD_READ ,
	ERR_SD_WRITE ,
	ERR_PARAM ,
	ERR_OTHER 
}ERR_STATUS;

enum _ERR_FW_DOWNLOAD{
	ERR_FD_CONN = 0x01,
	
};


#endif __EDS_ERROR_NO_H__