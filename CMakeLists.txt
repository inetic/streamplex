cmake_minimum_required (VERSION 3.2)
################################################################################
set(CMAKE_BUILD_TYPE Debug)
set(GLOB BOOST_VERSION 1.58)

################################################################################
enable_testing()

find_package(Boost ${BOOST_VERSION} COMPONENTS unit_test_framework
                                               coroutine
                                               system
                                               REQUIRED)
find_package(Threads)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -Wall -ggdb")

include_directories(
  "${Boost_INCLUDE_DIR}"
  "${CMAKE_SOURCE_DIR}/include")

file(GLOB sources
  "${CMAKE_SOURCE_DIR}/tests/*.cpp")

foreach(test_src ${sources})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} ${Boost_LIBRARIES})
    add_test(NAME ${test_name} COMMAND ${test_name} --debug -VV)
endforeach(test_src)
