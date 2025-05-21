
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
# 定义 app1 可执行文件
add_executable(opencv-dbnet-test dbnet.cpp ${SRC_FILES})

target_include_directories(opencv-dbnet-test PRIVATE ${OpenCV_INCLUDE_DIRS} ${ncnn_INCLUDE_DIRS} "../../src" "../../include")
target_link_libraries(opencv-dbnet-test PRIVATE ${OpenCV_LIBS} ncnn)
