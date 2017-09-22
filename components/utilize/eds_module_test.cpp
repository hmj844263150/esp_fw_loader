#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include "eds_module_test.h"

using namespace std;

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX - 1)
#define NOCPLUSPLUS_11 
typedef long long _Longlong;


extern "C" int module_test_cpp(char* mdl_logs);


int read_csv_data(map<string, vector<vector<string>>> &csv_dict, string file_path);
int read_log_data(map<string, vector<vector<string>>> &log_dict, string file_path, string mode);
int data_process_dictList(map<string, vector<vector<string>>> &csv_dic, map<string, vector<vector<string>>> &log_dic, int adc_en, char* mdl_logs);
void map_example();
void vector_insert(vector<vector<string>>, string str);
vector< string> split_2( string str, string pattern); //for specific func
vector< string> split( string str, string pattern);
string& replace_all_distinct(string& str, const string& old_value, const string& new_value);
string& strip(string &s, const string sp);
string& strip_esp32(string &s, const string sp);
string& trim(string &s);
string join(vector<string> &s, const string sp, int offset);
void size_values(map<string, vector<vector<string>>> &csv_dic, map<string, vector<vector<string>>> &log_dic);
vector<int> split_toi(string str, string pattern);

int stoi(const string& _Str);
float stof(const string& _Str);
inline string to_string(_Longlong mun);


enum err_module_test{
	RMT_SUCCESS = 0,
	RMT_OPENFILE,
};

int module_test_cpp(char* mdl_logs){
	cout << "module_test_cpp start" << endl;
	
	map<string, vector<vector<string>>> map_csv;  
	map<string, vector<vector<string>>> map_log;
	
	if(read_log_data(map_log, "/sdcard/mdltest/MDLRST.TXT", "ESP32") != 0){
		cout << "read log failed\n" << endl;
		return -1;
	}
	
	if(read_csv_data(map_csv, "/sdcard/mdltest/fth.csv") != 0){
		cout << "read csv failed\n" << endl;
		return -1;
	}
	//size_values(map_csv, map_log);

	if(data_process_dictList(map_csv, map_log, 1, mdl_logs) != 0){
		cout << "process failed\n" << endl;
		return -1;
	}
	
	cout << "module_test_cpp end" <<endl;
	return 0;
}

