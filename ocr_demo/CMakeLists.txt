cmake_minimum_required(VERSION 3.4.1)

project(ocr_demo_linux)

set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (CMAKE_C_COMPILER MATCHES "aarch64")
  set(LIB_ARCH lib64)
else()
  set(LIB_ARCH lib)
endif()

# include
include_directories(${CMAKE_SOURCE_DIR}/include)

# rknn api
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/librknn_api)
include_directories(${RKNN_API_PATH}/include)
set(RKNN_API_LIB ${RKNN_API_PATH}/${LIB_ARCH}/librknn_api.so)

# opencv
# set(OPENCV_PATH ${CMAKE_SOURCE_DIR}/3rdparty/opencv)
# include_directories(${OPENCV_PATH}/include)
# set(OPENCV_LIB ${OPENCV_PATH}/${LIB_ARCH}/libopencv_core.a
#                 ${OPENCV_PATH}/${LIB_ARCH}/libopencv_imgcodecs.a
#                 ${OPENCV_PATH}/${LIB_ARCH}/libopencv_imgproc.a
# )
set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/3rdparty/opencv/opencv-linux-armhf/share/OpenCV)
find_package(OpenCV REQUIRED)

# #stb
# include_directories(${CMAKE_SOURCE_DIR}/3rdparty/)

set(CMAKE_INSTALL_RPATH "lib")

add_executable(ocr_demo
        src/main.cpp
        src/ocr_det.cpp
        src/ocr_rec.cpp
        src/postprocess_op.cpp
        src/preprocess_op.cpp
        src/utility.cpp
        include/clipper.cpp
        )

target_link_libraries(ocr_demo
	${RKNN_API_LIB}
        ${OpenCV_LIBS}
	dl
)

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/ocr_demo)
install(TARGETS ocr_demo DESTINATION ./)
install(DIRECTORY rknn_weights DESTINATION ./)
install(DIRECTORY test_imgs DESTINATION ./)
install(FILES ${CMAKE_SOURCE_DIR}/src/ppocr_keys_v1.txt DESTINATION ./)
install(FILES ${CMAKE_SOURCE_DIR}/run_rv1109_rv1126.sh DESTINATION ./)
install(PROGRAMS ${RKNN_API_LIB} DESTINATION lib)
install(PROGRAMS ${OPENCV_LIB} DESTINATION lib)
