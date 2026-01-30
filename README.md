# libweb - WebSocket & HTTP 服务器演示项目

一个基于 libwebsockets 库实现的 WebSocket 和 HTTP 服务器演示项目，包含 C 语言服务器端和 HTML/JavaScript 客户端。

## 项目简介

libweb 是一个演示项目，展示了如何使用 libwebsockets 库创建一个同时支持 HTTP 和 WebSocket 协议的服务器。该项目包含：

- **服务器端**：使用 C 语言和 libwebsockets 库实现的 HTTP 和 WebSocket 服务器
- **客户端**：HTML 页面，包含 JavaScript 实现的 WebSocket 客户端
- **实时通信**：支持多客户端连接和消息广播功能
- **用户界面**：友好的 Web 界面，支持消息发送、接收和连接状态显示

## 功能特性

- ✅ HTTP 服务器 - 提供 Web 页面服务
- ✅ WebSocket 服务器 - 支持实时双向通信
- ✅ 多客户端支持 - 可同时处理多个客户端连接
- ✅ 消息广播 - 服务器可将消息广播给所有连接的客户端
- ✅ Ping/Pong 功能 - 测试连接健康状况
- ✅ 响应式界面 - 美观的 Web 界面，实时显示消息

## 系统要求

- Linux 或 macOS 操作系统
- C 编译器 (GCC 或 Clang)
- CMake (可选，也可以直接使用 gcc)
- libwebsockets 开发库

## 安装依赖

### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install libwebsockets-dev build-essential
```

### CentOS/RHEL/Fedora:
```bash
sudo dnf install libwebsockets-devel gcc make
```

### macOS:
```bash
brew install libwebsockets
```

## 构建项目

### 方法一：使用 gcc 直接编译（推荐）
```bash
cd libweb
gcc -I/usr/include -L/usr/lib/aarch64-linux-gnu src/server.c -lwebsockets -o libweb_server
```

### 方法二：使用 CMake
```bash
mkdir build
cd build
cmake ..
make
```

## 运行服务器

```bash
./libweb_server
```

服务器将在端口 8080 上启动 HTTP 和 WebSocket 服务。

## 使用方法

1. 启动服务器：`./libweb_server`
2. 打开浏览器访问：`http://localhost:8080`
3. 点击 "Connect" 按钮建立 WebSocket 连接
4. 在输入框中输入消息并点击 "Send" 发送给服务器
5. 使用 "Ping" 按钮测试服务器响应
6. 服务器会将消息广播给所有连接的客户端

## 项目结构

```
libweb/
├── src/
│   └── server.c          # C语言WebSocket/HTTP服务器源码
├── www/
│   └── index.html        # HTML/JavaScript客户端界面
├── README.md             # 项目说明文档
├── RUNNING.md            # 运行说明文档
├── CMakeLists.txt        # CMake构建配置
└── build.sh              # 自动构建脚本
```

## 技术架构

- **服务器端**: C 语言 + libwebsockets 库
- **协议支持**: HTTP/1.1, WebSocket
- **客户端**: HTML5 + JavaScript WebSocket API
- **通信模式**: 全双工实时通信
- **消息格式**: JSON 格式

## 开发说明

### 服务器端功能
- HTTP 请求处理
- WebSocket 连接管理
- 客户端消息广播
- 连接状态跟踪

### 客户端功能
- WebSocket 连接建立/断开
- 消息发送与接收
- 连接状态显示
- 消息历史记录

## 故障排除

如果遇到编译错误：
1. 确认已安装 libwebsockets 开发库
2. 检查库路径是否正确（某些系统可能是 `/usr/lib/x86_64-linux-gnu`）

如果无法访问页面：
1. 确认服务器正在运行
2. 检查防火墙设置
3. 确认端口 8080 未被占用

## 许可证

MIT License