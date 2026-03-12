# WebSocketPractice2.0
跨平台（Windows/Linux）TCP网络通信库封装实践项目，基于C++实现底层Socket API的统一封装，屏蔽不同系统的接口差异，提供简洁、易用的TCP通信核心接口，支持基础的客户端/服务端开发，同时预留高并发、HTTP协议扩展能力。

## 项目简介
### 核心目标
解决原生Socket编程在Windows/Linux平台下接口不统一、开发适配成本高的问题，封装通用的XTcp类，提供“创建-绑定-监听-连接-收发数据-关闭”全流程TCP通信能力，降低上层网络应用开发门槛。

### 适用场景
- 本科/入门级网络编程学习与实践
- 轻量级TCP客户端/服务端快速开发
- 跨平台网络通信组件复用
- IO多路复用、HTTP协议等进阶网络编程的基础验证

## 技术栈
- 核心语言：C++
- 系统编程：Windows/Linux Socket API、跨平台系统调用适配
- 网络协议：TCP/IP、基础HTTP协议（扩展）
- 工程化：动态库/静态库编译、模块化拆分
- 进阶能力：IO多路复用（epoll/Linux）、阻塞/非阻塞套接字、连接超时控制

## 目录结构
```
WebSocketPractice2.0/
├── bin/                # 编译生成的可执行文件&动态库
│   ├── tcpclient.exe   # Windows客户端可执行程序
│   ├── tcpserver.exe   # Windows服务端可执行程序
│   ├── xsocket.dll     # XTcp核心动态库（Windows）
│   └── *.pdb           # 调试符号文件
├── lib/                # 编译生成的静态库文件
│   └── xsocket.lib     # XTcp核心静态库（Windows）
├── src/                # 核心源代码
│   ├── epoll/          # Linux epoll IO多路复用（预留扩展）
│   ├── tcpclient/      # TCP客户端实现（基于XTcp）
│   ├── tcpserver/      # TCP服务端实现（基于XTcp）
│   ├── xhttp/          # HTTP协议封装（基于TCP扩展）
│   └── xsocket/        # 核心：XTcp类实现
│       ├── XTcp.cpp
│       └── XTcp.h
├── backup/             # 模块备份文件（版本回溯）
└── README.md           # 项目说明文档
```

## 核心功能（XTcp类）
XTcp是项目的核心封装类，屏蔽跨平台差异，提供以下核心接口：

| 接口名         | 功能说明                                                                 |
|----------------|--------------------------------------------------------------------------|
| `CreateSocket` | 创建TCP套接字（AF_INET + SOCK_STREAM）                                  |
| `Bind`         | 绑定端口并启动监听（支持任意IP绑定，最大挂起连接数10）                   |
| `Accept`       | 阻塞接收客户端连接，返回新的XTcp对象（对应单个客户端）                   |
| `Connect`      | 客户端连接服务端（支持超时控制，避免无限阻塞）                           |
| `Recv`         | 接收数据（封装系统recv调用）                                             |
| `Send`         | 发送数据（基础版封装send，预留循环发送逻辑解决半包问题）                 |
| `SetBlock`     | 设置套接字阻塞/非阻塞模式（跨平台实现）                                 |
| `Close`        | 关闭套接字，释放系统资源                                                 |

### 核心特性
1. **跨平台兼容**：通过`_WIN32`宏区分系统，适配Windows/Linux Socket API差异：
   - Windows：依赖`winsock2.h`，使用`SOCKET`句柄、`WSAStartup`初始化；
   - Linux：依赖`sys/socket.h`，使用`int`句柄、`close`关闭套接字。
2. **超时控制**：`Connect`接口基于`select`实现连接超时，支持自定义超时时间；
3. **工程化复用**：核心逻辑编译为动态库/静态库，可直接被其他模块链接使用；
4. **扩展预留**：内置epoll（Linux）、HTTP协议封装目录，支持高并发、应用层协议扩展。

## 快速开始
### 编译环境
- Windows：Visual Studio（支持C++编译、动态库/静态库生成）
- Linux：GCC/G++、make（需安装`libc6-dev`等基础编译库）

### 编译步骤
#### Windows（Visual Studio）
1. 打开项目工程文件（或新建工程导入`src/`下源码）；
2. 将`xsocket/XTcp.cpp`标记为“导出动态库”（生成`xsocket.dll`/`xsocket.lib`）；
3. 分别编译`tcpclient/`、`tcpserver/`下源码，生成可执行文件（依赖`xsocket.lib`）；
4. 编译产物输出到`bin/`、`lib/`目录。

#### Linux
1. 进入项目根目录，编写Makefile（示例核心编译指令）：
   ```makefile
   # 编译XTcp为静态库
   xsocket.a: src/xsocket/XTcp.cpp
       g++ -c src/xsocket/XTcp.cpp -o xsocket.o -fPIC
       ar rcs lib/xsocket.a xsocket.o
   
   # 编译服务端
   tcpserver: src/tcpserver/tcpserver.cpp lib/xsocket.a
       g++ src/tcpserver/tcpserver.cpp -o bin/tcpserver -L./lib -lxsocket -lpthread
   
   # 编译客户端
   tcpclient: src/tcpclient/tcpclient.cpp lib/xsocket.a
       g++ src/tcpclient/tcpclient.cpp -o bin/tcpclient -L./lib -lxsocket -lpthread
   ```
2. 执行`make`编译，产物输出到`bin/`、`lib/`目录。

### 运行示例
#### 服务端
```bash
# Windows
bin/tcpserver.exe 8080  # 监听8080端口

# Linux
./bin/tcpserver 8080
```

#### 客户端
```bash
# Windows
bin/tcpclient.exe 127.0.0.1 8080  # 连接本地8080端口服务端

# Linux
./bin/tcpclient 127.0.0.1 8080
```

## 扩展与优化
### 现有待优化点
1. `Send`接口：启用注释中的“循环发送”逻辑，解决TCP半包数据传输问题；
2. 粘包处理：自定义消息协议（如“消息头+长度+内容”），在XTcp中封装解包/封包逻辑；
3. 高并发：基于`epoll`（Linux）/`IOCP`（Windows）实现IO多路复用，替代单线程`Accept`；
4. 异常处理：增加断连重连、数据校验（CRC）、日志完善等机制。

### 扩展方向
1. HTTP服务：基于XTcp实现简易HTTP服务器，支持静态资源访问、GET/POST请求处理；
2. 多线程/多进程：服务端增加线程池，处理多客户端并发请求；
3. 跨平台编译脚本：编写CMakeLists.txt，统一Windows/Linux编译流程。

## 注意事项
1. Windows平台需先初始化Winsock2库（XTcp构造函数已自动处理）；
2. Linux编译需链接`pthread`库（多线程场景）；
3. 端口号需选择1024以上未被占用的端口，避免权限问题；
4. 非阻塞套接字使用时需注意`Recv/Send`的返回值处理（如返回-1时区分“无数据”和“错误”）。

## 免责声明
本项目为网络编程学习实践项目，仅适用于学习、实验场景，不建议直接用于生产环境。如需生产级使用，需补充完善异常处理、安全校验（如防注入、限流）等机制。
