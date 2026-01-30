# 如何运行 libweb 项目

要运行此WebSocket和HTTP服务器演示项目，您需要按照以下步骤操作：

## 系统要求

- Linux 或 macOS 环境
- C 编译器 (GCC 或 Clang)
- CMake
- libwebsockets 开发库

## 安装依赖

### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install libwebsockets-dev cmake build-essential
```

### CentOS/RHEL/Fedora:
```bash
sudo dnf install libwebsockets-devel cmake gcc
```

### macOS:
```bash
brew install libwebsockets cmake
```

## 构建项目

```bash
cd libweb
./build.sh
```

或者手动构建：
```bash
mkdir build
cd build
cmake ..
make
```

## 运行服务器

```bash
./build/libweb_server
```

## 访问应用

1. 启动服务器后，打开浏览器
2. 访问 `http://localhost:8080`
3. 点击 "Connect" 按钮建立WebSocket连接
4. 可以发送消息到服务器，服务器会广播给所有连接的客户端
5. 尝试 "Ping" 按钮查看响应

## 项目特点

- HTTP服务器提供网页内容
- WebSocket服务器实现实时双向通信
- 支持多客户端连接
- 消息广播功能
- 客户端-服务器通信

## 代码结构

- `src/server.c`: C语言WebSocket/HTTP服务器
- `www/index.html`: HTML/JavaScript客户端界面
- `CMakeLists.txt`: 构建配置
- `build.sh`: 自动构建脚本