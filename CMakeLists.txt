cmake_minimum_required(VERSION 3.0)

#### setable variables ####
set(SDCC_EXECUTABLE "C:/SDCC/bin/sdcc.exe" CACHE PATH "path to sdcc executable" FORCE)
set(SDCC_LIB "C:/SDCC/lib" CACHE PATH "path to sdcc library directory" FORCE)

############# toolchain info, must come before project definition ##############
set(CMAKE_SYSTEM_PROCESSOR PIC12F683)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER ${SDCC_EXECUTABLE})
set(CMAKE_FIND_ROOT_PATH ${SDCC_LIB})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")
set(CMAKE_SHARED_LIBRARY_PREFIX "")         # lib
set(CMAKE_SHARED_LIBRARY_SUFFIX ".lib")     # .so
set(CMAKE_IMPORT_LIBRARY_PREFIX )
set(CMAKE_IMPORT_LIBRARY_SUFFIX )
set(CMAKE_EXECUTABLE_SUFFIX ".ihx")         # intel hex file
set(CMAKE_LINK_LIBRARY_SUFFIX ".lib")
set(CMAKE_DL_LIBS "")
set(CMAKE_C_OUTPUT_EXTENSION ".rel")

get_filename_component(SDCC_LOCATION "${CMAKE_C_COMPILER}" PATH)
find_program(SDCCLIB_EXECUTABLE sdcclib PATHS "${SDCC_LOCATION}" NO_DEFAULT_PATH)
find_program(SDCCLIB_EXECUTABLE sdcclib)
set(CMAKE_AR "${SDCCLIB_EXECUTABLE}" CACHE FILEPATH "The sdcc librarian" FORCE)

# preprocessor and linker flags
set(CMAKE_C_FLAGS_INIT "--use-non-free")
set(CMAKE_EXE_LINKER_FLAGS_INIT "--use-non-free")

# compile a C file into an object file
set(CMAKE_C_COMPILE_OBJECT  "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")

# link object files to an executable
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <OBJECTS> -o <TARGET> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES>")

# needs sdcc 2.7.0 + sddclib from cvs
set(CMAKE_C_CREATE_STATIC_LIBRARY
      "\"${CMAKE_COMMAND}\" -E remove <TARGET>"
      "<CMAKE_AR> -a <TARGET> <LINK_FLAGS> <OBJECTS> ")

# not supported by sdcc
set(CMAKE_C_CREATE_SHARED_LIBRARY "")
set(CMAKE_C_CREATE_MODULE_LIBRARY "")

############################## project definition ##############################

project("FAN_CONTROL" C)

set(CMAKE_BUILD_TYPE "Debug")
set(COMPILER_CLASSB_FLASH_END_ADDRES 0x7FE) #2k - 2

add_definitions(
    -D__SDCC_PIC12F683
    -DCOMPILER_CLASSB_FLASH_END_ADDRES=${COMPILER_CLASSB_FLASH_END_ADDRES}
)

file(GLOB PROJECT_SOURCES
    "C:/sdcc/non-free/lib/src/pic14/libdev/pic12f683.c"
	"main.c"
)

include_directories(
    "C:/SDCC/include"
    "C:/SDCC/include/pic14"
    "C:/SDCC/non-free/include/pic14"
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# generate CRC in hex file
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMENT
        "Generate CRC16 in the .hex file"
        DEPENDS
        ${PROJECT_NAME}.ihx        
    COMMAND
        ../out/srec_cat.exe ${PROJECT_NAME}.ihx -intel -crop 0 ${COMPILER_CLASSB_FLASH_END_ADDRES} -fill 0xFF 0x0000 ${COMPILER_CLASSB_FLASH_END_ADDRES} -crc16-l-e ${COMPILER_CLASSB_FLASH_END_ADDRES} -o ../out/Project.hex -intel -address-length=2
)

# generate hex to bin file
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMENT
        "Generate bin on the .hex file"
    DEPENDS
     ../out/Project.hex
    COMMAND
    ../out/srec_cat.exe ../out/Project.hex -Intel -o ../BinaryFile.bin -Binary
)

# print buil info
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "--------------------------------------------------"
    COMMAND ${CMAKE_COMMAND} -E echo " Build Information                                "
    COMMAND ${CMAKE_COMMAND} -E echo "--------------------------------------------------"
    COMMAND ${CMAKE_COMMAND} -E echo " PROJECT_NAME            : ${PROJECT_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo " PROJECT_VERSION         : ${VERSION_STR}"
    COMMAND ${CMAKE_COMMAND} -E echo "--------------------------------------------------"
    COMMAND ${CMAKE_COMMAND} -E echo " CMAKE_SYSTEM_PROCESSOR  : ${CMAKE_SYSTEM_PROCESSOR}"
    COMMAND ${CMAKE_COMMAND} -E echo " CMAKE_AR                : ${CMAKE_AR}"
    COMMAND ${CMAKE_COMMAND} -E echo " CMAKE_C_COMPILER        : ${CMAKE_C_COMPILER}"
    COMMAND ${CMAKE_COMMAND} -E echo " CMAKE_VERSION           : ${CMAKE_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "--------------------------------------------------"
)
