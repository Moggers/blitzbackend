cmake_minimum_required(VERSION 2.8.7)
project(blitzserver)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
find_package(Threads)
find_package(LibConfig)
find_package(MySQL)
message( ${MYSQL_LIBRARY})

include_directories(src)
file(GLOB SOURCES "src/*.cpp")

add_executable( blitzserver ${SOURCES})
message( ${CMAKE_MODULE_PATH}) 


include_directories( ${MYSQL_INCLUDE_DIR})
target_link_libraries( blitzserver 
	${LIBCONFIG_LIBRARIES}
	${MYSQL_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

install( TARGETS blitzserver DESTINATION bin)
