project(proxy)
cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pedantic -Wall -Wextra -Wno-char-subscripts -Wno-unused-result -g -fsanitize=address")
add_definitions(-D_GLIBCXX_DEBUG)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
file(GLOB_RECURSE HEADER_FILES include/*.h)
include_directories(include)
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra -Wno-char-subscripts -Wno-unused-result -g ")
#aux_source_directory(src/ SRC_LIST)
#add_executable(${PROJECT_NAME} ${SRC_LIST})
#set(THREADS_PREFER_PTHREAD_FLAG ON)
#set(CMAKE_C_FLAGS "-Wall -g -std=c++11")

#project(twitter_hashtag)
#cmake_minimum_required(VERSION 3.2)

##set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra -Wno-char-subscripts -Wno-unused-result -g -fsanitize=thread")
##set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -g")
##set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb")

##add_definitions(-D_LINUX)
##add_definitions(-DNDEBUG)



#if ("$ENV{GTEST}" STREQUAL "y")
#    message("!Gtests're enabled!")

#    find_package(GTest REQUIRED)

#    include_directories(tests)
#    add_definitions(-D_GTEST)
#endif ()

#link_directories(
#    "${PROJECT_SOURCE_DIR}/3dparty/chilkat/lib"
#    "${PROJECT_SOURCE_DIR}/3dparty/json_spirit/json_spirit" )


#if ("$ENV{GTEST}" STREQUAL "y")
#    target_link_libraries(${PROJECT_NAME} ${GTEST_BOTH_LIBRARIES} gmock)
#endif ()

#target_link_libraries(${PROJECT_NAME} chilkat-9.5.0 resolv pthread json_spirit)


##if ("${BENCHMARK}" STREQUAL "y")
##    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
##endif()


##if ("$ENV{BENCHMARK}" STREQUAL "y")

##    set(Boost_USE_STATIC_LIBS OFF)
##    set(Boost_USE_MULTITHREADED ON)
##    set(Boost_USE_STATIC_RUNTIME OFF)

##    find_package(Boost COMPONENTS regex)

##    if(Boost_FOUND)
##        include_directories(${Boost_INCLUDE_DIRS})
##        add_definitions(-DBENCHMARK)
##        set(BENCHMARK "y")
##    else ()
##        message("boost not found")
##    endif ()
##endif ()
