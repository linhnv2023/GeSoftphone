# GeSoftphone

<p align="center">
  <img src="res/gesoftphone.png" width="128" height="128" alt="GeSoftphone Logo" />
</p>

<p align="center">
  <b>Cross-Platform VoIP SIP Client for macOS and Linux</b><br>
  Built with C++20, Qt 6, PJSIP 2.14.1 & SQLite 3
</p>

<p align="center">
  <b>Developed by linhnv2</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="C++20" />
  <img src="https://img.shields.io/badge/Qt-6.7%2B-green.svg" alt="Qt 6" />
  <img src="https://img.shields.io/badge/PJSIP-2.14.1-orange.svg" alt="PJSIP" />
  <img src="https://img.shields.io/badge/Audio-Opus%20HD%2048kHz-blueviolet.svg" alt="Opus HD" />
  <img src="https://img.shields.io/badge/Platform-macOS%20%7C%20Linux-lightgrey.svg" alt="Platform" />
  <a href="https://github.com/linhnv2023/GeSoftphone/releases/latest"><img src="https://img.shields.io/github/v/release/linhnv2023/GeSoftphone?color=success&label=Latest%20Release" alt="Latest Release" /></a>
</p>

<p align="center">
  <a href="https://github.com/linhnv2023/GeSoftphone/releases/download/v1.0.0/GESoftphone.dmg">
    <img src="https://img.shields.io/badge/⬇️_Download-GESoftphone.dmg_(macOS)-2563eb?style=for-the-badge&logo=apple&logoColor=white" alt="Download macOS DMG" />
  </a>
  &nbsp;&nbsp;
  <a href="https://github.com/linhnv2023/GeSoftphone/releases/download/v1.0.0/gesoftphone_1.0.0_amd64.deb">
    <img src="https://img.shields.io/badge/⬇️_Download-Ubuntu_.deb_(Linux)-E95420?style=for-the-badge&logo=ubuntu&logoColor=white" alt="Download Ubuntu DEB" />
  </a>
</p>

---

## 📌 Giới thiệu (Overview)

**GeSoftphone** là phần mềm gọi thoại qua giao thức SIP (VoIP client) dành cho hệ điều hành macOS và Linux. Ứng dụng được xây dựng trên nền tảng C++20 và framework Qt 6, sử dụng thư viện PJSIP 2.14.1 để xử lý giao thức SIP và luồng âm thanh RTP.

Dự án cung cấp các chức năng chính:
- **Xử lý cuộc gọi thoại**: Kết nối và đăng ký tới các tổng đài PBX (Asterisk, FreePBX, Kamailio, v.v.), hỗ trợ các chuẩn nén âm thanh Opus (48kHz), G.711a (PCMA), G.711u (PCMU) và G.722.
- **Quản lý tài khoản SIP**: Hỗ trợ lưu trữ nhiều tài khoản SIP; cho phép chuyển đổi nhanh tài khoản hoạt động hoặc chuyển sang trạng thái ngắt kết nối (Offline) bằng 1 cú nhấp chuột.
- **Bàn phím & Điều khiển âm thanh**: Bàn phím số đầy đủ, thanh điều chỉnh âm lượng microphone/loa và hiển thị mức tín hiệu âm thanh (VU meter) thời gian thực.
- **Lưu trữ dữ liệu**: Quản lý lịch sử cuộc gọi và danh bạ lưu trữ cục bộ qua cơ sở dữ liệu SQLite 3.
- **Tương thích hệ điều hành**: Chạy trên macOS (Sonoma, Sequoia) và Linux (Ubuntu 22.04 trở lên).

---

## 📋 Chức năng & Thông số kỹ thuật (Specifications)

