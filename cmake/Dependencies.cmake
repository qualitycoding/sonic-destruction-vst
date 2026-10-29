include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG 08092139210881f01a34e60c5f2fd6afa0ea4024
    GIT_SHALLOW TRUE
)

set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(Catch2)

if(FAULTCOUNTER_BUILD_PLUGIN OR FAULTCOUNTER_BUILD_TESTS)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 3fb5cbc0340758596dac39f7c8de47d562b4896
        GIT_SHALLOW FALSE
    )

    FetchContent_MakeAvailable(JUCE)
endif()