int data_process_dictList(map<string, vector<vector<string>>> &csv_dic, map<string, vector<vector<string>>> &log_dic, int adc_en, char* mdl_logs){
	string r = "pass";
	string log = "logs here: \n";
	char log_tmp[100];
	vector<string> fail_list;
	string item="";

	map<string, vector<vector<string>>>::iterator m_it;
	m_it = csv_dic.begin();

	bool single_res = true;
	int i, j;
	while(m_it != csv_dic.end())
	{
		item = m_it->first + "\0";
		//cout << item<<":"<<csv_dic[item][0][0] << endl;
		
		if(log_dic.count(m_it->first) > 0 && log_dic[m_it->first][1].size() > 0){
			if(item.compare("fb_rssi")==0){
				;
			}
			if(item.compare("TOUT")==0){
				if(adc_en)
					if(stoi(log_dic[item][1][0]) < stoi(csv_dic[item][0][0]) || stoi(log_dic[item][1][0]) > stoi(csv_dic[item][1][0])){
						sprintf((char *)&log_tmp, "Part failure in %s  : %d < %d < %d \n ", item.c_str(), stoi(csv_dic[item][0][0]), stoi(log_dic[item][1][0]), stoi(csv_dic[item][1][0]));
						log.append(log_tmp);
						fail_list.push_back(item);
						single_res = false;
					}
			}
			else if(item.compare("RX_NOISEFLOOR")==0){
				set<int> val_tmp;
				int i;
				for(i=0; i<log_dic[item][1].size(); i++)
					val_tmp.insert(stoi(log_dic[item][1][i]));
				
				if(val_tmp.size()==1 && (val_tmp.find(-340)!=val_tmp.end() || val_tmp.find(-392)!=val_tmp.end())){
					sprintf((char *)&log_tmp, "Part failure in %s  : -340/-392 \n ", item.c_str());
					log.append(log_tmp);
					fail_list.push_back(item);
					single_res = false;
				}
			}
			else if(item.compare("fb_rx_num_max")==0){
				int fb_rx_num_max = stoi(split(log_dic[item][1][0], ",")[0]);
				if(fb_rx_num_max < stoi(csv_dic[item][0][0]) || fb_rx_num_max > stoi(csv_dic[item][1][0])){
					sprintf((char *)&log_tmp, "Part failure in %s  : %d !< %d !< %d \n ", item.c_str(), stoi(csv_dic[item][0][0]), fb_rx_num_max, stoi(csv_dic[item][1][0]));
					log.append(log_tmp);
					fail_list.push_back(item);
					single_res = false;
				}
			}
			else if(item.compare("TXDC")==0){
				vector<int> txdc_i = split_toi(log_dic[item][1][0], ",");
				vector<int> txdc_q = split_toi(log_dic[item][1][0], ",");
				sort(txdc_i.begin(), txdc_i.end());
				sort(txdc_q.begin(), txdc_q.end());

				if(txdc_i[1] < stoi(csv_dic[item][0][0]) || txdc_i[2] > stoi(csv_dic[item][1][0])){
					sprintf((char *)&log_tmp, "Part failure in %s_%s : %d %d %d %d  \n", item.c_str(), "i", txdc_i[0], txdc_i[1], txdc_i[2], txdc_i[3]);
					log.append(log_tmp);
					fail_list.push_back(item);
					single_res = false;
				}
				if(txdc_q[1] < stoi(csv_dic[item][0][0]) || txdc_q[2] > stoi(csv_dic[item][1][0])){
					sprintf((char *)&log_tmp, "Part failure in %s_%s : %d %d %d %d  \n", item.c_str(), "i", txdc_q[0], txdc_q[1], txdc_q[2], txdc_q[3]);
					log.append(log_tmp);
					fail_list.push_back(item);
					single_res = false;
				}
			}
			else if(item.compare("RXDC_RFRX_BT")==0){
				m_it ++;
				continue;
			}
			else if(item.compare("RXIQ_TEST_5M_diff")==0){
				float RXIQ_TEST_data_gain = stof(split(log_dic[item][1][0], ",")[0]);
				float RXIQ_TEST_data_phase = stof(split(log_dic[item][1][1], ",")[0]);

				if(RXIQ_TEST_data_gain<stoi(csv_dic[item][0][0]) || RXIQ_TEST_data_gain>stoi(csv_dic[item][1][0]) || 
					RXIQ_TEST_data_phase<stoi(csv_dic[item][0][0]) || RXIQ_TEST_data_phase>stoi(csv_dic[item][1][0]))
					{
						sprintf((char *)&log_tmp, "Part failure in %s    \n ", item.c_str());
						log.append(log_tmp);
						fail_list.push_back(item);
						single_res = false;
					}
			}
			else if(csv_dic[item][0].size() == 1){
				//cout <<"0 "<< item << ":" << log_dic[item][1].size() << ";" << split(log_dic[item][1][0], ",").size() << endl;

				for(i=0; i<log_dic[item][1].size(); i++){
					for(j=0; j<split(log_dic[item][1][i], ",").size(); j++){
						float ftmp = stof(split(log_dic[item][1][i], ",")[j]);
						if(ftmp<stof(csv_dic[item][0][0]) || ftmp>stof(csv_dic[item][1][0])){
							sprintf((char *)&log_tmp, "Part failure in %s  #%d,#%d : %f !< %f !< %f  \n", item.c_str(), i, j, stof(csv_dic[item][0][0]), ftmp, stof(csv_dic[item][1][0]));
							log.append(log_tmp);
							fail_list.push_back(item);
							single_res = false;
						}
					}
				}
			}
			else if(log_dic[item][1].size() == csv_dic[item][0].size()){
				//cout <<"1  "<< item << ":" << log_dic[item][1].size() << ";" << split(log_dic[item][1][0], ",").size() << endl;
				for(i=0; i<log_dic[item][1].size(); i++){
					for(j=0; j<split(log_dic[item][1][i], ",").size(); j++){
						float ftmp = stof(split(log_dic[item][1][i], ",")[j]);
						if(ftmp<stof(csv_dic[item][0][i]) || ftmp>stof(csv_dic[item][1][i])){
							sprintf((char *)log_tmp, "Part failure in %s  #%d,#%d : %.2f !< %.2f !< %.2f  \n", item.c_str(), i, j, stof(csv_dic[item][0][i]), ftmp, stof(csv_dic[item][1][i]));
							//cout << item << endl;
							//cout << csv_dic[item][0][i] << ";" << csv_dic[item][1][i] << endl;
							log.append((string)log_tmp);
							fail_list.push_back(item);
							single_res = false;
						}
					}
				}
			}
			else if(split(log_dic[item][1][0], ",").size() == csv_dic[item][0].size()){
				//cout <<"2  "<< item << ":" << log_dic[item][1].size() << ";" << split(log_dic[item][1][0], ",").size() << endl;
				for(i=0; i<log_dic[item][1].size(); i++){
					for(j=0; j<split(log_dic[item][1][i], ",").size(); j++){
						float ftmp = stof(split(log_dic[item][1][i], ",")[j]);
						if(ftmp<stof(csv_dic[item][0][j]) || ftmp>stof(csv_dic[item][1][j])){
							sprintf((char *)&log_tmp, "Part failure in %s  #%d,#%d : %.2f !< %.2f !< %.2f  \n", item.c_str(), i, j, stof(csv_dic[item][0][j]), ftmp, stof(csv_dic[item][1][j]));
							//cout << csv_dic[item][0][j] << ";" << csv_dic[item][1][j] << endl;
							log.append(log_tmp);
							fail_list.push_back(item);
							single_res = false;
						}
					}
				}
			}
			else{
				cout <<"3  hehe:"<< item << endl;
				cout << "csv size:" << csv_dic[item][0].size() << endl;	
				cout << "csv content:" << csv_dic[item][0][csv_dic[item][0].size()-1] << endl;
				cout << "log_dic_1 size:" << log_dic[item][1].size() << endl;
				cout << "log_dic_1_0 size:" << split(log_dic[item][1][0], ",").size() << endl;
			}
		}
		m_it ++;
	}
	cout << log << endl;

	stringstream sstr;
	sstr.clear();
	sstr << log;
	sstr >> mdl_logs;
	if(single_res){
		log.append("single_chip passed\r\n");
		return 0;
	}
	else
		return 1;
	
	
}

