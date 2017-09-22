deps_config := \
	/mnt/Share/esp-idf-master/esp-idf/components/aws_iot/Kconfig \
	/mnt/Share/esp-idf-master/esp-idf/components/bt/Kconfig \
	/mnt/Share/esp-idf-master/esp-idf/components/esp32/Kconfig \
	/mnt/Share/esp-idf-master/esp-idf/components/ethernet/Kconfig \
	/mnt/Share/esp-idf-master/esp-idf/components/fatfs/Kconfig \
	/mnt/Share/esp-idf-master/esp-idf/components/freertos/Kconfig \
	/mnt/Share/esp-idf-master/esp-idf/components/log/Kconfig \
	/mnt/Share/esp-idf-master/esp-idf/components/lwip/Kconfig \
	/mnt/Share/esp-idf-master/esp-idf/components/mbedtls/Kconfig \
	/mnt/Share/esp-idf-master/esp-idf/components/openssl/Kconfig \
	/mnt/Share/esp-idf-master/esp-idf/components/spi_flash/Kconfig \
	/mnt/Share/esp-idf-master/esp-idf/components/bootloader/Kconfig.projbuild \
	/mnt/Share/esp-idf-master/esp-idf/components/esptool_py/Kconfig.projbuild \
	/mnt/Share/esp-idf-master/esp-idf/components/partition_table/Kconfig.projbuild \
	/mnt/Share/esp-idf-master/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
