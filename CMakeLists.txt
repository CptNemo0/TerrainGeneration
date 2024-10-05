cmake_minimum_required(VERSION 3.25)
project(ClothSimulation VERSION 0.0.0)

set(CPP_INCLUDE_DIRS "")
set(CPP_SOURCE_FILES "")
set(CPP_HEADER_FILES "")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(ENABLE_WINDXSDK ON)
set(WINDOWKIT_VERSION 10)
set(WINDOWKIT_VERSION_UPDATE 10.0.26100)
set(WINDOW_BIT x64) 

set(ENABLE_IMGUI ON)

set(PROGRAMFILES_PATH $ENV{ProgramFiles\(x86\)})
set(WINDOW_KITS "${PROGRAMFILES_PATH}/Windows Kits/${WINDOWKIT_VERSION}")

set(WINDOW_DXSDK_INCULDE "${WINDOW_KITS}/Include/${WINDOWKIT_VERSION}/um")

set(WINDOW_DXSDK_LIB "${WINDOW_KITS}/Include/${WINDOWKIT_VERSION}/um/${WINDOW_BIT}")

list(APPEND CPP_INCLUDE_DIRS ${WINDOW_DXSDK_INCULDE})
link_directories(${WINDOW_DXSDK_LIB})

include(FetchContent)
set(FETCHCONTENT_QUIET NO)

################################################ imgui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG a9f72ab6818c3e55544378aa44c7659de7e5510f)

FetchContent_GetProperties(imgui)
FetchContent_Populate(imgui)
FetchContent_MakeAvailable(imgui)
################################################ end imgui

################################################ glm
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG 0af55ccecd98d4e5a8d1fad7de25ba429d60e863)

FetchContent_GetProperties(glm)
FetchContent_Populate(glm)
FetchContent_MakeAvailable(glm)
################################################ end glm

#add source files
include_directories(${CPP_INCLUDE_DIRS})

list(APPEND CPP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
list(APPEND CPP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Shader.cc)
list(APPEND CPP_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/Shader.h)

# Search for the assets files
file(GLOB_RECURSE ASSETS_FILES 
	 ${CMAKE_SOURCE_DIR}/res/*.*)
set_source_files_properties(${ASSETS_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group(TREE ${CMAKE_SOURCE_DIR}         FILES ${ASSETS_FILES})

add_executable(${PROJECT_NAME} 
  WIN32
  ${CPP_HEADER_FILES}
  ${CPP_SOURCE_FILES}
  ${ASSETS_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${glm_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${imgui_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} d3d11)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
				   COMMAND ${CMAKE_COMMAND} -E create_symlink 
				   ${CMAKE_SOURCE_DIR}/res 
				   ${CMAKE_CURRENT_BINARY_DIR}/res)