# GeSoftphone

<p align="center">
  <img src="res/gesoftphone.png" width="128" height="128" alt="GeSoftphone Logo" />
</p>

<p align="center">
  <b>Modern, High-Performance Cross-Platform Enterprise VoIP SIP Client</b><br>
  Designed for macOS and Linux • Built with C++20, Qt 6, PJSIP 2.14.1 & SQLite 3
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
</p>

---

## 📌 Overview

**GeSoftphone** is a modern, ultra-compact, enterprise-grade desktop VoIP softphone engineered for macOS and Linux. Building upon the proven lightweight concepts of MicroSIP, GeSoftphone has been completely re-architected and modernized from the ground up with C++20, Qt 6, and PJSIP 2.14.1. Combining studio-grade wideband audio algorithms with Apple Human Interface (HIG) ergonomics, it provides lightning-fast call handling, multi-account registration, and rock-solid network resilience for businesses, call centers, and power users.

---

## ✨ Key Features

### 🎧 Audio & Telephony Engine
- **Opus HD 48kHz Fullband**: Native 48kHz studio audio without downsampling or conversion distortion.
- **Adaptive Jitter Buffer**: Dynamic 40–350ms prefetch buffer preventing robot-voice and audio dropouts across Wi-Fi/4G.
- **Hardware DSP Integration**: Multi-band Acoustic Echo Cancellation (AEC) and Voice Activity Detection (VAD).
- **Comprehensive Codec Support**: Opus (48kHz), G.711a (PCMA - 8kHz), G.711u (PCMU - 8kHz), and G.722 (16kHz).
- **Real-time Live Audio VU Meters**: Dual input/output VU meters for real-time visual signal telemetry during calls.
- **Asynchronous Ringtone Engine**: Multi-threaded audio playback (`afplay` on macOS, `paplay`/`canberra` on Linux) guaranteeing zero audio stream collisions with the SIP CoreAudio engine.

### 👥 Advanced Multi-Account Management
- **Multiple SIP Accounts**: Store and manage unlimited SIP accounts simultaneously.
- **1-Click Active Switching**: Instantly switch the active calling account from the status bar badge or main menu without restarting.
- **Granular Account Settings**: Custom SIP Proxy with RFC 3261 loose routing (`lr`), Keep-Alive (UDP/TCP/TLS), and Register Refresh timeouts.
- **Strict Password Protection**:
  - Saved accounts automatically mask and protect passwords (`🔒 Password saved & protected`).
  - Plain-text verification is only available during new account creation or when typing an uncommitted password replacement.

### 🛡️ Enterprise Networking & Security
- **SRTP Media Encryption**: Optional or Mandatory end-to-end media encryption via Secure Real-time Transport Protocol.
- **SIP Transports**: Dual UDP and TCP support with RFC 3261 loose-routing traversal and automatic MTU packet protection.
- **NAT Traversal**: Built-in STUN client, ICE support, and dynamic Via/Contact rport rewriting.
- **Live Status Telemetry**: Instant display of SIP response codes (e.g. `401 Unauthorized`, `403 Forbidden`, `408 Request Timeout`) in the status bar and tooltip.
- **Active Call Guard**: Prevents accidental account switching or disruption while a live conversation is underway.

### ⚡ Data Persistence & Performance
- **Zero-Latency SQLite Database**: Uses SQLite 3 in Write-Ahead Logging (`WAL`) mode with `NORMAL` synchronous commits.
- **Indexed Search**: Real-time auto-completion across call history and contact book.
- **Smart Pruning**: Automatic history maintenance capped at 1,000 entries to maintain a minimal memory footprint.

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

### 🐧 2. Linux / Ubuntu Build (.deb Package)

#### Option A: Dockerized Isolated Build (Recommended)
Build a production-ready Ubuntu `.deb` package on any machine with Docker installed:
```bash
./packaging/build_ubuntu_docker.sh
```
*The compiled `.deb` package will be saved in `build_linux/`.*

#### Option B: Native Linux Build
Install dependencies on Ubuntu 22.04 / 24.04:
```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev libpjproject-dev libsqlite3-dev libsqlite3-0 libasound2-dev
```

Compile and package:
```bash
./packaging/build_linux.sh
```

Install the package:
```bash
sudo dpkg -i build_linux/gesoftphone_3.22.16_amd64.deb
sudo apt-get install -f # Fix any missing dependencies if needed
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
- **Heritage**: Inherits core architectural ideas from MicroSIP, completely reimagined for modern cross-platform environments.
- **Copyright**: © 2026 GeSoftphone. All rights reserved.
