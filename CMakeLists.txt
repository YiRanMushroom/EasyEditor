cmake_minimum_required(VERSION 3.31)
project(EasyEditor)

set(PROJECT_NAME EasyEditor)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE MODULES src/*.ixx src/*.cppm)

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