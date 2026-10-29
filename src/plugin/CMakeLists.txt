option(FAULTCOUNTER_BUILD_PLUGIN "Build the JUCE VST3 plug-in" ON)

if(FAULTCOUNTER_BUILD_PLUGIN)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    set(faultcounter_tables_embed "${CMAKE_CURRENT_BINARY_DIR}/TablesData.cpp")
    add_custom_command(
        OUTPUT "${faultcounter_tables_embed}"
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../../tools/tables/embed_tables.py"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../assets/tables/procedural.bin"
                "${faultcounter_tables_embed}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../../assets/tables/procedural.bin"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../tools/tables/embed_tables.py"
        COMMENT "Embedding procedural transfer tables"
        VERBATIM)

    juce_add_plugin(FaultCounter
        VERSION 0.1.0
        COMPANY_NAME "FaultCounter"
        BUNDLE_ID com.faultcounter.vst3
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PLUGIN_MANUFACTURER_CODE Fcnt
        PLUGIN_CODE Fcnt
        FORMATS VST3
        VST3_CATEGORIES Fx Distortion
        PRODUCT_NAME "Fault Counter")

    target_sources(FaultCounter PRIVATE
        ../PluginProcessor.cpp
        "${faultcounter_tables_embed}")

    # juce_add_plugin compiles plug-in sources via the shared-code target; the
    # generated tables translation unit must land there too or the VST3/test
    # link misses kEmbeddedTables.
    if(TARGET FaultCounter_SharedCode)
        target_sources(FaultCounter_SharedCode PRIVATE "${faultcounter_tables_embed}")
    endif()

    # Linux X11 extension feature detection: JUCE dynamically loads these
    # libraries at runtime, but the headers must exist at compile time. Missing
    # headers disable the corresponding optional feature rather than failing
    # the build (sandbox environments may lack the -dev packages).
    include(CheckIncludeFileCXX)
    check_include_file_cxx("X11/extensions/Xrandr.h" FAULTCOUNTER_HAVE_XRANDR_H)
    check_include_file_cxx("X11/extensions/Xinerama.h" FAULTCOUNTER_HAVE_XINERAMA_H)
    check_include_file_cxx("X11/extensions/Xcomposite.h" FAULTCOUNTER_HAVE_XCOMPOSITE_H)
    check_include_file_cxx("X11/Xcursor/Xcursor.h" FAULTCOUNTER_HAVE_XCURSOR_H)
    check_include_file_cxx("X11/extensions/XInput2.h" FAULTCOUNTER_HAVE_XINPUT2_H)

    add_library(faultcounter_juce_config INTERFACE)
    if(NOT FAULTCOUNTER_HAVE_XRANDR_H)
        target_compile_definitions(faultcounter_juce_config INTERFACE JUCE_USE_XRANDR=0)
    endif()
    if(NOT FAULTCOUNTER_HAVE_XINERAMA_H)
        target_compile_definitions(faultcounter_juce_config INTERFACE JUCE_USE_XINERAMA=0)
    endif()
    if(NOT FAULTCOUNTER_HAVE_XCOMPOSITE_H)
        target_compile_definitions(faultcounter_juce_config INTERFACE JUCE_USE_XRENDER=0)
    endif()
    if(NOT FAULTCOUNTER_HAVE_XCURSOR_H)
        target_compile_definitions(faultcounter_juce_config INTERFACE JUCE_USE_XCURSOR=0)
    endif()
    if(NOT FAULTCOUNTER_HAVE_XINPUT2_H)
        target_compile_definitions(faultcounter_juce_config INTERFACE JUCE_USE_XINPUT=0)
    endif()

    target_link_libraries(FaultCounter PUBLIC
        faultcounter_core
        faultcounter_juce_config
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_gui_extra)

    target_compile_definitions(FaultCounter PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_MODAL_LOOPS_PERMITTED=1)

    target_compile_definitions(FaultCounter PRIVATE
        JUCE_PLUGINHOST_VST3=0)
endif()
