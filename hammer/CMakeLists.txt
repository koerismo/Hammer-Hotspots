cmake_minimum_required(VERSION 3.10)
project(hamspot VERSION 1.0.0)

add_library(hamspot SHARED
	src/patcher/main.cpp
	src/module.cpp
	src/faceedit.cpp
)

add_executable(hamspot-launcher
	src/launcher/launcher.cpp
	src/module.cpp
	src/faceedit.cpp
)

# find_package(MinHook REQUIRED)
# target_link_libraries(main PRIVATE MinHook::MinHook)

# set(MINHOOK_LIB "${CMAKE_CURRENT_SOURCE_DIR}/ext/minhook/lib/libMinHook.x86.lib")
set(MINHOOK_LIB "${CMAKE_CURRENT_SOURCE_DIR}/ext/minhook/lib/libMinHook.x64.lib")

target_link_libraries(hamspot ${MINHOOK_LIB})
target_link_libraries(hamspot-launcher ${MINHOOK_LIB})

# Set output version
set_target_properties(hamspot PROPERTIES VERSION "${PROJECT_VERSION}")
set_target_properties(hamspot-launcher PROPERTIES VERSION "${PROJECT_VERSION}")

# Give outputs much cooler names
set_target_properties(hamspot PROPERTIES OUTPUT_NAME "ham_dll")
set_target_properties(hamspot-launcher PROPERTIES OUTPUT_NAME "HamSpotInjector")

set(HAMSPOT_INCLUDES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/ext/minhook/include/"
)

target_include_directories(hamspot PRIVATE ${HAMSPOT_INCLUDES})
target_include_directories(hamspot-launcher PRIVATE ${HAMSPOT_INCLUDES})
