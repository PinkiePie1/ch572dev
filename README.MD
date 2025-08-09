使用xmake的话，文件系统最好不要在windows下，而是到linux下，否则编译时间会延长很多。

从wch官网下载工具链。
编译之前，设置工具链的位置：`xmake f -p cross --sdk=/mnt/c/MRStoolChain/RISC-V\ Embedded\ GCC12`

或者通过`xmake f --menu`中的Basic Configuraion-Cross Complation Configuration-Set cross SDK directory中设置

使用下载器之前给minichlink执行权限：`chmod +x minichlink`
通过`xmake f --menu`的project configuration中的对应选项可以选择编译后是否下载，以及下载前是否进行unbrick（用于需要睡眠的程序）

查看有哪些可编译项目: `xmake -h`

编译：`xmake -b blink`

====

部分examples下有Makefile，也可直接在对应目录执行`make -j16 all`进行编译，`make -j16 flash`进行编译加下载
