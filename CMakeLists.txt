cmake_minimum_required(VERSION 3.16)
project(GESoftphone VERSION 1.0.0 LANGUAGES C CXX)

# Prevent CMake from searching for deprecated AGL framework on macOS
set(OPENGL_agl_LIBRARY "" CACHE FILEPATH "" FORCE)
set(WrapOpenGL_AGL "" CACHE FILEPATH "" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


# Find Qt 6 (or Qt 5 fallback)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

# Find PJSIP
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PJSIP libpjproject)
endif()

# Fallback paths for PJSIP if pkg-config not used
if(NOT PJSIP_FOUND)
    find_path(PJSIP_INCLUDE_DIRS NAMES pjsua-lib/pjsua.h PATHS /usr/local/include /opt/homebrew/include $ENV{HOME}/.local/include)
    find_library(PJSIP_LIB_PJSUA NAMES pjsua PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)
    find_library(PJSIP_LIB_PJSIP_UA NAMES pjsip-ua PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)
    find_library(PJSIP_LIB_PJSIP_SIMPLE NAMES pjsip-simple PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)
    find_library(PJSIP_LIB_PJSIP NAMES pjsip PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)
    find_library(PJSIP_LIB_PJMEDIA_CODEC NAMES pjmedia-codec PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)
    find_library(PJSIP_LIB_PJMEDIA_AUDIODEV NAMES pjmedia-audiodev PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)
    find_library(PJSIP_LIB_PJMEDIA NAMES pjmedia PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)
    find_library(PJSIP_LIB_PJNATH NAMES pjnath PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)
    find_library(PJSIP_LIB_PJLIB_UTIL NAMES pjlib-util PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)
    find_library(PJSIP_LIB_PJLIB NAMES pj pjlib PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)
    
    set(PJSIP_LIBRARIES 
        ${PJSIP_LIB_PJSUA} 
        ${PJSIP_LIB_PJSIP_UA}
        ${PJSIP_LIB_PJSIP_SIMPLE}
        ${PJSIP_LIB_PJSIP} 
        ${PJSIP_LIB_PJMEDIA_CODEC}
        ${PJSIP_LIB_PJMEDIA_AUDIODEV}
        ${PJSIP_LIB_PJMEDIA} 
        ${PJSIP_LIB_PJNATH} 
        ${PJSIP_LIB_PJLIB_UTIL} 
        ${PJSIP_LIB_PJLIB}
    )
    
    foreach(opt_lib srtp resample gsmcodec speex ilbccodec g7221codec webrtc)
        find_library(PJSIP_OPT_${opt_lib} NAMES ${opt_lib} PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)
        if(PJSIP_OPT_${opt_lib})
            list(APPEND PJSIP_LIBRARIES ${PJSIP_OPT_${opt_lib}})
        endif()
    endforeach()
endif()


# Find SQLite3
find_package(SQLite3 REQUIRED)

# SQLiteCpp headers and library
find_path(SQLITECPP_INCLUDE_DIR NAMES SQLiteCpp/SQLiteCpp.h PATHS /usr/local/include /opt/homebrew/include $ENV{HOME}/.local/include)
find_library(SQLITECPP_LIBRARY NAMES SQLiteCpp PATHS /usr/local/lib /opt/homebrew/lib $ENV{HOME}/.local/lib)

# Source files
set(SOURCES
    src/main.cpp
    src/core/SipEngine.cpp
    src/core/SipBridge.cpp
    src/core/RingtoneManager.cpp
    src/data/Database.cpp
    src/data/CallsRepository.cpp
    src/data/ContactsRepository.cpp
    src/ui/MainWindow.cpp
    src/ui/SettingsDialog.cpp
    src/ui/AccountDialog.cpp
    src/ui/IncomingCallDialog.cpp
    src/ui/ContactDialog.cpp
    src/ui/TransferDialog.cpp
    src/ui/AboutDialog.cpp
    src/ui/MessageDialog.cpp
    resources.qrc
)

set(HEADERS
    src/domain/Call.h
    src/domain/Contact.h
    src/core/SipTypes.h
    src/core/SipEngine.h
    src/core/SipBridge.h
    src/core/RingtoneManager.h
    src/data/ICallsRepository.h
    src/data/IContactsRepository.h
    src/data/Database.h
    src/data/CallsRepository.h
    src/data/ContactsRepository.h
    src/ui/MainWindow.h
    src/ui/SettingsDialog.h
    src/ui/AccountDialog.h
    src/ui/IncomingCallDialog.h
    src/ui/ContactDialog.h
    src/ui/TransferDialog.h
    src/ui/AboutDialog.h
    src/ui/MessageDialog.h
)

add_executable(gesoftphone ${SOURCES} ${HEADERS})

target_include_directories(gesoftphone PRIVATE
    src/
    ${PJSIP_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    ${SQLITECPP_INCLUDE_DIR}
)

target_link_directories(gesoftphone PRIVATE
    /usr/local/lib
    /usr/local/opt/openssl/lib
    $ENV{HOME}/.local/lib
)

target_link_libraries(gesoftphone PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    ${PJSIP_LIBRARIES}
    SQLite::SQLite3
    ${SQLITECPP_LIBRARY}
    ssl
    crypto
    opus
    opencore-amrnb
    opencore-amrwb
)


if(APPLE)
    target_link_libraries(gesoftphone PRIVATE
        "-framework CoreAudio"
        "-framework CoreServices"
        "-framework AudioUnit"
        "-framework AudioToolbox"
        "-framework Foundation"
        "-framework AppKit"
        "-framework AVFoundation"
        "-framework CoreMedia"
        "-framework Security"
        "-framework SystemConfiguration"
        "-framework Network"
    )

    set(MACOSX_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/GESoftphone.icns")
    set_source_files_properties(${MACOSX_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(gesoftphone PRIVATE ${MACOSX_ICON})

    set_target_properties(gesoftphone PROPERTIES
        OUTPUT_NAME "GESoftphone"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "GESoftphone"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.ge.softphone"
        MACOSX_BUNDLE_ICON_FILE "GESoftphone.icns"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "3.22.16"
        MACOSX_BUNDLE_BUNDLE_VERSION "3.22.16"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/Info.plist.in"
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(gesoftphone PRIVATE
        pthread
        asound
    )

    install(TARGETS gesoftphone DESTINATION bin)
    install(FILES packaging/linux/gesoftphone.desktop DESTINATION share/applications)
    install(FILES res/gesoftphone.png DESTINATION share/icons/hicolor/512x512/apps)
endif()

# CPack configuration for Debian / Ubuntu .deb package
set(CPACK_PACKAGE_NAME "gesoftphone")
set(CPACK_PACKAGE_VENDOR "GeSoftphone")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GeSoftphone - Modern VoIP SIP Client")
set(CPACK_PACKAGE_VERSION "3.22.16")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "GeSoftphone Team <support@ge.softphone>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libqt6core6, libqt6gui6, libqt6widgets6, libpjproject2 | libpjproject-dev, libsqlite3-0, libasound2")
set(CPACK_DEBIAN_PACKAGE_SECTION "net")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/ge-softphone")
include(CPack)



