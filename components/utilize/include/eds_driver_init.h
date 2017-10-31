#ifndef __EDS_DRIVER_INIT_H__
#define __EDS_DRIVER_INIT_H__
#include "eds_config.h"

enum BOARD_TYPE{
	BOT_WROVERKIT = 0,
	BOT_8089AGING,
};

ERR_STATUS eds_driver_init(enum BOARD_TYPE board_type);


#endif //__EDS_DRIVER_INIT_H__


