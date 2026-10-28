cmake_minimum_required(VERSION 3.30.5)

project(FaultCounter VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(FAULTCOUNTER_BUILD_TESTS "Build FaultCounter tests" ON)

include(cmake/Dependencies.cmake)

add_library(faultcounter_core STATIC
    src/Parameters.cpp
    src/dsp/BitOps.cpp
    src/dsp/SampleHold.cpp
    src/dsp/Tables.cpp
    src/dsp/Preamp.cpp
    src/dsp/Tone.cpp
    src/dsp/DcBlocker.cpp
    src/dsp/Engine.cpp
)

target_include_directories(faultcounter_core PUBLIC src)
target_compile_features(faultcounter_core PUBLIC cxx_std_20)
set_target_properties(faultcounter_core PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

if(FAULTCOUNTER_BUILD_TESTS)
    enable_testing()
    add_executable(FaultCounterTests
        tests/unit/BitOpsTest.cpp
        tests/unit/SampleHoldTest.cpp
        tests/unit/TablesTest.cpp
        tests/unit/EngineRoutingTest.cpp
        tests/unit/PreampToneDcTest.cpp
        tests/integration/EngineGoldenTest.cpp
    )
    target_link_libraries(FaultCounterTests PRIVATE faultcounter_core Catch2::Catch2WithMain)
    target_compile_definitions(FaultCounterTests PRIVATE
        FAULTCOUNTER_TABLE_ASSET_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/tables/procedural.bin"
    )
    include(Catch)
    catch_discover_tests(FaultCounterTests)
endif()

add_subdirectory(src/plugin)
