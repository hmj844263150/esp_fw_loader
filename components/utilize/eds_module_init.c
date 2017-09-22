#include "eds_module_init.h"
#include "freertos/semphr.h"
#include "soc/soc.h"
#include "soc/rtc.h"

#include "apps/sntp/sntp.h"
#include <time.h>
#include <sys/time.h>


const char* ccpMonth[12]={"Jan",	"Feb", "Mar", "Apr", "May", "Jun",\
					 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const uint8_t cuMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};


DATE_STU DateTime = {
	.year=1970,
	.month=Jan,
	.day=1,
	.hour=0,
	.min=0,
	.sec=0,
	.initRTC_Count=0,
	.rtc_cali_val =0
};

static uint32_t calibrate_one(rtc_cal_sel_t cal_clk)
{
    const uint32_t cal_count = 1000;
    const float factor = (1 << 19) * 1000.0f;
    uint32_t cali_val;
	
    cali_val = rtc_clk_cal(cal_clk, cal_count);
    printf("%.3f kHz\n", factor / (float) cali_val);
	
    return cali_val;
}


int str_to_format(char *cpDATE, char *cpTIME){
	switch(cpDATE[0]){
		case 'J':
			if(cpDATE[1] == 'a')
				DateTime.month = Jan;
			else{
				if(cpDATE[2] == 'n')
					DateTime.month = Jun;
				else
					DateTime.month = Jul;
			}
			break;
		case 'F':
			DateTime.month=Feb;
			break;
		case 'M':
			if(cpDATE[2] == 'r')
				DateTime.month = Mar;
			else
				DateTime.month=May;
			break;
		case 'A':
			if(cpDATE[1] == 'p')
				DateTime.month=Apr;
			else
				DateTime.month=Aug;
			break;
		case 'S':
			DateTime.month=Sep;
			break;
		case 'O':
			DateTime.month=Oct;
			break;
		case 'N':
			DateTime.month=Nov;
			break;
		case 'D':
			DateTime.month=Dec;
			break;
		default:
			printf("err:date error\n");
			return 1;
			break;
	}
	DateTime.day = (uint8_t)atoi(cpDATE+4);
	DateTime.year = atoi(cpDATE+16);
	DateTime.hour = (uint8_t)atoi(cpTIME);
	DateTime.min = (uint8_t)atoi(cpTIME+3);
	DateTime.sec = (uint8_t)atoi(cpTIME+6);
	return 0;
}

void date_sync(void){
}

static void sec_to_time(uint64_t u64_Sec_us, DATE_STU *Date){
	uint64_t Sec = u64_Sec_us/1000/1000;
	Date->sec = Sec % 60;
	Sec /= 60;
	Date->min = Sec % 60;
	Sec /= 60;
	Date->hour = Sec % 24;
	Sec /= 24;
	Date->day = Sec;
}

//calc date increse TODO:
static void calc_date(uint32_t increseDays){
	return;
}

//calc time increse
static void timeAdd(DATE_STU *Time, DATE_STU increseDate){
	int c=0;
	Time->sec = (DateTime.sec + increseDate.sec)%60;
	c=(DateTime.sec + increseDate.sec)/60;
	Time->min = (DateTime.min + increseDate.min + c)%60;
	c=(DateTime.min + increseDate.min + c)/60;
	Time->hour = (DateTime.hour + increseDate.hour + c)%24;
	c=(DateTime.hour + increseDate.hour)/24;

	calc_date(increseDate.day);
}

void getCurTime(DATE_STU *Time){
	uint64_t pass_Time;
	uint64_t curRTC_Count = rtc_time_get();
	pass_Time = rtc_time_slowclk_to_us(curRTC_Count-DateTime.initRTC_Count, DateTime.rtc_cali_val);
	DATE_STU increseDate={.hour=0,.min=0,.sec=0,.day=0};
	sec_to_time(pass_Time, &increseDate);
	timeAdd(Time, increseDate);

	Time->year = DateTime.year;
	Time->month = DateTime.month;
	Time->day = DateTime.day;
	return;
}

static void initialize_sntp(void)
{
    printf("Initializing SNTP\n");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

static void obtain_time(void)
{
    
    initialize_sntp();
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

void sntp_sync(){	
	time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	// Is time set? If not, tm_year will be (1970 - 1900).
	if (timeinfo.tm_year < (2016 - 1900)) {
		printf( "Time is not set yet. Connecting to WiFi and getting time over NTP.");
		obtain_time();

		// update 'now' variable with current time
		time(&now);
	}
	char strftime_buf[64];
	// Set timezone to China Standard Time
	setenv("TZ", "CST-8", 1);
	tzset();
	localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	
	printf("realy time:%s\n", strftime_buf);
	str_to_format(strftime_buf+4, strftime_buf+11);

	//sync rtc
	DateTime.initRTC_Count = rtc_time_get();
}


/* 
 * get the param from sd card, init module
 */
ERR_STATUS eds_module_init(void){
	//sntp_sync();
	DateTime.rtc_cali_val = calibrate_one(0);
	DateTime.initRTC_Count = rtc_time_get();
	printf("system time:%d %s %d, %d:%d:%d\n", DateTime.year, ccpMonth[DateTime.month], DateTime.day, DateTime.hour, DateTime.min, DateTime.sec);
	return SUCCESS;
}

/* 
 * check is chip plug in from a gpio
 */
bool esd_mi_chip_check(void){
	
	return 	SUCCESS;
}



