cmake_minimum_required(VERSION 3.30)
project(dq5_ds_rands)

set(CMAKE_CXX_STANDARD 20)

add_executable(dq5_ds_rands main.cpp)

if(MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=x86-64 -mtune=generic ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=x86-64 -mtune=generic")
    set(CMAKE_EXE_LINKER_FLAGS "-static -Wl,--gc-sections")
elseif(MSVC)
    set_property(TARGET newDirectory PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /O2")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()