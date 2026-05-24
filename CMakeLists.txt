cmake_minimum_required(VERSION 3.21)

########################################################################################################################
## Validate CommonLibSSE-ng version
########################################################################################################################

set(VERSION_CONF_FILENAME "version.conf")
set(COMMONLIBSSE_PORT_DIR "${CMAKE_SOURCE_DIR}/cmake/ports/commonlibsse-ng")
set(LOCAL_VERSION_CONF "${COMMONLIBSSE_PORT_DIR}/${VERSION_CONF_FILENAME}")
set(SHARED_VERSION_CONF "${VCPKG_INSTALLED_DIR}/${VERSION_CONF_FILENAME}")

file(READ "${LOCAL_VERSION_CONF}" LOCAL_VERSION_INFO)
string(REGEX MATCH "COMMONLIBSSE_REF=([a-f0-9]+)" _ "${LOCAL_VERSION_INFO}")
set(COMMONLIBSSE_REF "${CMAKE_MATCH_1}")
string(REGEX MATCH "COMMONLIBSSE_DATE=([0-9/]+)" _ "${LOCAL_VERSION_INFO}")
set(COMMONLIBSSE_DATE "${CMAKE_MATCH_1}")

message("\n\n-- Configuring CommonLibSSE-NG commit: ${COMMONLIBSSE_REF} (${COMMONLIBSSE_DATE})")

if(NOT EXISTS "${SHARED_VERSION_CONF}")
    file(WRITE "${SHARED_VERSION_CONF}" "${LOCAL_VERSION_INFO}")
    message("${VERSION_CONF_FILENAME} copied from ${COMMONLIBSSE_PORT_DIR} to ${VCPKG_INSTALLED_DIR}.")
else()
    file(READ "${SHARED_VERSION_CONF}" SHARED_VERSION_INFO)
    if(NOT "${LOCAL_VERSION_INFO}" STREQUAL "${SHARED_VERSION_INFO}")

        string(REGEX MATCH "COMMONLIBSSE_REF=([a-f0-9]+)" _ "${SHARED_VERSION_INFO}")
		set(SHARED_COMMONLIBSSE_REF "${CMAKE_MATCH_1}")
		string(REGEX MATCH "COMMONLIBSSE_DATE=([0-9/]+)" _ "${SHARED_VERSION_INFO}")
		set(SHARED_COMMONLIBSSE_DATE "${CMAKE_MATCH_1}")

        message(FATAL_ERROR 
            " CommonLibSSE-NG commit mismatch!\n"
            " Expected: ${COMMONLIBSSE_REF} (${COMMONLIBSSE_DATE})\n"
            " Installed: ${SHARED_COMMONLIBSSE_REF} (${SHARED_COMMONLIBSSE_DATE})"
        )
    endif()
endif()

message("Configuration valid!\n")

########################################################################################################################
## Define project
########################################################################################################################

project(
	InfinityUI
	VERSION 2.1.0
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

file(GLOB_RECURSE headers "include/*.h")
file(GLOB_RECURSE sources "source/*.cpp")

source_group(
	TREE
		${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${headers}
		${sources}
)

########################################################################################################################
## Configure target DLL
########################################################################################################################

find_package(CommonLibSSE CONFIG REQUIRED)
find_package(spdlog REQUIRED CONFIG)
find_package(directxtk CONFIG REQUIRED)

if(EXISTS "${VCPKG_INSTALLED_DIR}/vcpkg")
    message(STATUS "Removing vcpkg cache during configure...")
    file(REMOVE_RECURSE "${VCPKG_INSTALLED_DIR}/vcpkg")
endif()

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${headers} ${sources})
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		CommonLibSSE::CommonLibSSE
		spdlog::spdlog_header_only
		Microsoft::DirectXTK
)

if (MSVC)
	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF>"
	)
endif()

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		include/PCH.h
)

target_sources(
	${PROJECT_NAME}
	INTERFACE
	"$<BUILD_INTERFACE:CommonLibSSE.natvis>"
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
)


########################################################################################################################
## Automatic plugin deployment
########################################################################################################################

set(SKYRIM_SE_DIR "C:/Program Files (x86)/Steam/steamapps/common/Skyrim Special Edition")
set(SKYRIM_VR_DIR "C:/Program Files (x86)/Steam/steamapps/common/Skyrim VR")

if ("$ENV{RUNTIME_DISABLE_FLAGS}" STREQUAL "")
	set(COPY_SE YES)
	set(COPY_VR YES)
elseif ("$ENV{RUNTIME_DISABLE_FLAGS}" STREQUAL "-UENABLE_SKYRIM_VR")
	set(COPY_SE YES)
	set(COPY_VR NO)
elseif ("$ENV{RUNTIME_DISABLE_FLAGS}" STREQUAL "-UENABLE_SKYRIM_SE -UENABLE_SKYRIM_AE")
	set(COPY_SE NO)
	set(COPY_VR YES)
endif()

if (COPY_SE AND EXISTS "${SKYRIM_SE_DIR}/Data/SKSE/Plugins")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}>
		"${SKYRIM_SE_DIR}/Data/SKSE/Plugins/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
	)
endif()
if (COPY_VR AND EXISTS "${SKYRIM_VR_DIR}/Data/SKSE/Plugins")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}>
		"${SKYRIM_VR_DIR}/Data/SKSE/Plugins/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
	)
endif()
