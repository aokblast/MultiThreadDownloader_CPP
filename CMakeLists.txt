cmake_minimum_required(VERSION 3.21)
project(MultiThreadDownloader)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_STANDARD 17)

add_executable(MultiThreadDownloader main.cpp HttpDownloader.cpp HttpDownloader.h)

#...
# cURLpp requires libcurl to be installed
message(STATUS "Looking for curl...")
find_package(CURL REQUIRED)

if (CURL_FOUND)
    message(STATUS "Found curl version: ${CURL_VERSION_STRING}")
    message(STATUS "Using curl version: ${CURL_INCLUDE_DIRS}")
    message(STATUS "Using curl libraries: ${CURL_LIBRARIES}\n")
    list(APPEND Project_INCLUDE_DIRS ${CURL_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Could not find curl.")
endif()


message(STATUS "Looking for curlpp...")
find_package(CURLPP REQUIRED)

if (CURLPP_FOUND)
    message(STATUS "Found curlpp version: ${CURLPP_VERSION}")
    message(STATUS "Using curlpp include dir: ${CURLPP_INCLUDE_DIR}")
    message(STATUS "Using curlpp libraries: ${CURLPP_LIBRARY}\n")
    list(APPEND Project_INCLUDE_DIRS ${CURLPP_INCLUDE_DIR})
    target_include_directories(MultiThreadDownloader PRIVATE ${CURLPP_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Could not find curlpp.")
endif()

target_link_libraries(MultiThreadDownloader ${CURL_LIBRARIES})
target_link_libraries(MultiThreadDownloader ${CURLPP_LIBRARY})




