cmake_minimum_required(VERSION 3.2)
project(image-to-streaming)

include(cmake/3rd_party_x64.cmake)
find_package(PkgConfig)

pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.4
        gstreamer-sdp-1.0>=1.4
        gstreamer-video-1.0>=1.4
        gstreamer-app-1.0>=1.4)
add_library(gstreamer INTERFACE)
target_include_directories(gstreamer INTERFACE ${GST_INCLUDE_DIRS})
target_compile_options(gstreamer INTERFACE  ${GST_CFLAGS})
target_link_libraries (gstreamer INTERFACE ${GST_LIBRARIES})

pkg_check_modules(GTK REQUIRED gtk+-2.0)
add_library(gtk INTERFACE)
target_include_directories(gtk INTERFACE ${GTK_INCLUDE_DIRS})
target_compile_options(gtk INTERFACE  ${GTK_CFLAGS})
target_link_libraries (gtk INTERFACE ${GTK_LIBRARIES})

# test
add_executable(t1 basic_gstreamer/t1.cpp)
target_link_libraries(t1 PRIVATE gstreamer gtk)
add_executable(t3 app/t3.cpp)
target_link_libraries(t3 PRIVATE gstreamer gtk nanomsg_x64_static glog)
add_executable(t_yuv app/t_yuv.cpp)
target_link_libraries(t_yuv PRIVATE gstreamer gtk nanomsg_x64_static glog)
add_executable(t2 basic_gstreamer/t2.cpp)
target_link_libraries(t2 PRIVATE gstreamer gtk)
add_executable(test_gstreamer basic_gstreamer/test_gstreamer.cpp)
target_link_libraries(test_gstreamer PRIVATE gstreamer gtk)
add_executable(test_gstreamer2 basic_gstreamer/test_gstreamer2.cpp)
target_link_libraries(test_gstreamer2 PRIVATE gstreamer gtk)
add_executable(test_gstreamer3 basic_gstreamer/test_gstreamer3.cpp)
target_link_libraries(test_gstreamer3 PRIVATE gstreamer gtk)
