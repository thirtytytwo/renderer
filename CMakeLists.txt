cmake_minimum_required(VERSION 3.10)

set(${CMAKE_TOOLCHAIN_FILE} "C:/Users/32672/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(renderer)

#find required packages

find_package(glfw3 CONFIG REQUIRED)
message(STATUS "found glfw3 in ${GLFW3_LIBRARY_DIR}")
find_package(glm CONFIG REQUIRED)
message(STATUS "found glm in ${GLM_LIBRARY_DIR}")

#platform setting

if(WIN32)
    set(LIBS glfw3 opengl32)
elseif(APPLE)
    INCLUDE_DIRECTORIES(/System/Library/Frameworks)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
    FIND_LIBRARY(IOKit_LIBRARY IOKit)
    FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)
    MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
    SET(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
    SET(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY})
    set(LIBS ${LIBS} ${APPLE_LIBS})
else()
    set(LIBS )
endif(WIN32)

#set file

file(GLOB SRC_FILES
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glm::glm)

