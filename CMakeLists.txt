# CMake 3.18 due to https://gitlab.kitware.com/cmake/cmake/-/issues/20764
cmake_minimum_required(VERSION 3.18)

# set globally as Nuget gets confused about ZERO_CHECK, ALL_BUILD and INSTALL otherwise
set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8")

project(installer_omod LANGUAGES CXX CSharp)
set(project_type plugin)
set(enable_warnings OFF)


# Dummy .NET library as VS_PACKAGE_REFERENCES doesn't work on C++/CLI projects yet
# Needs to be declared before cmake_common stuff is included as that polutes the environment and makes C# get compiled as C++
# This only mostly works - you need to build via Visual Studio or run `msbuild -t:restore installer_omod.sln` at least once before this will build via the command line due to https://gitlab.kitware.com/cmake/cmake/-/issues/20646
add_library(dummy_cs_project SHARED DummyCSFile.cs)
set_target_properties(dummy_cs_project PROPERTIES
	LINKER_LANGUAGE CSharp
	VS_PACKAGE_REFERENCES "OMODFramework_2.2.0;OMODFramework.Scripting_2.2.0;RtfPipe_1.0.7388.1242"
)

if(DEFINED DEPENDENCIES_DIR)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/project.cmake)
else()
	include(../cmake_common/project.cmake)
endif()
add_subdirectory(src)

# Ideally we'd use "$<TARGET_FILE_DIR:dummy_cs_project>", but the relevant property doesn't support generator expressions.
set(omod_framework_prefix "${CMAKE_BINARY_DIR}/$(Configuration)" CACHE PATH "Path where OMODFramework.dll and OMODFramework.Scripting.dll can be found. The default value is where the nuget package artifacts will end up. Override if you want to use a custom build.")

set_target_properties(${PROJECT_NAME} PROPERTIES
	COMMON_LANGUAGE_RUNTIME ""
	# latest doesn't work with CLR
	CXX_STANDARD 17
	#DOTNET_TARGET_FRAMEWORK "netstandard2.0"
	VS_DOTNET_REFERENCE_OMODFramework "${omod_framework_prefix}/OMODFramework.dll"
	VS_DOTNET_REFERENCE_OMODFramework.Scripting "${omod_framework_prefix}/OMODFramework.Scripting.dll"
	VS_DOTNET_REFERENCE_RtfPipe "${CMAKE_BINARY_DIR}/$(Configuration)/RtfPipe.dll"
	VS_DOTNET_REFERENCES "System.dll;System.Core.dll"
)

# C++/CLI doesn't support two-phase lookup yet
target_compile_options(${PROJECT_NAME} PRIVATE "/Zc:twoPhase-")

# OMODFramework and OMODFramework.Scripting reference different .NET standard libraries. This generates warnings when using them together.
target_compile_options(${PROJECT_NAME} PRIVATE "/wd4691")

# We don't want ERROR defined
target_compile_definitions(${PROJECT_NAME} PRIVATE "NOGDI")

# We don't need to actually link with dummy_cs_project, especially as its dependencies aren't pulled in. We do need it to build first, though.
add_dependencies(${PROJECT_NAME} dummy_cs_project)

install(FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}>/OMODFramework.dll" DESTINATION "${install_dir}/data")
install(FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}>/OMODFramework.Scripting.dll" DESTINATION "${install_dir}/data")
install(FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}>/ICSharpCode.SharpZipLib.dll" DESTINATION "${install_dir}/data")
install(FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}>/System.Drawing.Common.dll" DESTINATION "${install_dir}/data")
install(FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}>/RtfPipe.dll" DESTINATION "${install_dir}/data")
install(FILES "$<TARGET_PDB_FILE_DIR:${PROJECT_NAME}>/ICSharpCode.SharpZipLib.pdb" DESTINATION pdb)
# Other PDB files get downloaded by Visual Studio during debugging when https://symbols.nuget.org/download/symbols is in the symbol server list
