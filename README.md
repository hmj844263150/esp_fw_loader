# esp download firmware

Note:
2017/5/3:
项目目前计划从uart直接下载固件下手，目前已经完成目标模块的启动并进入download模式，并完成同步。

2017/5/10：
完成从SD卡读取参数的功能

2017/5/11：
完成从SD卡按格式读取image功能

2017/5/19：
完成添加LCD驱动，能正常显示打印信息

2017/5/27：
完成单独下载固件功能，并验证通过

2017/6/5(11:29):
完成基本烧录功能，并验证通过

2017/6/5:
添加记录结果的功能。

2017/6/9:
添加stub功能，串口速率提升到115200*16，数据包大小为16KB

2017/6/13:
添加分段烧写多个bin文件的功能
目前需要添加的功能有：sntp、连接服务器并上传记录

2017/6/14:
验证8266下载的功能
目前第一个版本暂时不添加服务器上传的功能

2017/6/15:
修改上电启动操作：先读取SD配置信息，包括wifi连接信息，再进行wifi连接操作，5秒为wifi连接超时；
修正读取sd卡时存在的一个bug
改善芯片同步时的时间
验证使用机台进行下载的操作