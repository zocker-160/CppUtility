cmake_minimum_required(VERSION 3.10)
project(CppUtils LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC
	Helper/Helper.cpp
	SimpleIni/ConvertUTF.c
)

target_include_directories(${PROJECT_NAME} PUBLIC 
	Helper
	httplib
	SimpleIni
)

#target_include_directories(
#	${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

message(STATUS "INCLUDE DIR FOR ${PROJECT_NAME} = ${CMAKE_CURRENT_SOURCE_DIR}")
