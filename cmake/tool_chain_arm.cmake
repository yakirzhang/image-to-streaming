set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_CXX_COMPILER $ENV{STAGING}/bin/aarch64-linux-gnu-g++ CACHE PATH "" )
set(CMAKE_C_COMPILER $ENV{STAGING}/bin/aarch64-linux-gnu-gcc CACHE PATH "" )
set(CMAKE_FIND_ROOT_PATH $ENV{STAGING})
# search for programs in the build host directories (not necessary)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
SET(BOOST_LIBRARYDIR /usr/lib/x86_64-linux-gnu)
SET(BOOST_INCLUDE_DIR /usr/include/boost/)