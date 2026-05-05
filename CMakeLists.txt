cmake_minimum_required(VERSION 3.10)
project(Renderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用 OpenMP
#find_package(OpenMP)

include_directories(${CMAKE_SOURCE_DIR}/include)

link_directories(${CMAKE_SOURCE_DIR}/lib)


file(GLOB SOURCES src/*.h src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} SDL3)

# 链接 OpenMP
# if(OpenMP_CXX_FOUND)
#     target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
# endif()

# 复制 resources 文件夹到输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)