# 温湿度计程序

**注意，若使用5V供电（本项目电路板的方案）调试完成后记得将main函数的`#define POWER_PIN 0` 这个define改为1，否则将无法使用。**

硬件方案：152x152 1.54寸墨水屏加SHT40传感，蓝牙广播发送

## 文件说明：

`SHT40.c`：软i2c温度传感器驱动

`EPD_1IN54_SSD1680.c`：功耗优化过的墨水屏驱动

`font16.c`：字体

`imageData.h`：图像数据，使用img2lcd取模

`minigui.c`：生成显存内容的库


可将项目内文件都复制到mounriver的rf_basic项目下，预计同样可编译和下载
