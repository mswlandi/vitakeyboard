cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	if(DEFINED ENV{VITASDK})
		set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
	else()
		message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
	endif()
endif()

project(vitakeyboard)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(TITLE_ID "KEYBOARD0")
set(TITLE_NAME "VitaKeyboard")
set(VITA_VERSION "01.02")

set(CMAKE_C_FLAGS "-Wl,-q -Wall -std=gnu11") # -O0 -g3
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
	${PROJECT_SOURCE_DIR}/../skprx/uapi
)

link_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/../skprx/build/stubs
)

if (NOT ${RELEASE})
	add_definitions(-DENABLE_LOGGING)
endif()

add_executable(${PROJECT_NAME}.elf
	main.c
	debugScreenFont.c
)

target_link_libraries(${PROJECT_NAME}.elf
	hidkeyboard_stub_weak
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
	SceAppMgr_stub
)

vita_create_self(eboot.bin ${PROJECT_NAME}.elf)
vita_create_vpk(${PROJECT_NAME}.vpk ${TITLE_ID} eboot.bin
	VERSION	${VITA_VERSION}
	NAME ${TITLE_NAME}
	FILE pkg/sce_sys/icon0.png sce_sys/icon0.png
		 pkg/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
		 pkg/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
		 pkg/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)

add_custom_target(vpksend
	COMMAND curl -T ${PROJECT_NAME}.vpk ftp://$(PSVITAIP):1337/ux0:/VPK/
	DEPENDS ${PROJECT_NAME}.vpk
)

add_custom_target(send
	COMMAND curl -T eboot.bin ftp://$(PSVITAIP):1337/ux0:/app/${TITLE_ID}/eboot.bin
	DEPENDS eboot.bin
)