int read_csv_data(map<string, vector<vector<string>>> &csv_dict, string file_path){
	cout << "start to load csv" << endl;
	char buf[1024];
	string str;
	vector<string> str_list;
	vector<vector<string>> li_li;
	li_li.push_back(str_list);
	li_li.push_back(str_list);

	string item = "";

	ifstream fcsv(file_path);
	if(! fcsv.is_open()){
		cout << "Err open:" << file_path << endl;
		return RMT_OPENFILE;
	}
	while (!fcsv.eof())
    {
        fcsv.getline(buf,1024);
        str = buf;
		
		if(str.find(",,,,,,,,") != string::npos)
			continue;
		str = strip_esp32(str, ",");

		if(str.empty())
			continue;
		//cout << str << endl;
		str_list = split_2(str, ",");
		
		if(str_list[str_list.size()-1].length() <= 0 ){
			cout << "empty" << endl;
		}
		
		if(str.find("UPPER") == 0){
			csv_dict[str.substr(6, str.find_first_of(',',0)-6)] = li_li;
			csv_dict[str.substr(6, str.find_first_of(',',0)-6)][1] = str_list;
		}
		else if(str.find("LOWER") == 0)
			csv_dict[str.substr(6, str.find_first_of(',',0)-6)][0] = str_list;
    }
	cout<<"end to load csv"<<endl;
	return RMT_SUCCESS;
}

