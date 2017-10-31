#ifndef __EDS_FW_DOWNLOAD_H__
#define __EDS_FW_DOWNLOAD_H__
#include "eds_config.h"

ERR_STATUS eds_load_ram(void);
ERR_STATUS eds_connect(void);
ERR_STATUS eds_loadParam_init(void);
ERR_STATUS eds_firmware_download(void);
ERR_STATUS eds_module_test(char* mdl_logs);
ERR_STATUS eds_8089_aging(uint32_t flash_begin);

#endif //__EDS_FW_DOWNLOAD_H__
