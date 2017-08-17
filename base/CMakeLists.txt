#工程名称
project(base)
#支持的最低cmake版本
cmake_minimum_required(VERSION 2.8)

#支持cpp++11
add_compile_options(-std=c++11)

aux_source_directory(. SRC_LIST)

include_directories( "../lib3rd/glog/include" )



#生成静态库
ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRC_LIST})