### 1. Xử lý âm thanh & Cuộc gọi (Audio & Calls)
- **Audio Codecs**: Opus (48kHz fullband), G.711a/u (8kHz) và G.722 (16kHz).
- **Jitter Buffer**: Sử dụng adaptive jitter buffer tích hợp của PJSIP (40–350ms) để ổn định luồng âm thanh trên mạng độ trễ biến động.
- **Xử lý âm thanh**: Tích hợp Acoustic Echo Cancellation (AEC) và Voice Activity Detection (VAD).
- **Phát chuông**: Phát chuông cuộc gọi độc lập qua tiến trình riêng (`afplay` trên macOS, `paplay`/`canberra` trên Linux) để tránh xung đột với luồng CoreAudio/PulseAudio.

### 2. Quản lý tài khoản SIP (Account Management)
- **Đa tài khoản**: Cấu hình và lưu trữ danh sách tài khoản SIP trên máy.
- **Chuyển đổi trạng thái**: Chuyển đổi tài khoản gọi hoặc ngắt kết nối (Offline) trực tiếp từ thanh trạng thái hoặc menu.
- **Tùy chỉnh mạng SIP**: Hỗ trợ chỉ định SIP Server, Domain, Outbound Proxy (RFC 3261 loose routing), Keep-Alive (UDP/TCP/TLS) và chu kỳ Register Refresh.
- **Bảo mật mật khẩu**: Mật khẩu đã lưu được ẩn trên giao diện; chỉ hiển thị khi nhập mới hoặc khi người dùng chủ động sửa.

### 3. Lưu trữ dữ liệu (Data Persistence)
- **Cơ sở dữ liệu cục bộ**: Dùng SQLite 3 với chế độ Write-Ahead Logging (`WAL`) để lưu lịch sử cuộc gọi và danh bạ.
- **Giới hạn lưu trữ**: Duy trì tối đa 1.000 bản ghi lịch sử gần nhất để tiết kiệm bộ nhớ.

---

## 🏗️ Project Architecture

```
microsip-qt/
├── CMakeLists.txt             # Primary CMake build system & CPack Debian packaging
├── resources.qrc              # Compiled Qt resources (GeSoftphone icons, audio assets)
├── res/                       # High-resolution logos, icons (.icns, .png, .ico)
├── packaging/                 # Production packaging scripts
│   ├── build_mac.sh           # macOS standalone App Bundle & DMG installer builder
│   ├── build_linux.sh         # Linux native .deb package builder
│   ├── build_ubuntu_docker.sh # Isolated Docker packaging script for Ubuntu
│   ├── Dockerfile.ubuntu      # Multi-stage Ubuntu build environment
│   └── linux/gesoftphone.desktop
└── src/
    ├── main.cpp               # Application bootstrap & top-level exception handling
    ├── core/                  # Core VoIP & SIP Engine
    │   ├── SipEngine.cpp/.h   # PJSIP 2.14.1 wrapper (codecs, transports, media)
    │   ├── SipBridge.cpp/.h   # Qt signal/slot event bridge between PJSIP and UI
    │   ├── RingtoneManager.cpp/.h # Detached process ringtone player
    │   └── SipTypes.h         # Core data models (SipAccountConfig, CallInfo)
    ├── data/                  # Persistence Layer
    │   ├── Database.cpp/.h    # SQLite3 connection & table initialization
    │   ├── CallsRepository.cpp/.h # Call log persistence
    │   └── ContactsRepository.cpp/.h # Contacts persistence
    └── ui/                    # Qt 6 User Interface (Apple HIG Design)
        ├── MainWindow.cpp/.h  # Main 5-row dialer, logs table, contacts view
        ├── AccountDialog.cpp/.h # Multi-account manager & password security
        ├── SettingsDialog.cpp/.h # Audio devices, volume & network configuration
        ├── IncomingCallDialog.cpp/.h # Modal call banner (Answer / Decline)
        └── AboutDialog.cpp/.h # Application branding & author credits
```

---

## 🚀 Building & Packaging

### 🍎 1. macOS Build & DMG Creation

#### Prerequisites
- **macOS**: Sonoma 14+ or Sequoia 15+ (Apple Silicon or Intel)
- **Xcode Command Line Tools**: `xcode-select --install`
- **CMake**: `brew install cmake`
- **Qt 6**: Qt 6.7+ or 6.8+ (installed via official Qt Online Installer or Homebrew)
- **PJSIP**: 2.14.1 (headers in `/usr/local/include` or `~/.local/include`)
- **SQLiteCpp**: `brew install sqlitecpp`

