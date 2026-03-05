# Salamander Piano

A free, open-source virtual grand piano instrument for **Windows** (Standalone + VST3) and **Android**, powered by the [Salamander Grand Piano V3](https://archive.org/details/SalamanderGrandPianoV3) samples.

Built with [JUCE](https://github.com/juce-framework/JUCE) and [sfizz](https://github.com/sfztools/sfizz).

---

## Features

- Full 16-velocity-layer Salamander Grand Piano V3 SFZ soundfont
- Windows: Standalone application + VST3 plugin
- Android: standalone APK (arm64)
- MIDI input support (hardware keyboard or on-screen keyboard)
- Sustain pedal, string resonance and hammer noise controls

---

## Getting started (users)

### Samples

The Salamander Grand Piano V3 samples are **included in this repository via [Git LFS](https://git-lfs.com/)**.

When cloning, make sure Git LFS is installed so the sample files are downloaded automatically:

```bash
# Install Git LFS once (if not already done)
git lfs install

# Clone with samples
git clone --recurse-submodules https://github.com/<you>/SalamanderPianoApp.git
```

If you cloned without LFS (sample files are 0-byte placeholders), run:

```bash
git lfs pull
```

Samples are licensed under [Creative Commons Attribution 3.0](https://creativecommons.org/licenses/by/3.0/) by Alexander Holm.

### Windows

1. Run the installer `SalamanderPiano-<version>-win64.exe` (see [Releases](../../releases)).
2. The installer puts the app in `C:\Program Files\SalamanderPiano\`.
3. Copy or point the app to the SFZ samples folder (see *Sample search paths* below).

**Sample search paths (Windows)**

The app looks for the SFZ file in these locations (in order):
1. `<exe location>\sfz\`
2. `<exe location>\`
3. Current working directory `\sfz\`
4. `%APPDATA%\SalamanderPiano\sfz\`
5. `C:\Program Files\SalamanderPiano\sfz\`

Placing the samples in `C:\Program Files\SalamanderPiano\sfz\` is recommended.

### Android

Install the APK from the [Releases](../../releases) page:

```
adb install SalamanderPiano-<version>.apk
```

The first launch will extract the sample files from the APK into app storage automatically.

---

## Building from source

### Prerequisites (all platforms)

- Git with submodule support
- CMake 3.22+

```bash
git clone --recurse-submodules https://github.com/<you>/SalamanderPianoApp.git
```

---

### Windows (Standalone + VST3)

**Requirements:** Visual Studio 2022 with C++ workload, CMake 3.22+  
**For the installer:** [NSIS 3.x](https://nsis.sourceforge.io/)

```powershell
# Debug build
.\build_windows.ps1

# Release build
.\build_windows.ps1 -Release

# Release build + NSIS installer
.\build_windows.ps1 -Release -Package

# Install VST3 directly to C:\Program Files\Common Files\VST3 (UAC prompt)
.\build_windows.ps1 -Release -InstallVst
```

Output artefacts after build:
```
build\SalamanderPiano_artefacts\Release\Standalone\Salamander Piano.exe
build\SalamanderPiano_artefacts\Release\VST3\Salamander Piano.vst3\
```

Installer (with `-Package`):
```
build\SalamanderPiano-<version>-win64.exe
```

---

### Android (APK)

**Requirements:**
- Android Studio (or standalone SDK + NDK)
- NDK 28.x (`ndkVersion = "28.1.13356709"`)
- JDK 17+
- A Windows build done at least once (generates `juceaide.exe` used by the Android build)

```powershell
# First: build Windows to generate juceaide
.\build_windows.ps1

# Then build Android Debug APK
.\build_android.ps1

# Release APK
.\build_android.ps1 -Release

# Clean + Release
.\build_android.ps1 -Clean -Release
```

Output:
```
android\app\build\outputs\apk\debug_\debug\app-debug_-debug.apk
android\app\build\outputs\apk\release_\release\app-release_-release.apk
```

---

## Project structure

```
SalamanderPianoApp/
├── src/                    # C++ source (JUCE plugin)
│   ├── PluginProcessor.*   # Audio engine
│   ├── PluginEditor.*      # Main UI
│   ├── SfizzEngine.*       # sfizz integration
│   ├── SampleManager.*     # SFZ sample discovery / extraction
│   ├── MidiDeviceManager.* # MIDI device handling
│   ├── LookAndFeel.*       # Custom UI theme
│   └── ui/                 # UI panels
├── sfz/                    # SFZ instrument definition + sample stubs
│   └── Samples/            # (not tracked – download separately)
├── android/                # Android Gradle project
├── extern/
│   ├── JUCE/               # JUCE submodule
│   └── sfizz/              # sfizz submodule
├── CMakeLists.txt
├── build_windows.ps1       # Windows build script
└── build_android.ps1       # Android build script
```

---

## License

This project is licensed under the **MIT License** – see [LICENSE](LICENSE).  
Copyright © µsini

The **Salamander Grand Piano V3** samples are by Alexander Holm and released under  
[Creative Commons Attribution 3.0](https://creativecommons.org/licenses/by/3.0/).

JUCE is licensed under the [AGPLv3 / commercial license](https://juce.com/juce-open-source-licence).  
sfizz is licensed under the [BSD 2-Clause License](https://github.com/sfztools/sfizz/blob/develop/LICENSE.md).
