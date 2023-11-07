cmake_minimum_required(VERSION 3.12)
project(SteamIntegrationLib)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_STEAM "Build Steamworks. If off, will build stubs for distributing a non-steam version of the game." ON)

if(MSVC)
    set(INSTALL_FOLDER windows)
elseif(APPLE)
    set(INSTALL_FOLDER macos)
elseif(UNIX)
    set(INSTALL_FOLDER linux)
endif()

set(CMAKE_MACOSX_RPATH TRUE)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path")
else()
    set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()

# Define the shared library target
add_library(steam_integration SHARED steam_integration.cpp)

# Set the output name and version for the shared library
set_target_properties(steam_integration PROPERTIES
    OUTPUT_NAME steam_integration
)

if(BUILD_STEAM)
    message("Building a Steam version of the library.")
    include_directories(${PROJECT_SOURCE_DIR}/steamworks/sdk/public/steam)

    add_definitions(-DBUILD_STEAM=1)

    IF(WIN32)
        SET(STEAM_LIBNAME steam_api64)
        SET(STEAM_RUNTIMENAME steam_api64.dll)
        SET(STEAM_PATHNAME bin/windows)
    ELSEIF(APPLE)
        # universal binary
        SET(STEAM_LIBNAME steam_api)
        SET(STEAM_RUNTIMENAME libsteam_api.dylib)
        SET(STEAM_PATHNAME bin/macos)
    ELSE()
        SET(STEAM_LIBNAME steam_api)
        SET(STEAM_RUNTIMENAME libsteam_api.so)
        SET(STEAM_PATHNAME bin/linux)
    ENDIF()

    FIND_LIBRARY(STEAM_LIBRARY
        NAMES ${STEAM_LIBNAME}
        HINTS
        ${PROJECT_SOURCE_DIR}/${STEAM_PATHNAME}
    )

    FIND_FILE(STEAM_RUNTIME
        NAMES ${STEAM_RUNTIMENAME}
        HINTS
        ${PROJECT_SOURCE_DIR}/${STEAM_PATHNAME}
        PATHS
        ${EXTERNAL_BINARY_PATH}
        ${CMAKE_LIBRARY_PATH}
        /usr/local/lib
        /usr/lib
    )

    target_link_libraries(steam_integration ${STEAM_LIBRARY})
else()
    message("Building a Non-Steam version of the library.")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")

# Set the installation path to a local directory
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin")

# Specify the installation target
install(
    TARGETS steam_integration
    RUNTIME DESTINATION ${INSTALL_FOLDER}
    LIBRARY DESTINATION ${INSTALL_FOLDER}
    ARCHIVE DESTINATION ${INSTALL_FOLDER}
)