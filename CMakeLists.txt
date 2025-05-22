# cmake_minimum_required(VERSION 3.10)
# project(opencv)

# set(CMAKE_CXX_STANDARD 11)

# # 设置 OpenCV 路径（可被 -DOpenCV_DIR=xxx 覆盖）
# if(NOT OpenCV_DIR)
#     set(OpenCV_DIR "C:/Users/L3101/Documents/github/ncnn-20250428-windows-vs2022/opencv_mingw")  # 修改为你的路径
# endif()

# find_package(OpenCV REQUIRED)

# # 源文件：递归找所有 .cpp / .c
# file(GLOB_RECURSE SRC_FILES
#     #  "${CMAKE_SOURCE_DIR}/src/dbnet/*.cpp"
#     #  "${CMAKE_SOURCE_DIR}/src/dbnet/*.c"
#     # "${CMAKE_SOURCE_DIR}/src/handwrite/*.c"
#     opencv.cpp
#      )

# add_executable(opencv ${SRC_FILES})

# target_include_directories(opencv PRIVATE ${OpenCV_INCLUDE_DIRS})
# target_link_libraries(opencv PRIVATE ${OpenCV_LIBS})



cmake_minimum_required(VERSION 3.15)
project(handwrite LANGUAGES C CXX)     # 你用到了 .cpp，所以加 CXX

# ---------------------------------------------------------------------------
# 选项：是否启用 Unicode
option(USE_UNICODE "Build with UNICODE/_UNICODE defined" ON)
if(USE_UNICODE)
    add_compile_definitions(UNICODE _UNICODE)
endif()

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--subsystem,console")
endif()
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")

# ---------------------------------------------------------------------------
# 源文件：递归找所有 .cpp / .c
file(GLOB_RECURSE SRC_FILES
    #  "${CMAKE_SOURCE_DIR}/src/dbnet/*.cpp"
    #  "${CMAKE_SOURCE_DIR}/src/crnn/*.cpp"
    # "${CMAKE_SOURCE_DIR}/src/handwrite/*.c"
    "${CMAKE_SOURCE_DIR}/src/write_rec/*.cpp"
    # opencv.cpp
     )

add_executable(handwrite WIN32 ${SRC_FILES})

# -------- C++11 ----------
set_target_properties(handwrite PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

# # -------- Vista+ API -----
# add_compile_definitions(
#     _WIN32_WINNT=0x0600
#     WINVER=0x0600          # 可视窗口/线程同步用到 SRWLOCK
# )

add_definitions(-DNOMINMAX)

# ---------------------------------------------------------------------------
# 设置 OpenCV 路径（可被 -DOpenCV_DIR=xxx 覆盖）
if(NOT OpenCV_DIR)
    set(OpenCV_DIR "C:/Users/L3101/Documents/GitHub/handwrite/opencv_mingw")  # 修改为你的路径
endif()

find_package(OpenCV REQUIRED)

if(NOT ncnn_DIR)
    set(ncnn_DIR "C:/Users/L3101/Documents/GitHub/handwrite/ncnn_w64dev/lib/cmake/ncnn")  # 修改为你的路径
endif()
find_package(ncnn REQUIRED)
message(STATUS "ncnn_DIR = ${ncnn_DIR}")

target_include_directories(handwrite
    PRIVATE                                   # 仅编译本目标时可见
        ${OpenCV_INCLUDE_DIRS}
        # "${CMAKE_SOURCE_DIR}/opencv/include"
        "${CMAKE_SOURCE_DIR}/ncnn_w64dev/include/ncnn"
        ${ncnn_INCLUDE_DIRS}
        "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(handwrite
    PRIVATE                                   # 只给 handwrite 链接
    # "${CMAKE_SOURCE_DIR}/ncnn_w64dev/lib/libncnn.a"
    # ${CMAKE_SOURCE_DIR}/opencv/x64/vc16/lib/opencv_world4100.lib
    # ${CMAKE_SOURCE_DIR}/ncnn/lib/ncnn.lib
    # ${ncnn_LIBS}
    ncnn
    ${OpenCV_LIBS}
)

# ---------------------------------------------------------------------------
# 不同编译器 / 生成器的特定开关
if(MSVC)
    target_compile_options(handwrite PRIVATE /utf-8)
else()  # GNU / Clang (MinGW)
    # -municode & -mwindows 只在 Windows GUI 程序有效
    target_link_options(handwrite PRIVATE
        # $<$<BOOL:${USE_UNICODE}>:-municode>
        -mwindows
    )
endif()