#### One-Click Build & Packaging
Run the automated packaging script:
```bash
./packaging/build_mac.sh
```

**Build Output:**
- Standalone Application Bundle: `build_mac/GESoftphone.app`
- Native Disk Image Installer: `build_mac/GESoftphone.dmg`

---

### 🐧 2. Installation & Build on Ubuntu Desktop (Linux)

#### 📦 A. Install Pre-built `.deb` Package (Recommended for End Users)

GeSoftphone provides an official `.deb` package for Ubuntu 22.04 LTS / 24.04 LTS and Debian-based distributions.

##### 1. Download the Debian Package
Download the latest `gesoftphone_1.0.0_amd64.deb` from the [GitHub Releases](https://github.com/linhnv2023/GeSoftphone/releases/latest) page, or via terminal:
```bash
wget https://github.com/linhnv2023/GeSoftphone/releases/download/v1.0.0/gesoftphone_1.0.0_amd64.deb
```

##### 2. Install the Package

* **Method 1: Via Terminal with APT (Recommended - Automatically resolves all dependencies)**:
  ```bash
  sudo apt update
  sudo apt install ./gesoftphone_1.0.0_amd64.deb
  ```

* **Method 2: Via DPKG**:
  ```bash
  sudo dpkg -i gesoftphone_1.0.0_amd64.deb
  sudo apt-get install -f # Fix and install any missing dependencies if needed
  ```

* **Method 3: Graphical Desktop Installation (GUI)**:
  1. Open your **Files** manager and go to **Downloads**.
  2. Right-click on `gesoftphone_1.0.0_amd64.deb` and choose **Open With Software Install** (or double-click).
  3. Click the green **Install** button and authenticate with your user password.

##### 3. Launching on Ubuntu
- **From Application Menu**: Press the <kbd>Super</kbd> (Windows) key on your keyboard, type **GeSoftphone**, and click the application icon.
- **From Terminal**:
  ```bash
  gesoftphone
  ```

##### 4. Uninstalling (if needed)
```bash
sudo apt remove gesoftphone
```

---

#### 🛠️ B. Building from Source on Linux

##### Option 1: Dockerized Isolated Build (Recommended)
Build a production-ready Ubuntu `.deb` package on any machine with Docker:
```bash
./packaging/build_ubuntu_docker.sh
```
*The compiled `.deb` package will be saved in `build_linux/`.*

##### Option 2: Native Linux Build
Install dependencies on Ubuntu 22.04 / 24.04:
```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev libpjproject-dev libsqlite3-dev libsqlite3-0 libasound2-dev
```

Compile and package:
```bash
./packaging/build_linux.sh
```

---

## ⚙️ Quick Start Guide

1. **Launch Application**: Open `GeSoftphone.app` (macOS) or launch `gesoftphone` from your terminal or application launcher (Linux).
2. **Configure Account**:
   - Click the top-right corner dropdown menu (`▼`) and select **Account...** (or click the bottom account status badge).
   - Enter your SIP credentials:
     - **SIP Server**: e.g., `pbx.yourcompany.com`
     - **Username**: e.g., `101`
     - **Domain**: e.g., `pbx.yourcompany.com`
     - **Password**: Your extension password.
   - Click **Save & Apply**.
3. **Make Calls**:
   - Type the recipient's extension or phone number into the Omnibox.
   - Press <kbd>Enter</kbd> or click the green **Call** button.
   - To hang up, click the red **Hang Up** button.

---

## 👤 Author & Credits

- **Developer**: **linhnv2**
- **Project**: **GeSoftphone**
- **Tham khảo**: Thiết kế dựa trên mô hình hoạt động của MicroSIP, được phát triển lại bằng C++20 và Qt 6 cho macOS và Linux.
- **Copyright**: © 2026 GeSoftphone. All rights reserved.
