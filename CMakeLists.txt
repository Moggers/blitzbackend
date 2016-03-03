cmake_minimum_required(VERSION 2.8.7)
project(blitzserver)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
find_package(Threads)
find_package(LibConfig)
find_package(MySQL)
find_package(Jsoncpp)
find_package(POCO)
if(JSONCPP_FOUND)
	message("INFO: we found LibJsoncpp on your pc.")
	message(JSONCPP_FOUND = ${JSONCPP_FOUND})
	message(JSONCPP_INCLUDE_DIR = ${JSONCPP_INCLUDE_DIR}) 
	message(JSONCPP_LIBRARY = ${JSONCPP_LIBRARY})
else(JSONCPP_FOUND)
	message("WARNING: we couldn't find LibJsoncpp on your pc. DLC is disabled.")
endif(JSONCPP_FOUND)
message( ${MYSQL_LIBRARY})

include_directories(src)
file(GLOB SOURCES "src/*.cpp")

add_executable( blitzserver ${SOURCES})
message( ${CMAKE_MODULE_PATH}) 


include_directories( 
	${MYSQL_INCLUDE_DIR}
	${JSONCPP_INCLUDE_DIR}
	${POCO_INCLUDE_DIR}
	${POCO_Util_INCLUDE_DIR}
	${POCO_Net_INCLUDE_DIR}
	${POCO_NetSSL_INCLUDE_DIR})

target_link_libraries( blitzserver 
	${LIBCONFIG_LIBRARIES}
	${MYSQL_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	${JSONCPP_LIBRARY}
	${POCO_LIBRARY}
	${POCO_Util_LIBRARY}
	${POCO_Net_LIBRARY}
	${POCO_NetSSL_LIBRARY})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
set(Jsoncpp_PROCESS_INCLUDES Jsoncpp_INCLUDE_DIR)
set(Jsoncpp_PROCESS_LIBS Jsoncpp_LIBRARY)

install( TARGETS blitzserver DESTINATION bin)

