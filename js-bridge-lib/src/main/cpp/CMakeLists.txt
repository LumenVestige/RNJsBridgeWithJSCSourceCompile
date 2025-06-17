# Copyright (c) Facebook, Inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.6.0)
project(jsbridge LANGUAGES C CXX)


#-Wall:
#启用所有常规的警告消息。这有助于发现潜在的代码问题。
#-std=c++14:
#指定 C++ 标准为 C++14。确保编译器使用 C++14 标准进行编译。
#-pthread:
#启用多线程支持，并链接 pthread 库。这对于使用线程的程序是必需的。
#-fno-omit-frame-pointer:
#不省略帧指针。这对调试和性能分析工具（如 gdb 和 perf）有帮助。
#-fexceptions:
#启用 C++ 异常处理支持。允许在代码中使用 try, catch, throw。
#-frtti:
#启用运行时类型识别 (RTTI)。允许使用 C++ 的 typeid 和 dynamic_cast 操作符。
#-ffunction-sections:
#将每个函数或数据项放入单独的段中。这可以在链接时实现更好的优化和裁剪未使用的代码。
#-O3:
#启用最高级别的优化选项。这会生成更快的代码，但可能会使编译时间更长。
#-DNDEBUG:
#定义 NDEBUG 宏，通常用于禁用调试相关的断言 (assert)。
set(FOLLY_FLAGS "-DFOLLY_NO_CONFIG -DFOLLY_HAVE_CLOCK_GETTIME -DFOLLY_HAVE_MEMRCHR -DFOLLY_USE_LIBCPP -DFOLLY_NO_CONFIG=1 -DFOLLY_HAVE_XSI_STRERROR_R=1 -DENABLE_FBASSERT")

# 为 C++ 编译器添加 -v 标志
set(CMAKE_CXX_FLAGS "${FOLLY_FLAGS} -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_STRIP "")


set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_VERBOSE_MAKEFILE ON)


add_compile_options(
        -w # 关闭所有警告
        -Wall
        -pthread
        -fno-omit-frame-pointer
        -fexceptions
        -frtti
        -ffunction-sections
        -O0 # just for debug
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("💡 CMAKE_CXX_STANDARD = ${CMAKE_CXX_STANDARD}")

include_directories(icu/include)
include_directories(glog/glog/exported)
include_directories(fbgloginit)
include_directories(folly)
include_directories(boost/boost_1_63_0)
include_directories(jscexecutor)
include_directories(bridge/include)
include_directories(jsc/webkit/Source/JavaScriptCore/API)
include_directories(double_conversion)
include_directories(trace)

add_subdirectory(trace)
add_subdirectory(icu)
add_subdirectory(jsc)
add_subdirectory(cxxmodule)
add_subdirectory(jscexecutor)
add_subdirectory(boost)
add_subdirectory(folly)
add_subdirectory(fbgloginit)
add_subdirectory(glog/glog)
add_subdirectory(bridge)
add_subdirectory(fbjni)
add_subdirectory(double_conversion/double_conversion)