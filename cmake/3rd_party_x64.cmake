message(STATUS "include 3rd_party_x64.cmake")

if(${BUILD_STATIC} MATCHES "ON")
set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(opencv_dir "/home/julian/opencv-3.4.3/build_x64_static/install")
endif()

message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})
# nanomsg_static
set(nanomsg_dir ${CMAKE_CURRENT_SOURCE_DIR}/nanomsg/install)
add_library(nanomsg_x64_static INTERFACE)
target_link_libraries(nanomsg_x64_static INTERFACE ${nanomsg_dir}/lib/libnanomsg.a anl pthread)
target_include_directories(nanomsg_x64_static INTERFACE ${nanomsg_dir}/include)

#opencv
if(${BUILD_STATIC} MATCHES "ON")
find_package(OpenCV REQUIRED PATHS ${opencv_dir}/share/OpenCV/ NO_DEFAULT_PATH) # find_opencv will give a "opencv_${module_name}" lib-interface
else()
find_package(OpenCV REQUIRED) # find_opencv will give a "opencv_${module_name}" lib-interface
endif()
#    add_library(opencv INTERFACE)
#    message(STATUS ${OpenCV_INCLUDE_DIRS})
#    message(STATUS ${OpenCV_LIBS})
#    target_include_directories(opencv INTERFACE ${OpenCV_INCLUDE_DIRS})
#    target_link_libraries(opencv INTERFACE ${OpenCV_LIBS})

#boost
#
find_package(Boost REQUIRED COMPONENTS system filesystem)   # find_boost will give "Boost::boost" Head-only lib-interface
#and "Boost::${component_name}" lib-interface
add_library(boost INTERFACE)
message(STATUS ${Boost_INCLUDE_DIRS})
message(STATUS ${Boost_LIBRARIES})
target_include_directories(boost INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries(boost INTERFACE ${Boost_LIBRARIES})

#eigen
find_package(Eigen3 REQUIRED) # find_eigen will give a "Eigen3::Eigen" lib-interface

# Ceres
find_package(Ceres REQUIRED) # find_ceres will give a "ceres" lib-interface

## Protobuf locate at /usr/share/cmake-3.10/Modules/FindProtobuf.cmake
find_package(Protobuf REQUIRED) #find_protobuf will giva you protobuf::libprotobuf and protobuf::libprotobuf-lite
# yaml
#find_package(yaml-cpp REQUIRED) # yaml-cpp interface will get
add_library(yaml-cpp INTERFACE)
target_include_directories(yaml-cpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp/install/include)
target_link_libraries(yaml-cpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp/install/lib/libyaml-cpp.a)
