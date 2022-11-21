cmake_minimum_required(VERSION 3.2)

# Name of the project (will be the name of the plugin)
project (KEngineCoreNode)

# Essential include files to build a node addon,
# you should add this line in every CMake.js based project.
include_directories(${CMAKE_JS_INC})

include_directories("node_modules/nan")

add_library(${PROJECT_NAME} SHARED 
DataTreeFactory.h
DataTreeFactory.cpp
KEngineCoreWrapper.cpp
)

# This line will give our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Essential library files to link to a node addon,
# you should add this line in every CMake.js based project.
# Define Boost dependencies there.
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

function(add_submodule submodule)
    message("adding submodule ${submodule}")
    cmake_path(ABSOLUTE_PATH submodule OUTPUT_VARIABLE absolute_submodule)
    if (NOT EXISTS ${absolute_submodule})
        message("submodule ${submodule} missing, attempting update")

	set (git_cmd "git")
	#set (git_arg "submodule update --init --recursive -- ${submodule}")
	set (git_arg "submodule update")
	execute_process(COMMAND ${git_cmd} submodule update --init --recursive -- ${submodule}
  			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  			RESULT_VARIABLE git_result
  			OUTPUT_VARIABLE git_out)
	
	message(STATUS "git[${git_result}]: ${git_out}")
    endif()
    add_subdirectory(${submodule})
endfunction()

set (KENGINE_CORE_DELETE_STRINGS_RELEASE OFF CACHE BOOL "Delete strings from hashes in release builds" FORCE)
add_submodule(KEngineCore)
target_link_libraries(${PROJECT_NAME} KEngineCore)

