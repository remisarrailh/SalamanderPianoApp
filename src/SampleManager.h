#pragma once

#include <juce_core/juce_core.h>
#include <functional>
#include <atomic>

#if JUCE_ANDROID
#include <android/asset_manager.h>
#endif

class SampleManager
{
public:
    SampleManager();

    // Returns the path to the root SFZ file, or invalid File if not found
    juce::File locateSfzFile();

    // Check if samples are ready to use
    bool areSamplesReady() const { return ready.load(); }

    // Status message for UI
    juce::String getStatusMessage() const { return statusMessage; }

    // Get the SFZ root directory (for display in troubleshoot panel)
    juce::File getSfzDirectory() const { return sfzDirectory; }

    // Progress callback for asset extraction (Android)
    std::function<void (float progress, const juce::String& message)> onProgress;

private:
    juce::File findSfzInDirectory (const juce::File& dir);
    juce::StringArray getSearchPaths() const;

#if JUCE_ANDROID
    juce::File extractAndroidAssets();
    // AAssetDir_getNextFileName() does NOT list subdirectory names; we use Java
    // AssetManager.list() (via JNI) which returns both files and subdirs.
    static juce::StringArray listAssetEntries (JNIEnv* env, jobject javaAssetMgr,
                                               const juce::String& assetPath);
    int  countAssetFiles     (JNIEnv* env, jobject javaAssetMgr, AAssetManager* nativeMgr,
                               const juce::String& assetPath);
    void copyAssetDirectory  (JNIEnv* env, jobject javaAssetMgr, AAssetManager* nativeMgr,
                               const juce::String& assetPath,
                               const juce::File& destDir, int& filesCopied, int totalFiles);
#endif

    juce::File sfzDirectory;
    juce::String statusMessage { "Searching for samples..." };
    std::atomic<bool> ready { false };

    static constexpr const char* SFZ_FILENAME = "Salamander Grand Piano V3.sfz";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleManager)
};
