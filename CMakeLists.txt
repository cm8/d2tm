cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(d2tm)

SET(OpenGL_GL_PREFERENCE "GLVND")

file(GLOB d2tm_SRC
        "*.h"
        "*.cpp"
        "building/*.h"
        "building/*.cpp"
        "controls/*.h"
        "controls/*.cpp"
        "controls/mousestates/*.h"
        "controls/mousestates/*.cpp"
        "data/*.h"
        "data/*.cpp"
        "drawers/*.h"
        "drawers/*.cpp"
        "gameobjects/*.h"
        "gameobjects/*.cpp"
        "gameobjects/structures/*.h"
        "gameobjects/structures/*.cpp"
        "gameobjects/units/*.h"
        "gameobjects/units/*.cpp"
        "gameobjects/projectiles/*.h"
        "gameobjects/projectiles/*.cpp"
        "gameobjects/particles/*.h"
        "gameobjects/particles/*.cpp"
        "observers/*.h"
        "observers/*.cpp"
        "gamestates/*.h"
        "gamestates/*.cpp"
        "gui/*.h"
        "gui/*.cpp"
        "gui/actions/*.h"
        "gui/actions/*.cpp"
        "include/*.h"
        "mentat/*.h"
        "mentat/*.cpp"
        "managers/*.h"
        "managers/*.cpp"
        "map/*.h"
        "map/*.cpp"
        "player/*.h"
        "player/*.cpp"
        "player/brains/*.h"
        "player/brains/*.cpp"
        "player/brains/actions/*.h"
        "player/brains/actions/*.cpp"
        "player/brains/missions/*.h"
        "player/brains/missions/*.cpp"
        "player/brains/missions/campaign/*.h"
        "player/brains/missions/campaign/*.cpp"
        "player/brains/missions/skirmish/*.h"
        "player/brains/missions/skirmish/*.cpp"
        "player/brains/superweapon/*.h"
        "player/brains/superweapon/*.cpp"
        "player/*.h"
        "player/*.cpp"
        "sidebar/*.h"
        "sidebar/*.cpp"
        "upgrade/*.h"
        "upgrade/*.cpp"
        "utils/*.h"
        "utils/*.cpp"
        )

if (UNIX)
  list(APPEND d2tm_SRC "utils/alsa/midi_alsa_seq.cpp")
endif()

add_executable(${PROJECT_NAME}
  ${d2tm_SRC}
  Script.rc
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (UNIX)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
      alleg
      alfont
      fmt::fmt
      stdc++fs
      asound
  )
#  #builds 32-bit binaries, needs 32bit liballeg.so to link to
#  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-L/usr/lib/i386-linux-gnu/")
#  add_definitions( -m32)
#  set(CMAKE_EXE_LINKER_FLAGS         "${CMAKE_EXE_LINKER_FLAGS} -m32")
#  set(CMAKE_SHARED_LIBRARY_C_FLAGS   "${CMAKE_SHARED_LIBRARY_C_FLAGS} -m32")
#  set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CXX_FLAGS} -m32")
else()
  target_link_libraries(${PROJECT_NAME}
  PRIVATE
    allegro
    alfont
    fmt::fmt
    -static-libgcc
    -static-libstdc++
  )
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(D2TM_OPTMIZATION "-O0")
else()
  set(D2TM_OPTMIZATION "-O2")
  set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION TRUE
  )
endif()

target_compile_options(${PROJECT_NAME}
  PRIVATE
    ${D2TM_OPTMIZATION}
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Wnon-virtual-dtor"
    "-Wuseless-cast"
    "-Wzero-as-null-pointer-constant"
)

if (NOT UNIX)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
  )
endif()

add_subdirectory(dependencies)
