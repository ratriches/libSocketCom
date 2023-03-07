cmake_minimum_required(VERSION 2.8)

project(AppTestLocalServer)

message("\n-- INICIO CMAKE --\n")
message("CMAKE_CURRENT_SOURCE_DIR = " ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

##############################################################
# message("MAKE_CROSSCOMPILING = " $ENV{MAKE_CROSSCOMPILING})
if(DEFINED ENV{MAKE_CROSSCOMPILING} AND $ENV{MAKE_CROSSCOMPILING} STREQUAL "TRUE")
	set(CMAKE_CROSSCOMPILING "TRUE")
	add_definitions(-DCROSS_COMP=1)		# usado dentro do codigo
else()
	set(CMAKE_CROSSCOMPILING "FALSE")
	add_definitions(-DCROSS_COMP=0)		# usado dentro do codigo
endif()
message("CMAKE_CROSSCOMPILING = " ${CMAKE_CROSSCOMPILING})

##############################################################
# message("BUILD_MODE = " $ENV{BUILD_MODE})
if(DEFINED ENV{BUILD_MODE} AND $ENV{BUILD_MODE} STREQUAL "DEBUG")
	set(CMAKE_BUILD_TYPE "Debug")
else ()
	set(CMAKE_BUILD_TYPE "Release")
endif ()
message("CMAKE_BUILD_TYPE = " ${CMAKE_BUILD_TYPE})

##############################################################
if(DEFINED ENV{INSTALLDIR})
	set(installDir $ENV{INSTALLDIR})
else()
	set(installDir /usr/share/libSocketCom) # default
endif()

# set(libsdir ${installDir}/lib)

if(${CMAKE_CROSSCOMPILING} STREQUAL "TRUE")
	set(libsdir ../build/lib)
	find_library(SOCKETCOM_LIBRARY SocketCom HINTS ${libsdir})

	set(tools /opt/FriendlyArm/gcc-linaro-5.4.1)
	set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gnueabihf-gcc)
	set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-gnueabihf-g++)
else()
	set(libsdir ../buildPC/lib)
	find_library(SOCKETCOM_LIBRARY SocketCom HINTS ${libsdir})

	# set(tools /usr)
	# set(CMAKE_C_COMPILER ${tools}/bin/gcc)
	# set(CMAKE_CXX_COMPILER ${tools}/bin/g++)
endif()

if (DEFINED SOCKETCOM_LIBRARY)
	message("SOCKETCOM_LIBRARY = " ${SOCKETCOM_LIBRARY})
endif()

message("installDir=" ${installDir})
message("CMAKE_C_COMPILER = " ${CMAKE_C_COMPILER})
message("CMAKE_CXX_COMPILER = " ${CMAKE_CXX_COMPILER})

##############################################################
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -Wall")
#set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -O2 -g" )
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
message("CMAKE_CXX_FLAGS = " ${CMAKE_CXX_FLAGS})
message("CMAKE_C_FLAGS = " ${CMAKE_C_FLAGS})

##############################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ExpInc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ExpInc)

# if (DEFINED SOCKETCOM_LIBRARY)
# 	include_directories(${installDir}/include)
# endif()

##############################################################
if(${CMAKE_CROSSCOMPILING} STREQUAL "TRUE")
	set(dir ${CMAKE_CURRENT_SOURCE_DIR}/build/)
else()
	set(dir ${CMAKE_CURRENT_SOURCE_DIR}/buildPC/)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${dir})
set(CMAKE_BUILD_FILES_DIRECTORY ${dir})
set(CMAKE_BUILD_DIRECTORY ${dir})
set(CMAKE_BINARY_DIR ${dir})
set(EXECUTABLE_OUTPUT_PATH ${dir})
set(LIBRARY_OUTPUT_PATH ${dir}lib)
set(CMAKE_CACHEFILE_DIR ${dir})
message("OutBinDir = " ${dir})
message("CMAKE_SYSTEM_NAME = " ${CMAKE_SYSTEM_NAME})

# local das libs so
set(CMAKE_INSTALL_RPATH ${installDir}/lib) # para correto funcionamento da instalacao
# ou set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) 

##############################################################
set(MAIN_FILES
	Src/main.cpp
	Src/LogSys.cpp
)

# option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

add_executable(${PROJECT_NAME}
	${MAIN_FILES}

)

if (DEFINED SOCKETCOM_LIBRARY)
	#  precisa estar abaixo de add_executable
	target_link_libraries(${PROJECT_NAME} PUBLIC ${SOCKETCOM_LIBRARY})
	# target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${SOCKETCOM_LIBRARY})
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
# NOTE: com o comando cmake --install . --prefix "/usr/share/libSocketCom/"
# destino final sera "/usr/share/libSocketCom/bin"
# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ExpInc/* DESTINATION AppTest/include)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ExpInc/ DESTINATION AppTest/include FILES_MATCHING PATTERN "*.h")


message("\n-- FIM CMAKE --\n")

