cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
# CMP0000: Call the cmake_minimum_required() command at the beginning of the top-level
# CMakeLists.txt file even before calling the project() command.
# The cmake_minimum_required(VERSION) command implicitly invokes the cmake_policy(VERSION)
# command to specify that the current project code is written for the given range of CMake
# versions.
project(lxqt-config)

set(LXQT_CONFIG_PROJECT "${PROJECT_NAME}")

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files" OFF)
option(WITH_APPEARANCE "Build the 'lxqt-config-appearance'" ON)
option(WITH_BRIGHTNESS "Build the 'lxqt-config-brightness'" ON)
option(WITH_FILE_ASSOCIATIONS "Build the 'lxqt-config-file-associations'" ON)
option(WITH_INPUT "Build the 'lxqt-config-input'" ON)
option(WITH_LOCALE "Build the 'lxqt-config-locale'" ON)
option(WITH_MONITOR "Build the 'lxqt-config-monitor'" ON)
option(WITH_TOUCHPAD "Build touchpad settings" ON)

include(GNUInstallDirs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pipe -integrated-as -mllvm -polly -mllvm -polly-vectorizer=stripmine -mllvm -polly-invariant-load-hoisting -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-invariant-load-hoisting -flto -mllvm -polly-run-inliner -mllvm -polly-run-dce -fdata-sections -ffunction-sections -mllvm -polly-detect-keep-going -mllvm -polly-ast-use-context -fvisibility=hidden -Wall -Wextra -ferror-limit=0 -Wno-return-type -Wno-unused-parameter -Wno-c++98-compat -Wno-c++98-compat-pedantic -DGLM_FORCE_INTRINSICS -Wno-unicode-whitespace -Wno-format -Wno-unused-function -fmerge-all-constants -D_FILE_OFFSET_BITS=64 -mllvm -inlinehint-threshold=100000 -D_GNU_SOURCE -D__USE_GNU")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pipe -integrated-as -mllvm -polly -mllvm -polly-vectorizer=stripmine -mllvm -polly-invariant-load-hoisting -mllvm -polly-run-inliner -mllvm -polly-run-dce -mllvm -polly-invariant-load-hoisting -flto -mllvm -polly-run-inliner -mllvm -polly-run-dce -fdata-sections -ffunction-sections -mllvm -polly-detect-keep-going -mllvm -polly-ast-use-context -fvisibility=hidden -Wall -Wextra -ferror-limit=0 -Wno-return-type -Wno-unused-parameter -Wno-c++98-compat -Wno-c++98-compat-pedantic -DGLM_FORCE_INTRINSICS -Wno-unicode-whitespace -Wno-format -Wno-unused-function -Wno-vla-cxx-extension -frtti -fmerge-all-constants -D_FILE_OFFSET_BITS=64 -mllvm -inlinehint-threshold=100000 -D_GNU_SOURCE -D__USE_GNU")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -flto -O3 -Wl,--strip-all -fvisibility=hidden -Wl,-O3 -Wl,--gc-sections -Wl,--as-needed -Wl,--exclude-libs,ALL -Wl,--lto-O3")

set(KF6_MINIMUM_VERSION "6.0.0")
set(KF6SCREEN_MINIMUM_VERSION "6.0.0")
set(LXQT_MINIMUM_VERSION "2.4.0")
set(QT_MINIMUM_VERSION "6.6.0")

find_package(Qt6DBus ${QT_MINIMUM_VERSION} REQUIRED)
find_package(Qt6LinguistTools ${QT_MINIMUM_VERSION} REQUIRED)
find_package(Qt6Widgets ${QT_MINIMUM_VERSION} REQUIRED)
find_package(Qt6Xml ${QT_MINIMUM_VERSION} REQUIRED)
find_package(lxqt ${LXQT_MINIMUM_VERSION} REQUIRED)
find_package(lxqt-menu-data ${LXQT_MINIMUM_VERSION} REQUIRED)

if (Qt6Gui_VERSION VERSION_GREATER_EQUAL "6.10.0")
    find_package(Qt6GuiPrivate ${QT_MINIMUM_VERSION} REQUIRED)
endif()

include(LXQtPreventInSourceBuilds)
include(LXQtCompilerSettings NO_POLICY_SCOPE)

# Patch Version
set(LXQT_CONFIG_PATCH_VERSION 0)

set(LXQT_CONFIG_VERSION ${LXQT_MAJOR_VERSION}.${LXQT_MINOR_VERSION}.${LXQT_CONFIG_PATCH_VERSION})
add_definitions(
    "-DLXQT_CONFIG_VERSION=\"${LXQT_CONFIG_VERSION}\""
)

include(LXQtTranslate)

add_subdirectory(src)
if (WITH_INPUT OR WITH_APPEARANCE)
    add_subdirectory(liblxqt-config-cursor)
endif()
if (WITH_INPUT)
    add_subdirectory(lxqt-config-input)
endif()
if (WITH_FILE_ASSOCIATIONS)
    add_subdirectory(lxqt-config-file-associations)
endif()
if (WITH_APPEARANCE)
    add_subdirectory(lxqt-config-appearance)
endif()
if (WITH_MONITOR)
    add_subdirectory(lxqt-config-monitor)
endif()
if (WITH_BRIGHTNESS)
    add_subdirectory(lxqt-config-brightness)
endif()
if (WITH_LOCALE)
    add_subdirectory(lxqt-config-locale)
endif()

install(FILES
    man/lxqt-config.1
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
    COMPONENT Runtime
)
