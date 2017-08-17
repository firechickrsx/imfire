project(acessServer)
cmake_minimum_required(VERSION 2.8)

#设置顶级目录
IF(NOT IMFIRE_ROOT_DIR)
   SET(IMFIRE_ROOT_DIR PROJECT_BINARY_DIR)
ENDIF(NOT IMFIRE_ROOT_DIR)

#add_subdirectory(${IMFIRE_ROOT_DIR}/base)



#支持cpp++11
add_compile_options(-std=c++11)


include_directories(${IMFIRE_ROOT_DIR})

aux_source_directory(. SRC_LIST)

link_directories(${IMFIRE_ROOT_DIR}/build)
link_libraries(base)

link_directories(${IMFIRE_ROOT_DIR}/lib3rd/glog/lib)
MESSAGE(${IMFIRE_ROOT_DIR}/lib3rd/glog/lib)


link_libraries(pthread)

link_libraries(libglog.a)
add_executable(${PROJECT_NAME} ${SRC_LIST})








