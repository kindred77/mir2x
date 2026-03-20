IF(NOT MIR2X_BUILD_LIBRIME)
    RETURN()
ENDIF()

MESSAGE(STATUS "Add librime...")

INCLUDE(ExternalProject)

# for the CMAKE_ARGS, the author recommends -DCPACK_PACKAGING_INSTALL_PREFIX=xxx
# but seems on windows this doesn't work

ExternalProject_Add(
        librime

        GIT_REPOSITORY "https://github.com/rime/librime.git"
#        GIT_TAG        "master"
        GIT_CLONE_FLAGS   "--recurse-submodules --timeout=6000"     # 递归子模块 + 浅克隆（加快速度）
        #PATCH_COMMAND     git submodule update --init --recursive --force

        SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/librime"
        INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/librime/build"

#        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/librime/build/install -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC=ON -DBUILD_TEST=OFF

        LOG_DOWNLOAD      1
        LOG_BUILD 1
        LOG_CONFIGURE 1
        LOG_INSTALL 1
)

SET(RIME_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/librime/build/install/include")
SET(RIME_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}librime${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${RIME_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/librime/build/install/lib)

MESSAGE(STATUS "librime include ${RIME_INCLUDE_DIRS}, lib: ${MIR2X_3RD_PARTY_DIR}/librime/build/install/lib/${RIME_LIBRARIES}")
#ADD_DEPENDENCIES(mir2x_3rds librime)
