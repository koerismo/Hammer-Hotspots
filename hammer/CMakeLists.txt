cmake_minimum_required(VERSION 3.10)
project(hamspot VERSION 1.0.0)

add_library(hamspot SHARED
	src/patcher/main.cpp
	src/module.cpp
	src/hotspot.cpp
	src/faceedit.cpp
)

# REMOVE ME!!!!
# set(IS_X86 1)
# set(IS_P2 1)
set(IS_X86 0)
set(IS_P2CE 1)

# Set this when using this as a hammer_dll passthrough
set(PASSTHRU 0)

# Configure for P2CE Hammer
add_compile_definitions(PASSTHRU=${PASSTHRU})

if (IS_P2CE)
add_compile_definitions(GAME_P2CE=1)
set(TIER0_PATH "tier0-p2ce-x64")
elseif(IS_P2)
add_compile_definitions(GAME_P2=1)
set(TIER0_PATH "tier0-p2-x86")
else()
message(FATAL_ERROR "No game target selected! Use the provided CMake presets!")
endif()

set(TIER0_LIB "${CMAKE_CURRENT_SOURCE_DIR}/ext/${TIER0_PATH}/lib/tier0.lib")
if (IS_X86)
set(DLL_POSTFIX "_x86")
set(MINHOOK_LIB "${CMAKE_CURRENT_SOURCE_DIR}/ext/minhook/lib/libMinHook.x86.lib")
else()
set(DLL_POSTFIX "_x64")
set(MINHOOK_LIB "${CMAKE_CURRENT_SOURCE_DIR}/ext/minhook/lib/libMinHook.x64.lib")
endif()

target_link_libraries(hamspot ${MINHOOK_LIB} ${TIER0_LIB})

# Set output version
set_target_properties(hamspot PROPERTIES VERSION "${PROJECT_VERSION}")

if(PASSTHRU)
	add_compile_definitions(PASSTHRU=1)
	set(DLL_NAME "hammer_dll")
else()
	set(DLL_NAME "hamspot")
endif()

# Give outputs much cooler names
set_target_properties(hamspot PROPERTIES OUTPUT_NAME "${DLL_NAME}${DLL_POSTFIX}")

set(HAMSPOT_INCLUDES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/ext/minhook/include/"
	"${CMAKE_CURRENT_SOURCE_DIR}/ext/${TIER0_PATH}/include/"
)

target_include_directories(hamspot PRIVATE ${HAMSPOT_INCLUDES})
