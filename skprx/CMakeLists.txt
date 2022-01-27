cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	if(DEFINED ENV{VITASDK})
		set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
	else()
		message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
	endif()
endif()

# set(CMAKE_SYSTEM_NAME "Generic")
# set(CMAKE_C_COMPILER "arm-vita-eabi-gcc")
# set(CMAKE_CXX_COMPILER "arm-vita-eabi-g++")

project(hidkeyboard)
include("${VITASDK}/share/vita.cmake" REQUIRED)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O2 -std=gnu99")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O2 -nostdlib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

# include_directories(
# )

# link_directories(
# 	${CMAKE_CURRENT_BINARY_DIR}
# )

add_executable(${PROJECT_NAME}.elf
	main.c
	hidkeyboard.c
	log.c
)

target_link_libraries(${PROJECT_NAME}.elf
	SceSysclibForDriver_stub
	SceThreadmgrForDriver_stub
	SceCpuForDriver_stub
	SceCtrlForDriver_stub
	SceUdcdForDriver_stub
	SceIofilemgrForDriver_stub
)

# set_target_properties(${PROJECT_NAME}.elf
# 	PROPERTIES LINK_FLAGS "-nostdlib"
# 	COMPILE_FLAGS "-D__VITA_KERNEL__"
# )

vita_create_self(${PROJECT_NAME}.skprx ${PROJECT_NAME}.elf
	CONFIG exports.yml
	UNSAFE
)

vita_create_stubs(stubs ${PROJECT_NAME}.elf ${CMAKE_SOURCE_DIR}/exports.yml KERNEL)

# install(DIRECTORY ${CMAKE_BINARY_DIR}/stubs/
#   DESTINATION lib
#   FILES_MATCHING PATTERN "*.a"
# )

add_custom_target(send
	COMMAND curl -T ${PROJECT_NAME}.skprx ftp://$(PSVITAIP):1337/ux0:/tai/
	DEPENDS ${PROJECT_NAME}.skprx
)
