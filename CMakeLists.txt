cmake_minimum_required(VERSION 3.2)
project(cmake_demo)


###########
# Toolchain
###########
set(toolchain   "$ENV{HOME}/toolchain_desktop.cmake"      CACHE FILEPATH     "")  # override with e.g. -D toolchain=~/toolchain_embedded.cmake
                                                                                  # e.g. cmake -D toolchain="" ..

if (toolchain AND EXISTS ${toolchain})
  message("==> Including toolchain_file ${toolchain}")
  include(${toolchain})
endif()



###########
# Debug or Release
###########
if (NOT CMAKE_BUILD_TYPE)
  ## set default to Debug
  set(CMAKE_BUILD_TYPE Debug)  # override with -DCMAKE_BUILD_TYPE=Release
  message("==> CMAKE_BUILD_TYPE empty. Changing it to Debug.")
else()
  message("==> CMAKE_BUILD_TYPE == ${CMAKE_BUILD_TYPE}.")
endif()

## https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_macros.html
## During Debug, use debug version of libstdc++ (asserts on access to invalid iterators, etc!)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")


###########
# Suppress certain warnings
###########
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")


###########
# clean-all (removes cmake's cache)
###########

# http://stackoverflow.com/a/13714219
add_custom_target(clean-all
  COMMAND ${CMAKE_BUILD_TOOL} clean
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/clean-all.cmake
  )


##############
# C++ Standard
##############
set(CMAKE_CXX_STANDARD   14)
set(CMAKE_CXX_EXTENSIONS OFF)


##############
# Sources and Target Name
##############
include_directories(../include)

set(src    main.cpp)
set(target go)


##############
# Boost
##############
if(WIN32)
   set(Boost_USE_STATIC_LIBS ON)  # on windows: http://www.cmake.org/pipermail/cmake/2011-May/044190.html
                                  # C:\home\boost\lib\libboost_bla.lib are created by the default build and are static libs
endif()

find_package(Boost COMPONENTS system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  set(libs ${libs}    ${Boost_LIBRARIES})
endif()


##############
# PThread
##############
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads)
set(libs ${libs} ${CMAKE_THREAD_LIBS_INIT})


##############
# Build and Link
##############
add_executable(${target} ${src})
target_link_libraries(${target} ${libs})