int read_log_data(map<string, vector<vector<string>>> &values, string file_path, string mode){
	cout<<"start to load log"<<endl;
	char buf[1024];
	string str;
	vector<string> str_list, vs_tmp;
	vector<vector<string>> li_li, line;
	li_li.push_back(str_list);
	li_li.push_back(str_list);
	string end_flg = "";

#if 1 //数据预处理模块
{
	values.insert(pair<string, vector<vector<string>>>("rx_para_cal", li_li));
	values.insert(pair<string, vector<vector<string>>>("fb_rx_num_max", li_li));
	values.insert(pair<string, vector<vector<string>>>("TXCAP_TMX2G_CCT_LOAD", li_li));
	values.insert(pair<string, vector<vector<string>>>("rombist_rslt", li_li));
	values.insert(pair<string, vector<vector<string>>>("rxiq_compute_num", li_li));
	values.insert(pair<string, vector<vector<string>>>("rx_switch_gain_check", li_li));
	values.insert(pair<string, vector<vector<string>>>("RXIQ_TEST_5M_diff", li_li));
	values.insert(pair<string, vector<vector<string>>>("RXIQ_tot_power", li_li));
	values.insert(pair<string, vector<vector<string>>>("freq_offset_cal", li_li));
	values.insert(pair<string, vector<vector<string>>>("DeepSleep_IDD_VBAT", li_li));
	values.insert(pair<string, vector<vector<string>>>("RXDC", li_li));
	values.insert(pair<string, vector<vector<string>>>("BT_TXDC", li_li));
	values.insert(pair<string, vector<vector<string>>>("txreq_start_time", li_li));
	values.insert(pair<string, vector<vector<string>>>("RX_SWITCH_GAIN", li_li));
	values.insert(pair<string, vector<vector<string>>>("dut_rxrssi", li_li));
	values.insert(pair<string, vector<vector<string>>>("RX_NOISEFLOOR", li_li));
	values.insert(pair<string, vector<vector<string>>>("rx_para_cal_tone", li_li));
	values.insert(pair<string, vector<vector<string>>>("ADC_DAC_SNR", li_li));
	values.insert(pair<string, vector<vector<string>>>("txp_state", li_li));
	values.insert(pair<string, vector<vector<string>>>("adc_dac_snr_2tone", li_li));
	values.insert(pair<string, vector<vector<string>>>("TXIQ", li_li));
	values.insert(pair<string, vector<vector<string>>>("LightSleep_IDD_DVDD_IO", li_li));
	values.insert(pair<string, vector<vector<string>>>("vdd33", li_li));
	values.insert(pair<string, vector<vector<string>>>("LightSleep_IDD_VBAT", li_li));
	values.insert(pair<string, vector<vector<string>>>("DeepSleep_IDD_DVDD_IO", li_li));
	values.insert(pair<string, vector<vector<string>>>("rxsdut_max_rssi", li_li));
	values.insert(pair<string, vector<vector<string>>>("dco_sweep_test_DCO", li_li));
	values.insert(pair<string, vector<vector<string>>>("rc_cal_dout", li_li));
	values.insert(pair<string, vector<vector<string>>>("wifi_init_time", li_li));
	values.insert(pair<string, vector<vector<string>>>("RX_GAIN_CHECK", li_li));
	values.insert(pair<string, vector<vector<string>>>("VDD33", li_li));
	values.insert(pair<string, vector<vector<string>>>("txp_pwctrl_atten", li_li));
	values.insert(pair<string, vector<vector<string>>>("TX_POWER_BACKOFF", li_li));
	values.insert(pair<string, vector<vector<string>>>("TXDC", li_li));
	values.insert(pair<string, vector<vector<string>>>("CHIP_ID", li_li));
	values.insert(pair<string, vector<vector<string>>>("RXIQ_TEST_-5M", li_li));
	values.insert(pair<string, vector<vector<string>>>("cal_rf_ana_gain", li_li));
	values.insert(pair<string, vector<vector<string>>>("TXBB_TXIQ", li_li));
	values.insert(pair<string, vector<vector<string>>>("RX_PATH_GAIN", li_li));
	values.insert(pair<string, vector<vector<string>>>("TX_PWRCTRL_ATTEN", li_li));
	values.insert(pair<string, vector<vector<string>>>("TXCAP_PA2G_CCT_STG1", li_li));
	values.insert(pair<string, vector<vector<string>>>("TXCAP_PA2G_CCT_STG2", li_li));
	values.insert(pair<string, vector<vector<string>>>("site_num", li_li));
	values.insert(pair<string, vector<vector<string>>>("txp_result", li_li));
	values.insert(pair<string, vector<vector<string>>>("fb_rx_num", li_li));
	values.insert(pair<string, vector<vector<string>>>("BT_TXIQ", li_li));
	values.insert(pair<string, vector<vector<string>>>("fb_rx_num_sum", li_li));
	values.insert(pair<string, vector<vector<string>>>("wi_pad 3 and ri_pad 5", li_li));
	values.insert(pair<string, vector<vector<string>>>("check_result_t", li_li));
	values.insert(pair<string, vector<vector<string>>>("TX_PWCTRL_CHAN_OFFSET", li_li));
	values.insert(pair<string, vector<vector<string>>>("TX_VDD33", li_li));
	values.insert(pair<string, vector<vector<string>>>("RX_GAIN_CHECK_POWER_hdb", li_li));
	values.insert(pair<string, vector<vector<string>>>("Chip_PD_IDD_DVDD_IO", li_li));
	values.insert(pair<string, vector<vector<string>>>("BBRX2_RXIQ", li_li));
	values.insert(pair<string, vector<vector<string>>>("filepath", li_li));
	values.insert(pair<string, vector<vector<string>>>("RXIQ_REMAIN", li_li));
	values.insert(pair<string, vector<vector<string>>>("timeout_fail", li_li));
	values.insert(pair<string, vector<vector<string>>>("io_test_result", li_li));
	values.insert(pair<string, vector<vector<string>>>("RXIQ_TEST_5M", li_li));
	values.insert(pair<string, vector<vector<string>>>("wi_pad 0 and ri_pad 4", li_li));
	values.insert(pair<string, vector<vector<string>>>("rxiq_cover_fail_num", li_li));
	values.insert(pair<string, vector<vector<string>>>("dco_sweep_test_ADC_STEP", li_li));
	values.insert(pair<string, vector<vector<string>>>("TX_VDD33_DIFF", li_li));
	values.insert(pair<string, vector<vector<string>>>("rxsdut_cnt", li_li));
	values.insert(pair<string, vector<vector<string>>>("RXDC_RXBB_BT", li_li));
	values.insert(pair<string, vector<vector<string>>>("rx_para_cal_tone_sig_pwr_db_2", li_li));
	values.insert(pair<string, vector<vector<string>>>("rx_para_cal_tone_sig_pwr_db_3", li_li));
	values.insert(pair<string, vector<vector<string>>>("rx_para_cal_tone_sig_pwr_db_1", li_li));
	values.insert(pair<string, vector<vector<string>>>("rx_para_cal_tone_sig_pwr_db_4", li_li));
	values.insert(pair<string, vector<vector<string>>>("rssi", li_li));
	values.insert(pair<string, vector<vector<string>>>("RXDC_RFRX_BT", li_li));
	values.insert(pair<string, vector<vector<string>>>("RX_PATH_SNR", li_li));
	values.insert(pair<string, vector<vector<string>>>("VDD_RTC_testV1", li_li));
	values.insert(pair<string, vector<vector<string>>>("CHIP_VERSION", li_li));
	values.insert(pair<string, vector<vector<string>>>("FREQ_OFFSET", li_li));
	values.insert(pair<string, vector<vector<string>>>("timer expire", li_li));
	values.insert(pair<string, vector<vector<string>>>("rx_suc_num", li_li));
	values.insert(pair<string, vector<vector<string>>>("AnaWorkIDD_VBAT", li_li));
	values.insert(pair<string, vector<vector<string>>>("dut_rx_num", li_li));
	values.insert(pair<string, vector<vector<string>>>("SVN_Version", li_li));
	values.insert(pair<string, vector<vector<string>>>("RTC_freq_170khz", li_li));
	values.insert(pair<string, vector<vector<string>>>("RTC_freq_70khz", li_li));
	values.insert(pair<string, vector<vector<string>>>("Chip_PD_IDD_VBAT", li_li));
	values.insert(pair<string, vector<vector<string>>>("AnaWorkIDD_DVDD_IO", li_li));
	values.insert(pair<string, vector<vector<string>>>("RXIQ", li_li));
	values.insert(pair<string, vector<vector<string>>>("fb_rxrssi", li_li));
	values.insert(pair<string, vector<vector<string>>>("VDD_RTC_testV2", li_li));
	values.insert(pair<string, vector<vector<string>>>("RXDC_RFRX_WIFI", li_li));
	values.insert(pair<string, vector<vector<string>>>("TEST_NUM", li_li));
	values.insert(pair<string, vector<vector<string>>>("TOUT", li_li));
	values.insert(pair<string, vector<vector<string>>>("TXBB_TXDC", li_li));
	values.insert(pair<string, vector<vector<string>>>("WIFI_INIT_ITEM", li_li));
	values.insert(pair<string, vector<vector<string>>>("DVDD_testV1", li_li));
	values.insert(pair<string, vector<vector<string>>>("DVDD_testV2", li_li));


	string tmp_li[][5] = {
		{""},
		{"_1","_2","_3"},
		{"_gain","_phase"},
		{"_i","_q"},
		{"_CH1","_CH6","_CH11"},
		{"_1","_2"},
		{"_gain","_phase"},
		{"_CH1","_CH6","_CH11"},
		{"_gain","_phase"},
		{"_gain","_phase"},
		{"_i","_q"},
		{"_i","_q"},
		{"_gain","_phase"},
		{"_c_i","_c_q","_f_i","_f_q"},
		{"_bbrx1","_bbrx2","_total_pwr_db","_sig_pwr_db","_sw_g"},
		{"_gain","_phase"},
		{"_gain","_phase"},
		{"_gain","_phase"},
		
		
	};
	values["rx_para_cal"][0].assign(begin(tmp_li[1]), end(tmp_li[1])-2);
	values["TXBB_TXIQ"][0].assign(begin(tmp_li[2]), end(tmp_li[2])-3);
	values["TXBB_TXDC"][0].assign(begin(tmp_li[3]), end(tmp_li[3])-3);
	values["RX_GAIN_CHECK"][0].assign(begin(tmp_li[4]), end(tmp_li[4])-2);
	values["RX_GAIN_CHECK_POWER_hdb"][0].assign(begin(tmp_li[5]), end(tmp_li[5])-3);
	values["BBRX2_RXIQ"][0].assign(begin(tmp_li[6]), end(tmp_li[6])-3);
	values["RX_NOISEFLOOR"][0].assign(begin(tmp_li[7]), end(tmp_li[7])-2);
	values["TXIQ"][0].assign(begin(tmp_li[8]), end(tmp_li[8])-3);
	values["BT_TXIQ"][0].assign(begin(tmp_li[9]), end(tmp_li[9])-3);
	values["TXDC"][0].assign(begin(tmp_li[10]), end(tmp_li[10])-3);
	values["BT_TXDC"][0].assign(begin(tmp_li[11]), end(tmp_li[11])-3);
	values["RXIQ"][0].assign(begin(tmp_li[12]), end(tmp_li[12])-3);
	values["RXDC"][0].assign(begin(tmp_li[13]), end(tmp_li[13])-1);
	values["rx_switch_gain_check"][0].assign(begin(tmp_li[14]), end(tmp_li[14])-0);
	values["RXIQ_TEST_-5M"][0].assign(begin(tmp_li[15]), end(tmp_li[15])-3);
	values["RXIQ_TEST_5M"][0].assign(begin(tmp_li[16]), end(tmp_li[16])-3);
	values["RXIQ_TEST_5M_diff"][0].assign(begin(tmp_li[17]), end(tmp_li[17])-3);

	values["fb_rx_num_max"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["TXCAP_TMX2G_CCT_LOAD"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["rxiq_compute_num"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["RXIQ_tot_power"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["RX_SWITCH_GAIN"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["dut_rxrssi"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["rx_para_cal_tone"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["ADC_DAC_SNR"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["txp_state"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["rxsdut_max_rssi"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["wifi_init_time"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["txp_pwctrl_atten"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["TX_POWER_BACKOFF"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["CHIP_ID"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["RX_PATH_GAIN"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["TX_PWRCTRL_ATTEN"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["TXCAP_PA2G_CCT_STG1"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["TXCAP_PA2G_CCT_STG2"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["txp_result"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["fb_rx_num"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["TX_PWCTRL_CHAN_OFFSET"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["RXIQ_REMAIN"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["TX_VDD33_DIFF"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["rxsdut_cnt"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["rx_para_cal_tone_sig_pwr_db_2"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["rx_para_cal_tone_sig_pwr_db_3"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["rx_para_cal_tone_sig_pwr_db_1"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["rx_para_cal_tone_sig_pwr_db_4"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["rssi"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["RX_PATH_SNR"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["CHIP_VERSION"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["FREQ_OFFSET"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["rx_suc_num"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["dut_rx_num"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["SVN_Version"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["RTC_freq_170khz"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["RTC_freq_70khz"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["fb_rxrssi"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);
	values["WIFI_INIT_ITEM"][0].assign(begin(tmp_li[0]), end(tmp_li[0])-4);

}
#endif

#if 1 //mode deal
	if(mode.compare("module") == 0){}
	else if(mode.compare("ate log") == 0){}
	else if(mode.compare("ate_0530_2noisefloor") == 0){}
	else if(mode.compare("ate_0530_4noisefloor") == 0){}
	else if(mode.compare("ate_new") == 0){}
	else if(mode.compare("module2515") == 0){
		end_flg.operator=("TEST_NUM");
		
		values["dco_sweep_test_ADC_STEP"][0] = split("_min_i','_max_i','_min_q','_max_q", "','");
		values["dco_sweep_test_DCO"][0] = split("_low_i','_hgh_i','_low_q','_hgh_q", "','");
		values["RX_NOISEFLOOR"][0].push_back("_CH14");
	}
	else if(mode.compare("ESP32") == 0){
		end_flg.operator=("MODULE_TEST EDN!!!");

		values["dco_sweep_test_ADC_STEP"][0] = split("_min_i','_max_i','_min_q','_max_q", "','");
		values["dco_sweep_test_DCO"][0] = split("_low_i','_hgh_i','_low_q','_hgh_q", "','");
		//values["dco_sweep_test_ADC_STEP"][0] = split("_min_i','_max_i','_min_q','_max_q", "','");
		//values["dco_sweep_test_ADC_STEP"][0] = split("_low_i','_hgh_i','_low_q','_hgh_q", "','");
	}
	else if(mode.compare("ate") == 0){}
	else if(mode.compare("130608_fpga") == 0){}
	else if(mode.compare("130624_fpga") == 0){}
	else if(mode.compare("130626_fpga") == 0){}
	else if(mode.compare("ate130716") == 0){}
#endif 
	string item = ""; 
		
	ifstream fcsv(file_path);
	if(! fcsv.is_open()){
		cout << "Err open:" << file_path << endl;
		return RMT_OPENFILE;
	}
	while (!fcsv.eof())
	{
		fcsv.getline(buf,1024);
        str = buf;
		if(str.size() <= 1)
			continue;
		//cout << str << endl;
		if(str.find("site_num") != string::npos)
			str = replace_all_distinct(replace_all_distinct(str, ",", "="), ";", ",");
		else if(str.find("dco_sweep_test_ADC_STEP") != string::npos || str.find("dco_sweep_test_DCO") != string::npos )
			str = replace_all_distinct(str, ";", ",");
		else if(str.find("RTC_freq_170khz") != string::npos || str.find("RTC_freq_70khz") != string::npos )
			str = replace_all_distinct(str, "=", ",");
		else if(str.find("rssi") != string::npos || str.find("rx_suc_num") != string::npos )
			str = replace_all_distinct(str, ":", ",");
		else if(str.find("TOUT") != string::npos)
			str = replace_all_distinct(str, "=", ",");

		str = replace_all_distinct(replace_all_distinct(replace_all_distinct(replace_all_distinct(str, ":", ","), "PPM", ""), "us", ""), "dB", "");
		str = strip(strip(strip(strip(strip(str, "\n"), "\n\r"), " "), ";"), ",");
		str_list = split(str, ",");

		if(str_list[0].find("=") != string::npos){
			str_list[0] = trim(split(str_list[0], "=")[0]) + "," + str_list[0];
			str_list = split(join(str_list, ",", 0), ",");
		}
		item = str_list[0];
		if(item.compare("dut_rx_num")==0 || item.compare("fb_rx_num")==0 || 
			item.compare("dut_rssi")==0 || item.compare("fb_rssi")==0)
			continue;

		str_list = split(join(str_list, ",", 1), ";");

		int i=0, j=0, k=0;
		line.clear();
		for(i=0; i<str_list.size(); i++ )
			line.push_back(split(str_list[i], ","));
		
		if(values.count(item) > 0){
			if(item.compare("TX_VDD33") == 0){
				if(values["vdd33"][1].size() == 0){
					values["TX_VDD33_DIFF"][1].push_back(to_string((_Longlong)(stoi(split(values["vdd33"][1][0], ",")[0]) - stoi(split(line[0][0], "=")[1]))));
				}
			}
			if(item.compare("TOUT") == 0){
				values["TOUT"][1].push_back(line[0][0]);
			}
			else if(line[0][0].find("=") != string::npos){
				for(i=0; i<line[0].size(); i++){
					if(line[0][i].find("=") != string::npos){
						values[item][0].push_back(split(line[0][i], "=")[0]);
						vs_tmp.push_back(split(line[0][i], "=")[1]);
					}
				}
				values[item][1].push_back(join(vs_tmp, ",", 0));
			}
			else if(item.find("IQ")!=string::npos || item.find("DC")!=string::npos || 
				item.compare("rx_switch_gain_check")==0 || item.compare("RX_GAIN_CHECK_POWER_hdb")==0){
					for(j=0; j<line[0].size(); j++){
						vs_tmp.clear();
						for(k=0; k<line.size(); k++)
							vs_tmp.push_back(line[k][j]);
						values[item][1].push_back(join(vs_tmp, ",", 0));
					}
			}
			else if(item.compare("timer expire") == 0){
				if(line[0][0].find("pass") != string::npos)
					cout << "hehe" << endl;
				else
					cout << "woqu" << endl;
			}
			else if(item.compare("TX_VDD33") == 0){
				cout << "item:" << item << endl;
				//cout << "line:" << line << endl;
			}
			else
				values[item][1].push_back(join(line[0], ",", 0));
		}
		if(item.find(end_flg) != string::npos){
			if(values["CHIP_ID"][1].size() != 0)
				values["filepath"][0].push_back(file_path);
			if(values["RXIQ_TEST_-5M"][1].size() != 0 && values["RXIQ_TEST_5M"][1].size() != 0){
				values["RXIQ_TEST_5M_diff"][1].push_back(to_string((_Longlong)(stoi(split(values["RXIQ_TEST_-5M"][1][0], ",")[0]) - stoi(split(values["RXIQ_TEST_-5M"][1][0], ",")[0]))));
                values["RXIQ_TEST_5M_diff"][1].push_back(to_string((_Longlong)(stoi(split(values["RXIQ_TEST_-5M"][1][1], ",")[0]) - stoi(split(values["RXIQ_TEST_-5M"][1][1], ",")[0]))));
			}
			if(values["fb_rx_num"][1].size() != 0){
				int max = stoi(values["fb_rx_num"][1][0]);
				for(i=0; i<values["fb_rx_num"][1].size(); i++){
					if(stoi(values["fb_rx_num"][1][i]) > max)
						max = stoi(values["fb_rx_num"][1][i]);
				}
				values["fb_rx_num_max"][1].push_back(to_string((_Longlong)max));
			}
		}
	}
	cout<<"end to load log"<<endl;
	return RMT_SUCCESS;
}

vector<int> split_toi(string str, string pattern){
	vector<int> ret;
	ret.push_back(1);
	if(pattern.empty()) 
		return ret;
	if(str.length() == 0)
		return ret;

	size_t start=0,index=str.find_first_of(pattern,0);
	while(index!=str.npos)
	{
		if(start!=index)
			ret.push_back(stoi(str.substr(start,index-start)));
		start=index+1;
		index=str.find_first_of(pattern,start);
	}
	if(!str.substr(start).empty()){
		ret.push_back(stoi(str.substr(start)));
	}
	return ret;
}

//字符串分割函数 for specific func
vector< string> split_2( string str, string pattern)
{
	vector<string> ret;
	if(pattern.empty()) 
		return ret;
	if(str.length() == 0)
		return ret;
	size_t start=0,index=str.find_first_of(pattern,0);
	start = index;
	while(index!=str.npos)
	{
		if(start!=index)
			ret.push_back(str.substr(start,index-start)+"\0");
			
		start=index+1;
		index=str.find_first_of(pattern,start);
	}
	if(!str.substr(start).empty())
		ret.push_back(str.substr(start));
	return ret;
}

//字符串分割函数 for normal
vector< string> split( string str, string pattern)
{
	vector<string> ret;
	if(pattern.empty()) 
		return ret;
	if(str.length() == 0)
		return ret;

	size_t start=0,index=str.find_first_of(pattern,0);
	while(index!=str.npos)
	{
		if(start!=index)
			ret.push_back(str.substr(start,index-start)+"\0");
		start=index+1;
		index=str.find_first_of(pattern,start);
	}
	if(!str.substr(start).empty())
		ret.push_back(str.substr(start));
	return ret;
}

string& replace_all_distinct(string& str, const string& old_value, const string& new_value){
	for(string::size_type pos(0); pos!=string::npos; pos+=new_value.length()){
		if((pos=str.find(old_value,pos)) != string::npos)
			str.replace(pos,old_value.length(),new_value);
		else
			break;
	}     
	return   str;     
}

string& trim(string &s){
    if (s.empty())
		return s;

    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

string& strip(string &s, const string sp){
    if (s.empty())
		return s;
    s.erase(0,s.find_first_not_of(sp));
    s.erase(s.find_last_not_of(sp) + 1);
    return s;
}


int find_last_not_of_esp32(const string s, const char c){	
	int rear = s.length()-2;
	while(rear > 0){
		if(s[rear] != c)
			return rear;
		rear --;
	}
	return rear;
}

string& strip_esp32(string &s, const string sp){
	if (s.empty())
		return s;
    s.erase(0,s.find_first_not_of(sp));
    s.erase(find_last_not_of_esp32(s, sp[0]) + 1);
    return s;
}


string join(vector<string> &s, const string sp, int offset){
	string str = "";
	vector<string>::iterator it;
	it = s.begin();
	it += offset;

	if(it == s.end())
		return s[0];

	str.append(it->data());
	it ++;
	while(it != s.end()){
		str.append(sp + it->data());
		it ++;
	}
	return str;
}

void size_values(map<string, vector<vector<string>>> &csv_dic, map<string, vector<vector<string>>> &log_dic){
	map<string, vector<vector<string>>>::iterator m_it;
	m_it = log_dic.begin();
	int sum=0, i=0, j=0;
	while(m_it != log_dic.end())
	{
		for(i=0; i<m_it->second.size(); i++)
			for(j=0; j<m_it->second[i].size(); j++)
				sum += m_it->second[i][j].length();
		m_it ++;
	}
	cout << sum <<endl;

	m_it = csv_dic.begin();
	sum = 0;
	while(m_it != csv_dic.end())
	{
		for(i=0; i<m_it->second.size(); i++)
			for(j=0; j<m_it->second[i].size(); j++)
				sum += m_it->second[i][j].length();
		m_it ++;
	}
	cout << sum <<endl;

	cout << &log_dic["ADC_DAC_SNR"] << endl;
	cout << &log_dic["wifi_init_time"] << endl;
}

int stoi(const string& _Str)
{	// convert string to int
	int _Ans = atoi(_Str.c_str());
	return ((int)_Ans);
}

float stof(const string& _Str)
{	// convert string to int
	float _Ans = atof(_Str.c_str());
	return ((float)_Ans);
}

inline string to_string(_Longlong mun){
	char c_str[50];
	sprintf(c_str, "%lld", mun);

	return (string)c_str;
}




void map_example(){
	map<const char*, string> value_list;
	value_list["1"] = "first";
	value_list["2"] = "second";
	value_list["3"] = "third";

	map<const char*, string>::iterator m_it;
	m_it = value_list.begin();

	while(m_it != value_list.end())
	{
		//it->first;
		//it->second;
		cout << "key:" << m_it->first;
		cout << ", value:" << m_it->second << endl;
		m_it ++;         
	}
	
}


