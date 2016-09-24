cmake_minimum_required(VERSION 2.8)

project(dstore C CXX)

set(CXX_FLAGS
 -g
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -std=c++0x
 )

set(CMAKE_CXX_COMPILER "g++")

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories("${PROJECT_SOURCE_DIR}/dstore/common")
include_directories("${PROJECT_SOURCE_DIR}/dstore/network")
include_directories("${PROJECT_SOURCE_DIR}/dstore/main")

add_subdirectory(dstore/common)
add_subdirectory(dstore/network)
add_subdirectory(dstore/main)
add_subdirectory(dstore/test)

foreach(src_file ${COMMON_SRCS})
  set(SRCS ${SRCS} "dstore/common/${src_file}")
endforeach(src_file)

foreach(src_file ${NETWORK_SRCS})
  set(SRCS ${SRCS} "dstore/network/${src_file}")
endforeach(src_file)

foreach(src_file ${TEST_SRCS})
  set (EXECUTE_TEST_SRCS ${SRCS} "dstore/test/${src_file}")
  string(REPLACE "." ";" src_list ${src_file})
  list(GET src_list 0 EXECUTABLE_NAME)
  add_executable(${EXECUTABLE_NAME} ${EXECUTE_TEST_SRCS})
endforeach(src_file)

foreach(src_file ${MAIN_SRCS})
  set(SRCS ${SRCS} "dstore/main/${src_file}")
endforeach(src_file)

#message(STATUS "${SRCS}")
#add_executable(dstore ${SRCS})
