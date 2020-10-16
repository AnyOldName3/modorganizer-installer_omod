cmake_minimum_required(VERSION 3.16)
if(DEFINED DEPENDENCIES_DIR)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/src.cmake)
else()
	include(../../cmake_common/src.cmake)
endif()

requires_project(game_features)

# I'd like to use get_target_property(source_files ${PROJECT_NAME} SOURCES) as globbing is naughty, but need to filter out the things that aren't relative to this directory.
file(GLOB_RECURSE source_files CONFIGURE_DEPENDS *.cpp;*.h;*.ui)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src FILES ${source_files})