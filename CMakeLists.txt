cmake_minimum_required(VERSION 3.0.0)
project(EasyAIoT VERSION 0.1.0)

set(CMAKE_C_FLAGS "-std=gnu99")
set(CMAKE_CXX_FLAGS "-fpermissive -D_DEBUG")
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_C_COMPILER "arm-cortex_a9-linux-gnueabi-gcc")
SET(CMAKE_CXX_COMPILER "arm-cortex_a9-linux-gnueabi-g++")
SET(CMAKE_SYSTEM_PROCESSOR "arm")

include_directories(./ )
include_directories(./Algo )
include_directories(./APP )
include_directories(./Audio )
include_directories(./Common )
include_directories(./MBEDTLS )
include_directories(./Net )

aux_source_directory(./ DIR_SRCS)
aux_source_directory(./Algo DIR_ALGO_SRCS)
aux_source_directory(./APP DIR_APP_SRCS)
aux_source_directory(./Audio DIR_AUDIO_SRCS)
IF (WIN32)
include_directories(./Audio )
aux_source_directory(./Audio/WIN DIR_AUDIO_SUB_SRCS)
ELSEIF (APPLE)
	MESSAGE(STATUS "Now is Apple systems.")
ELSEIF (UNIX)
include_directories(./Audio/Linux ./Audio/Linux/tinyalsa ./Audio/Linux/tinyalsa/include)
aux_source_directory(./Audio/Linux DIR_AUDIO_SUB_SRCS)
ENDIF ()
aux_source_directory(./Common DIR_COMMON_SRCS)
aux_source_directory(./Net DIR_NET_SRCS)

add_library(EasyAIoT STATIC
				${DIR_SRCS} 
				${DIR_ALGO_SRCS} 
				${DIR_APP_SRCS} 
				${DIR_AUDIO_SRCS} 
				${DIR_AUDIO_SUB_SRCS} 
				${DIR_COMMON_SRCS}
				${DIR_NET_SRCS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

IF (WIN32)
target_link_libraries(EasyAIoT winmm)
target_link_libraries(EasyAIoT ws2_32)
ELSEIF (UNIX)
target_link_libraries(EasyAIoT pthread)
ENDIF ()
