#ifndef __SD_API_H__
#define __SD_API_H__
#include "eds_config.h"
#include "_download_api.h"

ERR_STATUS getParam_fromSD(SD_PARAM *param);
ERR_STATUS get_Image(char *image_path, IMAGE *image, uint8_t chip_type);
ERR_STATUS get_SegmentData(char *fdPath, uint8_t *pdata, uint32_t offset, uint32_t size);
int getFileSize(char *fdPath);
ERR_STATUS write_to_SDcard(char *time, char *mac, char *bin_ver, char *becth_no, bool res);

#endif //__SD_API_H__
