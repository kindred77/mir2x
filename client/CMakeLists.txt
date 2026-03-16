ADD_SUBDIRECTORY(src)

ADD_EXECUTABLE(IMETest IMETest.cpp)
ADD_DEPENDENCIES(IMETest mir2x_3rds)

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