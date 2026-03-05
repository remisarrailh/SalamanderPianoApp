cmake_minimum_required(VERSION 3.22)
project(SalamanderPianoAndroid LANGUAGES C CXX)

# When cross-compiling for Android on Windows, JUCE's juceaide bootstrap fails
# because MSVC is not available. We skip it by pointing to a pre-built juceaide.exe
# that was already compiled during the normal Windows build (cmake -B build).
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    get_filename_component(_project_root "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
    set(JUCE_JUCEAIDE_EXECUTABLE
        "${_project_root}/build/extern/JUCE/tools/extras/Build/juceaide/juceaide_artefacts/Custom/juceaide.exe"
        CACHE FILEPATH "Path to pre-built juceaide (from Windows build)")

    if(NOT EXISTS "${JUCE_JUCEAIDE_EXECUTABLE}")
        message(FATAL_ERROR
            "Pre-built juceaide not found at: ${JUCE_JUCEAIDE_EXECUTABLE}\n"
            "Please build the Windows version first: cmake -B build && cmake --build build --target juceaide")
    endif()
endif()

# Include root project which has JUCE, sfizz, and our app
add_subdirectory(../.. ./salamander-build)
