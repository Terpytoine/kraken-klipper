cmake_minimum_required(VERSION 3.22)
project(KrakenKlipper VERSION 1.0.1 LANGUAGES C CXX)

# Link the Windows runtime statically so the VST3 does not require a separate
# Visual C++ runtime installer on a friend's computer.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(FetchContent)
FetchContent_Declare(
    JUCE
    URL https://github.com/juce-framework/JUCE/archive/refs/tags/9.0.3.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(KrakenKlipper
    COMPANY_NAME "Bay Area Plugins"
    BUNDLE_ID com.bayareaplugins.doublecupclipper
    PLUGIN_MANUFACTURER_CODE TODB
    PLUGIN_CODE Kk01
    PRODUCT_NAME "DOUBLE CUP CLIPPER"
    FORMATS VST3
    VST3_CATEGORIES Fx
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(KrakenKlipper)

target_sources(KrakenKlipper PRIVATE
    Source/ClipperCurve.h
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
)

target_compile_features(KrakenKlipper PRIVATE cxx_std_17)
target_compile_definitions(KrakenKlipper PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(KrakenKlipper PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

include(CTest)
if(BUILD_TESTING)
    add_executable(KrakenCurveChecks tests/curve_checks.cpp)
    target_include_directories(KrakenCurveChecks PRIVATE Source)
    target_compile_features(KrakenCurveChecks PRIVATE cxx_std_17)
    add_test(NAME KrakenCurveChecks COMMAND KrakenCurveChecks)

    add_executable(DoubleCupProcessorChecks tests/processor_checks.cpp)
    target_include_directories(DoubleCupProcessorChecks PRIVATE
        Source
        "${CMAKE_CURRENT_BINARY_DIR}/KrakenKlipper_artefacts/JuceLibraryCode"
    )
    target_link_libraries(DoubleCupProcessorChecks PRIVATE KrakenKlipper)
    target_compile_features(DoubleCupProcessorChecks PRIVATE cxx_std_17)
    add_test(NAME DoubleCupProcessorChecks COMMAND DoubleCupProcessorChecks)
endif()
