#IF(NOT MIR2X_BUILD_LIBRIME)
    RETURN()
#ENDIF()

MESSAGE(STATUS "Begin to build librime...")

#INCLUDE(C:/mywork/projects/cpp/librime/install/share/cmake/rime/RimeConfig.cmake)
#SET(Rime_INCLUDE_DIR "C:/mywork/projects/cpp/librime/install/include")
#SET(Rime_LIBDIR "C:/mywork/projects/cpp/librime/install/lib")
#SET(Rime_LIB "${CMAKE_STATIC_LIBRARY_PREFIX}rime${CMAKE_STATIC_LIBRARY_SUFFIX}")
#LINK_DIRECTORIES(${Rime_LIBDIR})
#MESSAGE(STATUS "------${Rime_INCLUDE_DIR}---------${Rime_LIBDIR}----${Rime_LIB}")
FIND_PACKAGE(PkgConfig REQUIRED)
SET(ENV{PKG_CONFIG_PATH} C:/mywork/projects/cpp/librime/install/lib/pkgconfig:$ENV{PKG_CONFIG_PATH})
MESSAGE(STATUS "Update PKG_CONFIG_PATH: $ENV{PKG_CONFIG_PATH}")

RETURN()
INCLUDE(FetchContent)

IF(WIN32)
    MESSAGE(STATUS "We need to build berkeleydb manually")
    SET(LIBDB_INCLUDE "C:/mywork/projects/cpp/berkeleydb/build_windows")
    IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
        SET(LIBDB_LIBS "C:/mywork/projects/cpp/berkeleydb/build_windows/x64/Debug")
    ELSEIF (CMAKE_BUILD_TYPE STREQUAL "Release")
        SET(LIBDB_LIBS "C:/mywork/projects/cpp/berkeleydb/build_windows/x64/Release")
    ELSE ()
        MESSAGE(FATAL_ERROR "Not supported build type: ${CMAKE_BUILD_TYPE}")
    ENDIF ()
ELSE()
    FetchContent_Declare(
        libdb

        GIT_REPOSITORY https://github.com/berkeleydb/libdb.git
        GIT_TAG        master

        GIT_PROGRESS 1
    )

    FetchContent_MakeAvailable(libdb)

    IF(NOT DEFINED libdb_CONFIGURED)
        MESSAGE(STATUS "Configuring libdb")
        EXECUTE_PROCESS(
            COMMAND ${libdb_SOURCE_DIR}/dist/configure --prefix=${MIR2X_3RD_PARTY_DIR}/libdb/build --enable-shared=no
            WORKING_DIRECTORY ${libdb_BINARY_DIR}

            RESULT_VARIABLE libdb_CONFIGURED

            OUTPUT_QUIET
            COMMAND_ERROR_IS_FATAL ANY
        )
    ENDIF()

    IF(NOT DEFINED libdb_INSTALLED)
        MESSAGE(STATUS "Building libdb")
        EXECUTE_PROCESS(
            COMMAND make install
            WORKING_DIRECTORY ${libdb_BINARY_DIR}

            RESULT_VARIABLE libdb_INSTALLED

            OUTPUT_QUIET
            ERROR_QUIET
        )
    ENDIF()

    SET(LIBDB_INCLUDE ${MIR2X_3RD_PARTY_DIR}/libdb/build/include)
    SET(LIBDB_LIBS ${MIR2X_3RD_PARTY_DIR}/libdb/build/lib)
ENDIF ()

FetchContent_Declare(
    libpinyin

    GIT_REPOSITORY https://github.com/kindred77/libpinyin.git
    GIT_TAG        main

    GIT_PROGRESS 1
)

SET(DB_FOUND TRUE)
SET(DB_INCLUDE_DIR ${LIBDB_INCLUDE})
SET(DB_LIBRARIES ${LIBDB_LIBS})
FetchContent_MakeAvailable(libpinyin)

IF(NOT DEFINED libpinyin_CONFIGURED)
    MESSAGE(STATUS "Configuring libpinyin")
    EXECUTE_PROCESS(
        COMMAND env CFLAGS=-I${LIBDB_INCLUDE} CXXFLAGS=-I${LIBDB_INCLUDE} LIBS=-L${LIBDB_LIBS} sh ${libpinyin_SOURCE_DIR}/autogen.sh --prefix=${MIR2X_3RD_PARTY_DIR}/libpinyin/build --enable-shared=no
        WORKING_DIRECTORY ${libpinyin_SOURCE_DIR}

        RESULT_VARIABLE libpinyin_CONFIGURED

        OUTPUT_QUIET
        COMMAND_ERROR_IS_FATAL ANY
    )
ENDIF()

IF(NOT DEFINED libpinyin_INSTALLED)
    MESSAGE(STATUS "Building libpinyin")
    EXECUTE_PROCESS(
        COMMAND make install
        WORKING_DIRECTORY ${libpinyin_SOURCE_DIR}

        RESULT_VARIABLE libpinyin_INSTALLED

        OUTPUT_QUIET
        ERROR_QUIET
    )
ENDIF()

SET(ENV{PKG_CONFIG_PATH} ${MIR2X_3RD_PARTY_DIR}/libpinyin/build/lib/pkgconfig:$ENV{PKG_CONFIG_PATH})
MESSAGE(STATUS "Update PKG_CONFIG_PATH: $ENV{PKG_CONFIG_PATH}")
