#!/bin/bash
set -e

echo "=== Building GeSoftphone for macOS ==="

# 1. Check CMake
if ! command -v cmake &> /dev/null; then
    if [ -x "/usr/local/CMake.app/Contents/bin/cmake" ]; then
        ln -sf /usr/local/CMake.app/Contents/bin/cmake /usr/local/bin/cmake 2>/dev/null || true
    fi
fi
echo "✓ CMake ready: $(cmake --version | head -n 1)"

# 2. Check Qt 6 (Fast pre-built binary via aqt into $HOME/Qt)
QT_DIR=""
if [ -d "$HOME/Qt/6.7.3/macos/lib/QtCore.framework" ]; then
    QT_DIR="$HOME/Qt/6.7.3/macos"
elif [ -d "/usr/local/opt/qt" ]; then
    QT_DIR="/usr/local/opt/qt"
elif [ -d "/usr/local/opt/qt@6" ]; then
    QT_DIR="/usr/local/opt/qt@6"
else
    echo "Installing pre-built Qt 6 into $HOME/Qt (fast, no compilation needed)..."
    AQT_BIN=$(command -v aqt || echo "$HOME/Library/Python/3.9/bin/aqt")
    if [ ! -x "$AQT_BIN" ]; then
        pip3 install aqtinstall
        AQT_BIN=$(command -v aqt || echo "$HOME/Library/Python/3.9/bin/aqt")
    fi
    mkdir -p "$HOME/Qt"
    "$AQT_BIN" install-qt mac desktop 6.7.3 clang_64 -m qtbase -O "$HOME/Qt" || true
    QT_DIR="$HOME/Qt/6.7.3/macos"
    
    # Fix macOS SDK AGL deprecated framework removal in Qt 6 FindWrapOpenGL
    WRAP_GL="${QT_DIR}/lib/cmake/Qt6/FindWrapOpenGL.cmake"
    if [ -f "$WRAP_GL" ]; then
        sed -i '' 's/set(__opengl_agl_fw_path "-framework AGL")/# set(__opengl_agl_fw_path "-framework AGL")/g' "$WRAP_GL" 2>/dev/null || true
    fi
fi
echo "✓ Qt 6 ready at: ${QT_DIR}"

LOCAL_DIR="$HOME/.local"
mkdir -p "${LOCAL_DIR}/include" "${LOCAL_DIR}/lib"

# 3. Check SQLiteCpp
if [ ! -f "${LOCAL_DIR}/include/SQLiteCpp/SQLiteCpp.h" ] && [ ! -f "/usr/local/include/SQLiteCpp/SQLiteCpp.h" ]; then
    echo "Installing SQLiteCpp into ${LOCAL_DIR}..."
    TMP_SQLITE="/tmp/SQLiteCpp_build"
    rm -rf "$TMP_SQLITE"
    git clone --depth 1 https://github.com/SRombauts/SQLiteCpp.git "$TMP_SQLITE"
    cd "$TMP_SQLITE"
    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DSQLITECPP_RUN_CPPLINT=OFF -DCMAKE_INSTALL_PREFIX="${LOCAL_DIR}"
    cmake --build . -j$(sysctl -n hw.ncpu)
    cmake --install .
    cd - > /dev/null
    rm -rf "$TMP_SQLITE"
fi
echo "✓ SQLiteCpp ready."

# 4. Check PJSIP (pjproject)
if [ ! -f "${LOCAL_DIR}/include/pjsua-lib/pjsua.h" ] && [ ! -f "/usr/local/include/pjsua-lib/pjsua.h" ]; then
    echo "Downloading and building PJSIP into ${LOCAL_DIR}..."
    TMP_PJ="/tmp/pjsip_build"
    rm -rf "$TMP_PJ"
    git clone --depth 1 --branch 2.14.1 https://github.com/pjsip/pjproject.git "$TMP_PJ"
    cd "$TMP_PJ"
    ./configure --disable-video --enable-sound --disable-speex-aec --prefix="${LOCAL_DIR}"
    make dep
    make -j$(sysctl -n hw.ncpu)
    make install
    cd - > /dev/null
    rm -rf "$TMP_PJ"
fi
echo "✓ PJSIP ready."

# 5. Build GeSoftphone App
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_ROOT}/build_mac"

# 5.1 Ensure macOS icon (.icns) is compiled
if [ ! -f "${PROJECT_ROOT}/res/GESoftphone.icns" ]; then
    echo "Creating macOS application icon (GESoftphone.icns)..."
    python3 -c "
import os
from PIL import Image

src_path = '${PROJECT_ROOT}/res/gesoftphone.png'
base_img = Image.open(src_path).convert('RGBA')
iconset_dir = '${PROJECT_ROOT}/res/GESoftphone.iconset'
os.makedirs(iconset_dir, exist_ok=True)

sizes = [
    ('icon_16x16.png', (16, 16)),
    ('icon_16x16@2x.png', (32, 32)),
    ('icon_32x32.png', (32, 32)),
    ('icon_32x32@2x.png', (64, 64)),
    ('icon_128x128.png', (128, 128)),
    ('icon_128x128@2x.png', (256, 256)),
    ('icon_256x256.png', (256, 256)),
    ('icon_256x256@2x.png', (512, 512)),
    ('icon_512x512.png', (512, 512)),
    ('icon_512x512@2x.png', (1024, 1024)),
]

for filename, size in sizes:
    resized = base_img.resize(size, Image.Resampling.LANCZOS)
    resized.save(os.path.join(iconset_dir, filename))
"
    iconutil -c icns "${PROJECT_ROOT}/res/GESoftphone.iconset" -o "${PROJECT_ROOT}/res/GESoftphone.icns"
fi

echo "Building GeSoftphone in ${BUILD_DIR}..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake "${PROJECT_ROOT}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="${QT_DIR};${LOCAL_DIR}"

cmake --build . --config Release -j$(sysctl -n hw.ncpu)

# 6. Deploy Qt dependencies into the App Bundle
echo "Deploying Qt libraries into App Bundle..."
"${QT_DIR}/bin/macdeployqt" "${BUILD_DIR}/GESoftphone.app"

# 6.1 Install application icon into App Bundle
echo "Installing application icon into App Bundle..."
mkdir -p "${BUILD_DIR}/GESoftphone.app/Contents/Resources"
cp -f "${PROJECT_ROOT}/res/GESoftphone.icns" "${BUILD_DIR}/GESoftphone.app/Contents/Resources/GESoftphone.icns"

# Ensure CFBundleIconFile is properly set in Info.plist
PLIST="${BUILD_DIR}/GESoftphone.app/Contents/Info.plist"
if [ -f "$PLIST" ]; then
    /usr/libexec/PlistBuddy -c "Set :CFBundleIconFile GESoftphone.icns" "$PLIST" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Add :CFBundleIconFile string GESoftphone.icns" "$PLIST" 2>/dev/null || true
fi

# Touch app bundle so macOS Finder immediately registers the new icon
touch "${BUILD_DIR}/GESoftphone.app"

# 7. Create DMG Package
echo "Creating DMG installer..."
hdiutil create -volname "GESoftphone" -srcfolder "${BUILD_DIR}/GESoftphone.app" -ov -format UDZO "${BUILD_DIR}/GESoftphone.dmg"

echo "============================================="
echo " 🎉 GeSoftphone Built Successfully!"
echo " App Bundle: ${BUILD_DIR}/GESoftphone.app"
echo " DMG Installer: ${BUILD_DIR}/GESoftphone.dmg"
echo "============================================="
