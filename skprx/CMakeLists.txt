cmake_minimum_required(VERSION 2.8)

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_C_COMPILER "arm-vita-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-vita-eabi-g++")

project(hidkeyboard)

set(CMAKE_C_FLAGS "-Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
)

link_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}.elf
	main.c
	hidkeyboard.c
)

target_link_libraries(${PROJECT_NAME}.elf
	gcc
	SceSysclibForDriver_stub
	SceSysmemForDriver_stub
	SceSysmemForKernel_stub
	SceThreadmgrForKernel_stub
	SceThreadmgrForDriver_stub
	SceKernelSuspendForDriver_stub
	SceCpuForDriver_stub
	SceUdcdForDriver_stub
	SceCtrlForDriver_stub
)

set_target_properties(${PROJECT_NAME}.elf
	PROPERTIES LINK_FLAGS "-nostdlib"
	COMPILE_FLAGS "-D__VITA_KERNEL__"
)

add_custom_target(${PROJECT_NAME}.skprx ALL
	COMMAND vita-elf-create -e ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.yml ${PROJECT_NAME}.elf ${PROJECT_NAME}.velf
	COMMAND vita-make-fself ${PROJECT_NAME}.velf ${PROJECT_NAME}.skprx
)
add_dependencies(${PROJECT_NAME}.skprx ${PROJECT_NAME}.elf)

add_custom_target(send
	COMMAND curl -T ${PROJECT_NAME}.skprx ftp://${PSVITAIP}:1337/ux0:/tai/
	DEPENDS ${PROJECT_NAME}.skprx
)
