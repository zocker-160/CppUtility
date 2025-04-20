cmake_minimum_required(VERSION 3.10)
project(CppUtils LANGUAGES CXX)

add_library(${PROJECT_NAME} SHARED
	Helper/Helper.cpp
	SimpleIni/ConvertUTF.c
)

target_include_directories(${PROJECT_NAME} PRIVATE 
	Helper
	httplib
	SimpleIni
)
