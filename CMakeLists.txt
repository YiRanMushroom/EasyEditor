cmake_minimum_required(VERSION 3.31)
project(EasyEngineSandbox)

set(PROJECT_NAME EasyEngineSandbox)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE MODULES src/*.ixx src/*.cppm)

add_executable(EasyEngineSandbox ${SOURCES} ${MODULES})

target_sources(EasyEngineSandbox
        PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        FILES ${MODULES}
)

add_subdirectory(EasyEngineCore)

target_link_libraries(EasyEngineSandbox
        PRIVATE
        EasyEngineCore
)