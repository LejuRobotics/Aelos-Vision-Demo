#@创建时间 2017-07-01
#@最后修改 2017-07-13

目前项目包括两个工程文件，一个是TCP_Server _Console,放在nanopi上运行的服务端，通过opencv获取摄像头图像，通过UDP协议发送给客户端，同时搭建了一个TCP服务器，将计算和识别到的结果通过TCP协议发送给客户端并向串口发送动作指令；另外一个是QT-ColorSegmentation,是放在windows PC端运行的客户端，负责接收显示Linux nanopi上的摄像头图像，以及选取目标颜色、与服务端进行通信。

***
开发环境：该项目全部由qt creartor开发工具编写完成，基于nanopi上安装的qt版本是5.3.2，建议使用基于这个版本以上的qt cteator，本人使用的是qt5.8.0。

运行环境： 实际使用中，客户端是放在windows PC机上运行，服务端是放在Linux nanopi上运行，服务端所在系统上需要安装摄像头，依赖opencv库文件对图像进行处理， 工程目录中已包含能在windows和arm-linux上运行的opencv库文件，版本是opencv3.2.0，如果需要在其它平台上测试，可自己下载源码编译。

编译程序： 客户端直接在windows上编译就行，服务端需要在nanopi上进行编译（比较简单，推荐）,或者在linux系统上交叉编译，再放到nanopi上,源代码可通过共享文件夹放到nanopi上面，后面会讲。

(注： nanopi上运行的是debian系统，登陆nanopi上可借助putty,sourceCRT等工具，选择以ssh协议登入,我是用sudo apt-get install qt5-default安装的qt,版本是5.3.2，进入需要编译的源文件，qmake,然后make,在这里需要说明的是，由于之前已经安装了一个qt4，qmake默认是使用qt4去生成Makefile文件，现在已经更改成使用qt5的qmake，以nanopi上为例，修改/usr/lib/arm-linux-gnueabihf/qt-default/qtchooser文件，把文件中的qt4改为qt5就行了，不然需要指定qmake路径去生成编译文件，比如执行/usr/lib/arm-linux-gnueabihf/qt5/bin/qmake) 
***

其他： 

1. 关于客户端连接服务端，如果服务端没有连接上路由器，会自动变成热点模式，热点模式下nanopi固定IP为192.168.8.1，热点名称是机器人编号,比如AELOS150D00F，目前密码为12345678，可以通过客户端上的设置界面设置机器人连接指定的路由器。
2. nanopi上已安装samba服务器，可以在windows上访问nanopi上的共享文件夹，可以把要编译的源代码放到这个文件夹，登陆方法，在运行框中输入\192.168.8.1，然后运行，这个ip是nanopi上当前的ip地址，进入这个共享文件夹需要账号密码，账号为aelos,密码aelos
3. 我们已经设定TCP _Server _ Console服务器为自启动，若果是重新编译这个程序，在编译成功以后，需要把编译好的执行文件放到/opt/qt-project/TCP _Serve _Console下，重启。如果需要手动运行，先杀死已经运行的这个程序的进程，不然串口被占用。
4. 关于使用doxgen生成html文件，注释规范，网上有很多例子，也可以参考本项目中的工程，我在这里介绍一下如何使用doxgen工具生成html文档。先在官网http://www.doxygen.nl/download.html上面去下载工具，记得下载完整的，包括GUI工具，具体使用可参考http://blog.csdn.net/u010901792/article/details/52804232，http://www.cnblogs.com/chenyang920/p/5732643.html