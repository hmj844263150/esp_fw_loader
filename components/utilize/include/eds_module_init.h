#ifndef __EDS_MODULE_INIT_H__
#define __EDS_MODULE_INIT_H__
#include "eds_config.h"

enum _Month{
	Jan=0,
	Feb,
	Mar,
	Apr,
	May,
	Jun,
	Jul,
	Aug,
	Sep,
	Oct,
	Nov,
	Dec
};


typedef struct _DATE{
	uint8_t	month;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint32_t year;
	uint32_t day;
	uint32_t rtc_cali_val;
	uint64_t initRTC_Count;
	
}DATE_STU;


ERR_STATUS eds_module_init(void);
bool esd_mi_chip_check();
void getCurTime(DATE_STU *Time);
void sntp_sync();


#endif //__EDS_MODULE_INIT_H__

