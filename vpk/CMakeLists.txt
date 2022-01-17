cmake_minimum_required(VERSION 2.8)

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_C_COMPILER "arm-vita-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-vita-eabi-g++")

project(vitakeyboard)
set(TITLE_ID "KEYBOARD0")
set(TITLE_NAME "VitaKeyboard")

set(CMAKE_C_FLAGS "-Wl,-q -Wall -std=gnu11")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3") # added to gcc flags when MAKE_BUILD_TYPE=Debug
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
)

link_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

if (NOT ${RELEASE})
	add_definitions(-DENABLE_LOGGING)
endif()

add_executable(${PROJECT_NAME}.elf
	main.c
	debugScreenFont.c
)

target_link_libraries(${PROJECT_NAME}.elf
	taihen_stub
	vita2d
	SceDisplay_stub
	SceGxm_stub
	SceSysmodule_stub
	SceCtrl_stub
	SceCommonDialog_stub
	SceAppUtil_stub
	SceIme_stub
	SceSysmem_stub
	png
	jpeg
	z
	m
	c
	SceAppMgr_stub
)

add_custom_target(${PROJECT_NAME}.velf ALL
	COMMAND vita-elf-create ${PROJECT_NAME}.elf ${PROJECT_NAME}.velf
)

add_custom_target(eboot.bin ALL
	COMMAND vita-make-fself ${PROJECT_NAME}.velf eboot.bin
)

add_custom_target(${PROJECT_NAME}.vpk ALL
	COMMAND vita-mksfoex -s TITLE_ID=${TITLE_ID} "${TITLE_NAME}" param.sfo
	COMMAND vita-pack-vpk -s param.sfo -b eboot.bin ${PROJECT_NAME}.vpk
)

add_custom_target(vpksend
	COMMAND curl -T ${PROJECT_NAME}.vpk ftp://${PSVITAIP}:1337/ux0:/VPK/
	DEPENDS ${PROJECT_NAME}.vpk
)

add_custom_target(send
	COMMAND curl -T eboot.bin ftp://${PSVITAIP}:1337/ux0:/app/${TITLE_ID}/
	DEPENDS eboot.bin
)

add_dependencies(${PROJECT_NAME}.velf ${PROJECT_NAME}.elf)
add_dependencies(eboot.bin ${PROJECT_NAME}.velf)
add_dependencies(${PROJECT_NAME}.vpk eboot.bin)
