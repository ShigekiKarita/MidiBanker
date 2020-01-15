set(plug_sources
    source/version.h
    source/plugfactory.cpp
    source/plugids.h
    source/plugcontroller.h
    source/plugcontroller.cpp
    source/plugprocessor.h
    source/plugprocessor.cpp
    source/devicepatch.h
    source/devicepatch.cpp
)

#--- HERE change the target Name for your plug-in (for ex. set(target myDelay))-------
set(target midibanker)

smtg_add_vst3plugin(${target} ${plug_sources})
# NOTE: this is a tmp test resource
smtg_add_vst3_resource(${target} "resource/Yamaha MU50.txt")

set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(${target} PRIVATE base sdk)

if(SMTG_MAC)
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
    target_sources(${target} PRIVATE resource/plug.rc)
endif()

# require CMake installation
# see https://github.com/catchorg/Catch2/blob/master/docs/cmake-integration.md#installing-catch2-from-git-repository
find_package(Catch2 REQUIRED)
add_executable(
  test_${target}
  source/test_main.cpp
  source/test_parser.cpp)
target_link_libraries(test_${target} Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(test_${target})
