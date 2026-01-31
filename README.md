# libweb - MEC 智慧路口监控系统

基于 libwebsockets 实现的实时交通信号灯监控演示项目，包含 C 语言服务器端和现代化 Web 前端。

![Traffic Light Demo](https://img.shields.io/badge/WebSocket-实时通信-brightgreen) ![C](https://img.shields.io/badge/Language-C-blue) ![libwebsockets](https://img.shields.io/badge/Library-libwebsockets-orange)

## ✨ 功能特性

- 🚦 **实时交通灯模拟** - 南北/东西方向信号灯自动切换
- ⚡ **WebSocket 实时推送** - 每秒更新倒计时数据
- 🎨 **现代化 UI** - 深色主题、发光特效、平滑动画
- 📊 **状态监控面板** - 连接状态、消息计数、实时日志
- 🔄 **自动重连** - 断线后自动尝试重新连接

## 📋 系统要求

- Linux / macOS / WSL2
- GCC 编译器
- libwebsockets 开发库
- CMake 3.10+

## 🚀 快速开始

### 1. 安装依赖

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y libwebsockets-dev build-essential cmake pkg-config
```

**CentOS/Fedora:**
```bash
sudo dnf install libwebsockets-devel gcc make cmake
```

**macOS:**
```bash
brew install libwebsockets cmake
```

### 2. 编译项目

```bash
./build.sh
```

或手动编译：
```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### 3. 运行服务器

```bash
./build/libweb_server
```

### 4. 访问页面

打开浏览器访问：**http://localhost:8080**

## 📁 项目结构

```
libweb/
├── src/
│   └── server.c          # C语言 WebSocket/HTTP 服务器
├── www/
│   └── index.html        # 前端页面 (HTML5 + CSS3 + JS)
├── build.sh              # 一键编译脚本
├── CMakeLists.txt        # CMake 配置
└── README.md
```

## 🔧 技术架构

| 组件 | 技术 |
|------|------|
| 服务器 | C + libwebsockets 4.x |
| 协议 | HTTP/1.1 + WebSocket |
| 前端 | HTML5 + CSS3 + Vanilla JS |
| 通信格式 | JSON |
| WebSocket 协议 | `traffic-protocol` |

## 📡 WebSocket 消息格式

服务器每秒推送一次交通灯状态：

```json
{
  "type": "traffic_update",
  "ns": { "color": "green", "timer": 25 },
  "ew": { "color": "red", "timer": 28 }
}
```

- `ns` - 南北方向状态
- `ew` - 东西方向状态
- `color` - 当前颜色 (red/yellow/green)
- `timer` - 倒计时秒数

## 🎯 信号灯逻辑

- 绿灯：30秒
- 黄灯：3秒
- 红灯：30秒
- 南北/东西方向交替运行

## 🐛 故障排除

**编译失败：找不到 libwebsockets**
```bash
# 确认已安装开发库
pkg-config --libs libwebsockets
```

**页面无法自动更新**
- 按 `Ctrl+Shift+R` 强制刷新清除缓存
- 检查浏览器控制台是否有 WebSocket 连接错误

**端口被占用**
```bash
# 查看 8080 端口占用
lsof -i :8080
```

## 📄 License

MIT License
