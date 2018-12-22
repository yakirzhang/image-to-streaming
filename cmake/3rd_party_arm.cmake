set(DF_3RDPARTY_DIR $ENV{STAGING}/3rd_party)
#gflags
add_library(gflags_arm INTERFACE)
target_link_libraries(gflags_arm INTERFACE ${DF_3RDPARTY_DIR}/gflags_build/lib/libgflags.a)
target_include_directories(gflags_arm INTERFACE ${DF_3RDPARTY_DIR}/gflags_build/include)
#nanomsg
add_library(nanomsg_arm INTERFACE)
target_link_libraries(nanomsg_arm INTERFACE ${DF_3RDPARTY_DIR}/nanomsg_build/lib/libnanomsg.a anl pthread)
target_include_directories(nanomsg_arm INTERFACE ${DF_3RDPARTY_DIR}/nanomsg_build/include)
#opencv
find_package(OpenCV PATHS ${DF_3RDPARTY_DIR}/opencv_build/share/OpenCV NO_DEFAULT_PATH)
#Eigen
include_directories(${DF_3RDPARTY_DIR}/eigen3)
