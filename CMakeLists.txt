cmake_minimum_required(VERSION 3.20.0)

set(NAME "CPU_Skinning")
project(directx11_gltf_project)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB VERTEX_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*VS.hlsl"
)

file(GLOB PIXEL_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*PS.hlsl"
)

file(GLOB CODES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Common/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Common/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Inputs/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Inputs/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Objects/Model/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Objects/Model/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Objects/Sprite/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Objects/Sprite/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Window/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Window/*.cpp"
    ${VERTEX_SHADERS}
    ${PIXEL_SHADERS}
)

foreach(FILE ${VERTEX_SHADERS})
    set_source_files_properties(
        ${FILE}
        ${VERTEX_SHADERS} 
        PROPERTIES 
        VS_SHADER_TYPE "Vertex Shader (/vs)"
        VS_SHADER_MODEL "Shader Model 5.0 (/5_0)")
        
endforeach(FILE ${VERTEX_SHADERS})
        
foreach(FILE ${PIXEL_SHADERS})
    set_source_files_properties(
            ${FILE}
            ${PIXEL_SHADERS} 
            PROPERTIES 
            VS_SHADER_TYPE "Pixel Shader (/ps)"
            VS_SHADER_MODEL "Shader Model 5.0 (/5_0)")
    
endforeach(FILE ${PIXEL_SHADERS})

add_executable(${NAME} ${CODES})
target_include_directories(${NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ThirdLib/include")
target_compile_options(${NAME} PRIVATE /W3 /sdl /MP)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")