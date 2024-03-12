cmake_minimum_required(VERSION 3.1.0)

project(Network)

add_definitions("-Wall -g")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

message("project binary dir: " ${CMAKE_SOURCE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/include)

message("PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})

aux_source_directory(${PROJECT_SOURCE_DIR}/src/common COMM_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/tcp TCP_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/udp UDP_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/unix_socket/stream UN_STREAM_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/unix_socket/data_gram UN_DG_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/src MAIN)

message("TCP_SRC: " ${TCP_SRC})

add_executable(run ${MAIN} ${COMM_SRC} ${TCP_SRC} ${UDP_SRC} ${UN_STREAM_SRC} ${UN_DG_SRC})

target_link_libraries(run pthread)