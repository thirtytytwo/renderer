cmake_minimum_required(VERSION 3.16)
project(Renderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 导出 compile_commands.json，供 clangd / IDE 索引使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 启用 OpenMP
#find_package(OpenMP)

include_directories(${CMAKE_SOURCE_DIR}/include)

# ---------------- MiniFB (窗口 / 事件 / 帧缓冲呈现) ----------------
# 只构建库本体，跳过它自带的示例程序
set(MINIFB_BUILD_EXAMPLES     OFF CACHE BOOL "" FORCE)
set(MINIFB_BUILD_VERSION_INFO OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/minifb EXCLUDE_FROM_ALL)

file(GLOB SOURCES
    src/*.h src/*.cpp
    src/Render/*.h src/Render/*.cpp
    src/Platform/*.h src/Platform/*.cpp
)
add_executable(${PROJECT_NAME} ${SOURCES})

# 注意：不要把 src/ 加入 include 路径。macOS 文件系统大小写不敏感，
# src/Math.h 会遮蔽 libc++ 的 <math.h>，导致标准库头文件链断裂。
# 项目内头文件统一使用相对路径引用。
target_link_libraries(${PROJECT_NAME} PRIVATE minifb)

if(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        # ARM 上没有 <immintrin.h>，用 SIMDe 把 SSE/AVX intrinsic 映射到 NEON
        target_include_directories(${PROJECT_NAME} BEFORE PRIVATE
            ${CMAKE_SOURCE_DIR}/external/compat/arm
            ${CMAKE_SOURCE_DIR}/external/simde
        )
    endif()

    # 允许 Vec4f/Vec4d 中 union 内的匿名 struct（MSVC 扩展）
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-nested-anon-types -Wno-gnu-anonymous-struct)
endif()

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
