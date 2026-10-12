# Mandatory line, sets the minimum version of CMake that should be used with this repository.
# 3.22 is the minimum CMake version required by JUCE.
# 3.25 is required for the MSVC_DEBUG_INFORMATION_FORMAT feature to work.
# To verify your version run
# $ cmake --version
# 设定CMake最低版本，JUCE8要求至少3.22；CMP0141策略需要3.25
cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

# Enable setting CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
# CMP0141：控制 MSVC_DEBUG_INFORMATION_FORMAT 策略，
# 用于配置MSVC调试信息格式，设置为NEW启用新行为
set(CMAKE_POLICY_DEFAULT_CMP0141        NEW      CACHE STRING "" FORCE)

# Sets the minimum deployment target
# macOS 10.11 is the minimum required by JUCE
# macOS部署目标最低版本，JUCE支持的最低macOS版本
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "" FORCE)
# Builds a binary that works on Apple Silicon (arm64) and Intel-based Macs
# macOS同时编译支持x86_64(Intel)与arm64(Apple Silicon M系列)双架构
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" FORCE)

# 项目名称
project(juce_315_bandpass_plugin)

# 如果是MSVC(Windows编译器)，添加编译选项 /utf‑8，源码文件全部按照UTF‑8解析，避免中文注释乱码
if(MSVC)
    add_compile_options(/utf-8)
    #    add_compile_options(/P)    # 预处理器输出，调试用，注释关闭
endif()

# Set the C++ standard only if this project is standalone;
# otherwise, let clients decide.
# PROJECT_IS_TOP_LEVEL：判断当前CMakeLists是不是最顶层项目，不是被别人add_subdirectory引入
if(PROJECT_IS_TOP_LEVEL)
    if (NOT CMAKE_CXX_STANDARD)
        # Always use the newest C++ standard on green-field projects if possible.
        # C++23标准，现代JUCE推荐
        set(CMAKE_CXX_STANDARD 23)
    endif()

    # Links to C++ runtime statically
    # Windows MSVC：C++运行库静态链接。
    # MultiThreaded = Release静态MT；$<$<CONFIG:Debug>:Debug> Debug模式使用MTd。
    # 好处：编译出来插件不依赖系统安装的VS运行库dll，拷贝到别的电脑可以直接运行。
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    # Embeds debug information
    # MSVC调试信息嵌入obj文件，不需要单独.pdb外部文件
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>" CACHE STRING "" FORCE)

    # 警告直接当作错误，编译失败，强制写规范代码，项目规范
    set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
endif()

# 根据Debug/Release设置插件版本号
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PLUGIN_VERSION "0.0.0")
else ()
    set(PLUGIN_VERSION "0.0.1")
endif ()

# I like to download the dependencies to the same folder as the project.
# If you want to install them system-wide, set CPM_SOURCE_CACHE with a path to the dependencies
# either as an environment variable or pass it with -DCPM_SOURCE_CACHE=<path> when invoking cmake.
# 设置第三方库存放目录：项目根目录下libs文件夹，CPM下载的库全部放这里
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

# Downloads CPM if not already downloaded.
# CPM is an easy-to-use package manager nicely integrated with CMake.
# 引入cpm.cmake脚本，CPM是CMake的包管理器，自动下载管理第三方库
# 注意：要先下载 cpm.cmake 到 项目根目录下的cmake目录中
include(cmake/cpm.cmake)

# This commands downloads AND configures JUCE.
# It sets up some variables, like JUCE_SOURCE_DIR.
# CPM下载JUCE
cpmaddpackage(
    NAME
        JUCE
    GIT_TAG
        8.0.15      # 指定Git标签，锁定JUCE版本为8.0.15，保证所有人编译环境完全一致
    GITHUB_REPOSITORY
        juce-framework/JUCE
    SOURCE_DIR
        ${LIB_DIR}/juce     # 下载解压到项目libs/juce目录
)

# Adds compiler warning utilities & other.
# 引入编译器警告配置脚本，开启大量严格C++警告
include(cmake/CompilerWarnings.cmake)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Sets the Windows VST3 installation location to one
    # that doesn't require elevated permissions
    # Windows VST3自动复制目录：复制到%LOCALAPPDATA%\Programs\Common\VST3
    # 这个路径不需要管理员权限，DAW扫描可以直接读到编译出来的VST3插件
    set_directory_properties(PROPERTIES JUCE_VST3_COPY_DIR "$ENV{LOCALAPPDATA}/Programs/Common/VST3")
