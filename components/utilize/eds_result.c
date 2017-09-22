#include "eds_result.h"
#include "_sd_api.h"
#include "eds_module_init.h"

/* 
 * deal with once download fw result
 */
ERR_STATUS eds_result_opration(SD_PARAM *param, bool opt_flag){
	DATE_STU Time;
	getCurTime(&Time);
	char strTime[30];
	sprintf(strTime, "%d-%d-%d %d:%d:%d", Time.year, Time.month+1, Time.day, Time.hour, Time.min, Time.sec);
	if(write_to_SDcard(strTime, param->load_param->chip_mac, param->verify_param->bin_version, \
						param->verify_param->becth_no, opt_flag)!=SUCCESS){
		
		printf("record failed\n");
		return 1;
	}

	return SUCCESS;
}


