SET(IMGUI_HEAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends)
SET(IMGUI_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdlrenderer2.cpp)

ADD_EXECUTABLE(IMETest
        IMETest.cpp
        ${IMGUI_SOURCE}
)
ADD_DEPENDENCIES(IMETest mir2x_3rds)

TARGET_INCLUDE_DIRECTORIES(IMETest PRIVATE ${IMGUI_HEAD_DIR})

TARGET_LINK_LIBRARIES(IMETest
        ${SDL2_LIBRARIES}
        $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
        SDL2::SDL2_gfx
        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
        $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
)

IF(WIN32 AND NOT MSVC)
    TARGET_LINK_LIBRARIES(IMETest ws2_32 dbghelp)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
ENDIF()

#PKG_CHECK_MODULES(RIME REQUIRED rime)
SET(RIME_INCLUDE_DIRS "/home/kindred/mywork/projects/cpp/librime/install/include")
SET(RIME_LIBRARY_DIRS "/home/kindred/mywork/projects/cpp/librime/install/lib")
SET(RIME_LIBRARIES "rime")
LINK_DIRECTORIES(${RIME_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(SYSTEM ${RIME_INCLUDE_DIRS})
MESSAGE(STATUS "------${RIME_INCLUDE_DIRS}---------${RIME_LIBRARY_DIRS}----${RIME_LIBRARIES}---")
#MESSAGE(STATUS "------${RIME_INCLUDE_DIRS}---------${RIME_LIBRARIES}-------${RIME_CFLAGS_OTHER}")

ADD_EXECUTABLE(RIMETest
        RIMETest.cpp
)
ADD_DEPENDENCIES(RIMETest mir2x_3rds)
TARGET_INCLUDE_DIRECTORIES(IMETest PRIVATE ${Rime_INCLUDE_DIR})

#FIND_PACKAGE(glog REQUIRED)
#SET(RIME_LIBRARIES ${RIME_LIBRARIES} glog::glog)
#
#FIND_PACKAGE(leveldb REQUIRED)
#SET(RIME_LIBRARIES ${RIME_LIBRARIES} leveldb)
#
#FIND_PACKAGE(marisa REQUIRED)
#SET(RIME_LIBRARIES ${RIME_LIBRARIES} marisa)
#
#FIND_PACKAGE(opencc REQUIRED)
#SET(RIME_LIBRARIES ${RIME_LIBRARIES} opencc)
#
#FIND_PACKAGE(yaml-cpp REQUIRED)
#SET(RIME_LIBRARIES ${RIME_LIBRARIES} yaml-cpp)
#
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
TARGET_LINK_LIBRARIES(RIMETest
#        ${SDL2_LIBRARIES}
#        $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
#        SDL2::SDL2_gfx
#        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
#        $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
        ${RIME_LIBRARIES}
)
IF(WIN32 AND NOT MSVC)
    TARGET_LINK_LIBRARIES(RIMETest ws2_32 dbghelp)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
ENDIF()

IF(MIR2X_BUILD_LIBRIME)
    ADD_COMPILE_DEFINITIONS(
            USE_LIBRIME
    )
ENDIF ()

ADD_SUBDIRECTORY(src)