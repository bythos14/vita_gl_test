cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{DOLCESDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
  endif()
endif()

project(piglet)

include("$ENV{DOLCESDK}/share/dolce.cmake" REQUIRED)

set(VITA_APP_NAME "Piglet Sample")
set(VITA_TITLEID  "PGLVITA00")
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -Wall")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")

set (DOLCE_ELF_CREATE_FLAGS "${DOLCE_ELF_CREATE_FLAGS} -h 0x1000000")

include_directories(
	./include
)

add_executable(${PROJECT_NAME}.elf
  src/main.c
)

target_link_libraries(${PROJECT_NAME}.elf
  SceLibKernel_stub
  SceModulemgr_stub
  SceSysmodule_stub
  SceLibc_stub
  taihen_stub
)

dolce_create_self(eboot.bin ${PROJECT_NAME}.elf)
# The FILE directive lets you add additional files to the VPK, the syntax is
# FILE src_path dst_path_in_vpk. In this case, we add the LiveArea paths.
dolce_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)
