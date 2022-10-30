cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_USER_MAKE_RULES_OVERRIDE cmake/override.cmake)

set(CMAKE_C_COMPILER "C:/SDCC/bin/sdcc.exe")

# make CMake happy with sdcc
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE MinSizeRel)

project("Project" C)

set(MCU_ID "12f683")

set(CMAKE_C_FLAGS "--use-non-free -mpic14 -p${MCU_ID}")

file(GLOB PROJECT_SOURCES
	"src/main.c"
)

include_directories(
    "inc"
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# print buil info
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "--------------------------------------------------"
    COMMAND ${CMAKE_COMMAND} -E echo " Build Information                                "
    COMMAND ${CMAKE_COMMAND} -E echo "--------------------------------------------------"
    COMMAND ${CMAKE_COMMAND} -E echo " PROJECT_NAME            : ${PROJECT_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo " PROJECT_VERSION         : ${VERSION_STR}"
    COMMAND ${CMAKE_COMMAND} -E echo "--------------------------------------------------"
    COMMAND ${CMAKE_COMMAND} -E echo " CMAKE_SYSTEM_PROCESSOR  : ${MCU_ID}"
    COMMAND ${CMAKE_COMMAND} -E echo " CMAKE_AR                : ${CMAKE_AR}"
    COMMAND ${CMAKE_COMMAND} -E echo " CMAKE_C_COMPILER        : ${CMAKE_C_COMPILER}"
    COMMAND ${CMAKE_COMMAND} -E echo " CMAKE_VERSION           : ${CMAKE_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "--------------------------------------------------"
)
