cmake_minimum_required(VERSION 3.18)
project(worker)

set(SOURCES
    worker.cpp
    scheduler.cpp
)

set(HEADERS
    worker.h
    scheduler.h
)

add_library(worker SHARED ${SOURCES} ${HEADERS})
target_include_directories(worker PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
