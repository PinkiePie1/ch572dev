# 裸机EPD程序

1.54寸墨水屏驱动。
使用SPI.

## 接线

SPI0接口：
```
SCK(CSL)默认在PA13
MOSI(SDA)默认在PA14
```
如需改动需要配置GPIO重映射。

其他端口：RES DC CS BUSY为通用GPIO，默认配置：
```
PA12 RES 输出
PA7 DC 输出
PA9 CS 输出
PA8 BUSY 输入
```
