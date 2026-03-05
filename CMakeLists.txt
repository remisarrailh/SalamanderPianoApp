cmake_minimum_required(VERSION 3.22)

# Force CMP0091 for MSVC runtime library control
cmake_policy(SET CMP0091 NEW)

project(SalamanderPiano VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Force dynamic CRT everywhere on MSVC
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Helper: recursively force dynamic CRT on all targets in a directory
function(force_dynamic_crt DIR)
    get_property(_targets DIRECTORY ${DIR} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_target ${_targets})
        get_target_property(_type ${_target} TYPE)
        if(NOT _type STREQUAL "INTERFACE_LIBRARY")
            set_target_properties(${_target} PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
    endforeach()
    get_property(_subdirs DIRECTORY ${DIR} PROPERTY SUBDIRECTORIES)
    foreach(_subdir ${_subdirs})
        force_dynamic_crt(${_subdir})
    endforeach()
endfunction()

# ── Platform detection ──
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(SALAMANDER_ANDROID TRUE)
else()
    set(SALAMANDER_ANDROID FALSE)
endif()

# ── JUCE ──
add_subdirectory(extern/JUCE)

# ── sfizz (library only, no standalone tools) ──
set(SFIZZ_JACK OFF CACHE BOOL "" FORCE)
set(SFIZZ_RENDER OFF CACHE BOOL "" FORCE)
set(SFIZZ_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(SFIZZ_TESTS OFF CACHE BOOL "" FORCE)
set(SFIZZ_DEMOS OFF CACHE BOOL "" FORCE)
set(SFIZZ_DEVTOOLS OFF CACHE BOOL "" FORCE)
set(SFIZZ_SHARED OFF CACHE BOOL "" FORCE)

if(SALAMANDER_ANDROID)
    set(ENABLE_LTO OFF CACHE BOOL "" FORCE)
endif()

add_subdirectory(extern/sfizz)

# Fix sfizz targets: force dynamic CRT (sfizz defaults to static /MT)
if(MSVC)
    force_dynamic_crt(${CMAKE_CURRENT_SOURCE_DIR}/extern/sfizz)
endif()

# ── Source files ──
set(PLUGIN_SOURCES
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/SfizzEngine.cpp
    src/SampleManager.cpp
    src/MidiDeviceManager.cpp
    src/LookAndFeel.cpp
    src/ui/HeaderPanel.cpp
    src/ui/MidiStatusBar.cpp
    src/ui/ControlPanel.cpp
    src/ui/PianoKeyboard.cpp
    src/ui/TroubleshootPanel.cpp
)

# ── JUCE plugin target ──
juce_add_plugin(SalamanderPiano
    COMPANY_NAME "SalamanderPiano"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_MANUFACTURER_CODE Salp
    PLUGIN_CODE Splp
    FORMATS Standalone VST3
    PRODUCT_NAME "Salamander Piano"
)

target_sources(SalamanderPiano PRIVATE ${PLUGIN_SOURCES})

target_include_directories(SalamanderPiano PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(SalamanderPiano PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=0
    SALAMANDER_VERSION="${PROJECT_VERSION}"
)

if(SALAMANDER_ANDROID)
    target_compile_definitions(SalamanderPiano PUBLIC
        SALAMANDER_ANDROID=1
        JUCE_USE_ANDROID_OBOE=1
        JUCE_USE_ANDROID_OPENSLES=0
        JUCE_CORE_INCLUDE_JNI_HELPERS=1
        # Required: JuceActivity.java always declares appOnResume/appNewIntent as native.
        # JUCE only compiles their C++ implementations when this is defined.
        JUCE_PUSH_NOTIFICATIONS_ACTIVITY="com/rmsl/juce/JuceActivity"
        JUCE_PUSH_NOTIFICATIONS=1
        # Disable kiosk/immersive-sticky mode: it consumes every first tap to
        # show/hide the nav bar, making the app nearly unresponsive.
        JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE=0
    )
    # JUCE's Java bootstrap loads "libjuce_jni.so" - rename the Standalone target accordingly
    set_target_properties(SalamanderPiano_Standalone PROPERTIES OUTPUT_NAME "juce_jni")
endif()

target_link_libraries(SalamanderPiano
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_dsp
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        sfizz::sfizz
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

# ── Install rules (Windows) ──
if(NOT SALAMANDER_ANDROID)
    # Standalone EXE → <install>/
    install(TARGETS SalamanderPiano_Standalone
        RUNTIME DESTINATION .
        COMPONENT Standalone)

    # SFZ data → <install>/sfz/
    install(DIRECTORY sfz/
        DESTINATION sfz
        COMPONENT Standalone
        OPTIONAL)

    # VST3 plugin → <install>/   (user must copy to VST3 path manually, or use -InstallVst)
    install(DIRECTORY
        "${CMAKE_BINARY_DIR}/SalamanderPiano_artefacts/$<CONFIG>/VST3/Salamander Piano.vst3"
        DESTINATION .
        COMPONENT VST3
        OPTIONAL)
endif()

# ── CPack – Windows NSIS installer ──
if(NOT SALAMANDER_ANDROID)
    set(CPACK_PACKAGE_NAME            "SalamanderPiano")
    set(CPACK_PACKAGE_VENDOR          "Salamander Piano")
    set(CPACK_PACKAGE_VERSION         "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Salamander Piano – Free virtual grand piano (Standalone + VST3)")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY   "SalamanderPiano")
    set(CPACK_RESOURCE_FILE_LICENSE   "${CMAKE_SOURCE_DIR}/LICENSE")

    # Components
    set(CPACK_COMPONENTS_ALL Standalone VST3)
    set(CPACK_COMPONENT_STANDALONE_DISPLAY_NAME "Standalone Application")
    set(CPACK_COMPONENT_STANDALONE_DESCRIPTION
        "Salamander Piano standalone application with SFZ sample data.")
    set(CPACK_COMPONENT_STANDALONE_REQUIRED TRUE)

    set(CPACK_COMPONENT_VST3_DISPLAY_NAME "VST3 Plugin")
    set(CPACK_COMPONENT_VST3_DESCRIPTION
        "VST3 plugin for use in a DAW. Copy Salamander Piano.vst3 to your VST3 folder.")

    # ── NSIS specific ──
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME       "Salamander Piano ${PROJECT_VERSION}")
    set(CPACK_NSIS_PACKAGE_NAME       "Salamander Piano")
    set(CPACK_NSIS_INSTALL_ROOT       "$PROGRAMFILES64")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "Salamander Piano.exe")

    # Start Menu shortcut for the standalone
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Salamander Piano.lnk' '$INSTDIR\\\\Salamander Piano.exe'")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Salamander Piano.lnk'")

    # Also fall back to a ZIP for users without NSIS
    set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_BINARY_DIR}/CPackProjectConfig.cmake")
    file(WRITE "${CMAKE_BINARY_DIR}/CPackProjectConfig.cmake" "")  # placeholder

    include(CPack)
endif()
