add_executable(opencv-ncnn-test main.cpp)

target_include_directories(opencv-ncnn-test PRIVATE ${OpenCV_INCLUDE_DIRS} ${ncnn_INCLUDE_DIRS})
target_link_libraries(opencv-ncnn-test PRIVATE ${OpenCV_LIBS} ncnn)