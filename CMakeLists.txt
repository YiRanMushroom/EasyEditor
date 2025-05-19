cmake_minimum_required(VERSION 3.31)
project(EasyEditor)

set(PROJECT_NAME EasyEditor)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE MODULES src/*.ixx src/*.cppm)

if (MSVC)
    # use static runtime library
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else ()
endif ()

add_executable(${PROJECT_NAME} ${SOURCES} ${MODULES})

target_sources(${PROJECT_NAME}
        PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        FILES ${MODULES}
)

add_subdirectory(EasyEngineCore)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        EasyEngineCore
)

# Copy the files and directories to the output directory
set(FilesToCopy
#        "imgui.ini"
)

set(DirectoriesToCopy
        "assets"
        "Resources"
        "SandboxProject")

foreach(File ${FilesToCopy})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${File}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${File}")

    message(STATUS "Copying file: ${File}")
endforeach()

foreach(Directory ${DirectoriesToCopy})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/${Directory}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${Directory}")

    message(STATUS "Copying directory: ${Directory}")
endforeach()