endif()

# Adds a plugin target.
# 插件显示名字
set(JUCE_315_PLUGIN_NAME "OmgBandPass")

# ================= JUCE核心：juce_add_plugin 定义插件目标 =================
# juce_addd_plugin：生成可执行文件/插件 目标target
juce_add_plugin(
    OmgBandPassPlugin           # 创建可执行的最终CMake Target，并命名为OmgBandPassPlugin
    PRODUCT_NAME
        ${JUCE_315_PLUGIN_NAME}     # DAW里显示的插件名字 Tremolo
    VERSION
        ${PLUGIN_VERSION} # 0 forces AU hosts to always refresh
    COMPANY_NAME
        "InnoWisdom"                # 厂商名称
    PLUGIN_MANUFACTURER_CODE
        InWs                        # 厂商4字符ID
    PLUGIN_CODE
        Ogbp                        # 插件唯一4字符ID，每个插件不能重复
    FORMATS
        Standalone                  # 独立可执行程序，调试用
        AU                          # macOS AU插件格式
        VST3                        # VST3跨平台插件格式
    IS_SYNTH
        FALSE                       # 不是合成器，是效果器
    NEEDS_MIDI_INPUT
        FALSE                       # 不需要MIDI输入
    NEEDS_MIDI_OUTPUT
        FALSE                       # 不需要MIDI输出
    VST3_AUTO_MANIFEST
        TRUE                        # VST3自动生成manifest清单文件
    COPY_PLUGIN_AFTER_BUILD
        TRUE                        # ✅编译完成自动复制VST3到上面设置的VST3目录，DAW直接加载，不用手动拷贝
)

# Configures the JUCE sources included in the plugin target.
# 对插件目标（即上面juce_add_plugin定义的目标）添加编译宏
# 编译宏定义：关闭不需要的JUCE组件
target_compile_definitions(OmgBandPassPlugin
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# ========= juce_add_binary_data：二进制资源嵌入 =========
# 把图片、字体等资源编译进程序二进制内部，不需要外部资源文件
juce_add_binary_data(bandpass_plugin_assets
    HEADER_NAME
        "BandpassPluginAssets.h"
    NAMESPACE
        bandpass::assets
    SOURCES
        assets/Inter-Medium.ttf
        assets/Inter-Bold.ttf
)
add_library(bandpass::bandpass_plugin_assets ALIAS bandpass_plugin_assets)

# juce_add_module：创建一个module target，这是一个逻辑target
# JUCE模块化，把插件业务代码封装成JUCE Module
# 模块名称（target目标名称）同时也是模块所在文件夹的名称（JUCE约定）
juce_add_module(bandpass_plugin ALIAS_NAMESPACE bandpass)

# Additional definitions required by all users of this module.
# 给上面定义的模块目标添加编译宏
# 模块对外暴露编译宏
target_compile_definitions(bandpass_plugin
    INTERFACE
        JUCE_315_PLUGIN_NAME="${JUCE_315_PLUGIN_NAME}"
)

# Enables strict C++ warnings.
# 继续给模块目标添加编译选项：启用严格警告
target_compile_options(bandpass_plugin INTERFACE "${PROJECT_WARNINGS_CXX}")

# 继续给模块目标添加链接：链接二进制资源库
target_link_libraries(bandpass_plugin INTERFACE bandpass::bandpass_plugin_assets)

# 最终插件链接：业务模块 + JUCE官方推荐编译标志
target_link_libraries(
    OmgBandPassPlugin
    PRIVATE
        bandpass::bandpass_plugin
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)



# In Visual Studio this command provides a nice grouping of source files
# in "filters".
# source_group：在Visual Studio中把源文件做树形分组，CLion不生效
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# 单元测试开关，默认关闭；打开会自动下载GTest做自动化单元测试
option(BUILD_TESTS "When on, will download the GoogleTest framework and add the automated tests project" OFF)
if(BUILD_TESTS)
    # This command allows running tests from the "build" folder (the one where CMake generates the project to).
    enable_testing()

    # Adds all the targets configured in the "test" folder.
    add_subdirectory(test)
endif()