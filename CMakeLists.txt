cmake_minimum_required(VERSION 3.16)

# set globally as Nuget gets confused about ZERO_CHECK and ALL_BUILD otherwise
set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8")
# This doesn't fix the INSTALL target - that still needs doing manually

# Dummy .NET project as VS_PACKAGE_REFERENCES doesn't work on C++/CLI projects yet
project(dummy_cs_project LANGUAGES CSharp)
add_library(${PROJECT_NAME} SHARED DummyCSFile.cs)
set_target_properties(${PROJECT_NAME} PROPERTIES
	VS_PACKAGE_REFERENCES "OMODFramework_2.0.0;OMODFramework.Scripting_2.0.0;SharpZipLib_1.2.0;System.Drawing.Common_4.7.0"
)



project(installer_omod)
set(project_type plugin)
set(enable_warnings OFF)

if(DEFINED DEPENDENCIES_DIR)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/project.cmake)
else()
	include(../cmake_common/project.cmake)
endif()
add_subdirectory(src)

set_target_properties(${PROJECT_NAME} PROPERTIES
	COMMON_LANGUAGE_RUNTIME ""
	# latest doesn't work with CLR
	CXX_STANDARD 17
	#DOTNET_TARGET_FRAMEWORK "netstandard2.0"
	#VS_PACKAGE_REFERENCES "dummy_cs_project;OMODFramework.dll;OMODFramework.Scripting.dll"
	#VS_DOTNET_REFERENCES "dummy_cs_project;OMODFramework;OMODFramework.Scripting"
	# Ideally we'd use "$<TARGET_FILE_DIR:dummy_cs_project>/OMODFramework.dll", but this property doesn't support generator expressions.
	VS_DOTNET_REFERENCE_OMODFramework "${CMAKE_BINARY_DIR}/$(Configuration)/OMODFramework.dll"
	VS_DOTNET_REFERENCE_OMODFramework.Scripting "${CMAKE_BINARY_DIR}/$(Configuration)/OMODFramework.Scripting.dll"
)

# C++/CLI doesn't support two-phase lookup yet
target_compile_options(${PROJECT_NAME} PRIVATE "/Zc:twoPhase-")

# We don't need to actually link with dummy_cs_project, especially as its dependencies aren't pulled in. We do need it to build first, though.
add_dependencies(${PROJECT_NAME} dummy_cs_project)

install(FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}>/OMODFramework.dll" DESTINATION "${install_dir}/data")
install(FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}>/OMODFramework.Scripting.dll" DESTINATION "${install_dir}/data")
install(FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}>/ICSharpCode.SharpZipLib.dll" DESTINATION "${install_dir}/data")
install(FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}>/System.Drawing.Common.dll" DESTINATION "${install_dir}/data")
install(FILES "$<TARGET_PDB_FILE_DIR:${PROJECT_NAME}>/OMODFramework.pdb" DESTINATION pdb)
install(FILES "$<TARGET_PDB_FILE_DIR:${PROJECT_NAME}>/OMODFramework.Scripting.pdb" DESTINATION pdb)
install(FILES "$<TARGET_PDB_FILE_DIR:${PROJECT_NAME}>/ICSharpCode.SharpZipLib.pdb" DESTINATION pdb)
#install(FILES "$<TARGET_PDB_FILE_DIR:${PROJECT_NAME}>/System.Drawing.Common.pdb" DESTINATION pdb)