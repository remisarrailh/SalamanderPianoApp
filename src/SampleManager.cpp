#include "SampleManager.h"

#if JUCE_ANDROID
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#define ALOG(...) __android_log_print(ANDROID_LOG_ERROR, "SALTEST", __VA_ARGS__)
#else
#define ALOG(...) DBG(juce::String::formatted(__VA_ARGS__))
#endif

SampleManager::SampleManager()
{
}

juce::File SampleManager::locateSfzFile()
{
#if JUCE_ANDROID
    ALOG ("locateSfzFile: searching pre-extracted paths...");
    // On Android, first try to find already-extracted samples
    auto searchPaths = getSearchPaths();

    for (const auto& path : searchPaths)
    {
        ALOG ("locateSfzFile: checking %s", path.toRawUTF8());
        juce::File dir (path);
        auto sfzFile = findSfzInDirectory (dir);

        if (sfzFile.existsAsFile())
        {
            // Also verify Samples/ subfolder is present — otherwise this is a stale partial
            // extraction from the old code that only copied the .sfz file but not the samples.
            auto samplesDir = dir.getChildFile ("Samples");
            if (! samplesDir.isDirectory())
            {
                ALOG ("locateSfzFile: SFZ exists at %s but Samples/ missing - stale extraction, skipping",
                      sfzFile.getFullPathName().toRawUTF8());
                continue;
            }
            ALOG ("locateSfzFile: FOUND (with Samples/) at %s", sfzFile.getFullPathName().toRawUTF8());
            sfzDirectory = dir;
            ready.store (true);
            statusMessage = "Samples loaded from: " + dir.getFullPathName();
            return sfzFile;
        }
    }

    ALOG ("locateSfzFile: not pre-extracted, starting extraction...");
    // If not found, extract from APK assets
    auto extractedDir = extractAndroidAssets();

    if (extractedDir.isDirectory())
    {
        auto sfzFile = findSfzInDirectory (extractedDir);

        if (sfzFile.existsAsFile())
        {
            ALOG ("locateSfzFile: extract+found at %s", sfzFile.getFullPathName().toRawUTF8());
            sfzDirectory = extractedDir;
            ready.store (true);
            statusMessage = "Samples extracted to: " + extractedDir.getFullPathName();
            return sfzFile;
        }
        ALOG ("locateSfzFile: extraction dir exists but SFZ not found inside!");
    }

    ready.store (false);
    statusMessage = "Failed to extract samples from APK assets.";
    ALOG ("locateSfzFile: FAILED - %s", statusMessage.toRawUTF8());
    return {};
#else
    auto searchPaths = getSearchPaths();

    for (const auto& path : searchPaths)
    {
        juce::File dir (path);
        auto sfzFile = findSfzInDirectory (dir);

        if (sfzFile.existsAsFile())
        {
            sfzDirectory = dir;
            ready.store (true);
            statusMessage = "Samples loaded from: " + dir.getFullPathName();
            return sfzFile;
        }
    }

    ready.store (false);
    statusMessage = "Samples not found! Please place the Salamander Grand Piano SFZ files "
                    "in one of the expected locations.";
    return {};
#endif
}

juce::File SampleManager::findSfzInDirectory (const juce::File& dir)
{
    if (! dir.isDirectory())
        return {};

    // Both Android and Windows use the full SFZ (16 velocity layers, all samples)
    auto sfzFile = dir.getChildFile (SFZ_FILENAME);
    if (sfzFile.existsAsFile())
        return sfzFile;

    sfzFile = dir.getChildFile ("SFZ").getChildFile (SFZ_FILENAME);
    if (sfzFile.existsAsFile())
        return sfzFile;

    return {};
}

juce::StringArray SampleManager::getSearchPaths() const
{
    juce::StringArray paths;

#if JUCE_ANDROID
    // Android: internal app data directory
    auto appFiles = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
    paths.add (appFiles.getChildFile ("sfz").getFullPathName());
    paths.add (appFiles.getFullPathName());
#else
    // 1. Next to the executable: <exe_dir>/sfz/
    auto exeDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                      .getParentDirectory();
    paths.add (exeDir.getChildFile ("sfz").getFullPathName());
    paths.add (exeDir.getFullPathName());

    // 2. Working directory
    auto cwd = juce::File::getCurrentWorkingDirectory();
    paths.add (cwd.getChildFile ("sfz").getFullPathName());

    // 3. AppData (Windows) or Application Support (macOS)
    auto appData = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
    paths.add (appData.getChildFile ("SalamanderPiano").getChildFile ("sfz").getFullPathName());

#if JUCE_WINDOWS
    // 4. Program Files
    paths.add ("C:\\Program Files\\SalamanderPiano\\sfz");

    // 5. Common development paths
    auto desktop = juce::File::getSpecialLocation (juce::File::userDesktopDirectory);
    paths.add (desktop.getChildFile ("SalamanderGrandPiano-master").getChildFile ("SFZ").getFullPathName());
    paths.add (desktop.getChildFile ("SalamanderPianoApp").getChildFile ("sfz").getFullPathName());
#endif
#endif

    return paths;
}

#if JUCE_ANDROID

// ── Helper: list all entries (files AND subdirectories) at an asset path ──────
// The NDK AAssetDir_getNextFileName() only returns files at the current level and
// silently skips subdirectory names.  Java's AssetManager.list() returns everything.
juce::StringArray SampleManager::listAssetEntries (JNIEnv* env, jobject javaAssetMgr,
                                                    const juce::String& assetPath)
{
    juce::StringArray result;

    jclass cls = env->GetObjectClass (javaAssetMgr);
    jmethodID listMid = env->GetMethodID (cls, "list", "(Ljava/lang/String;)[Ljava/lang/String;");
    env->DeleteLocalRef (cls);

    if (listMid == nullptr)
    {
        ALOG ("listAssetEntries: AssetManager.list() method not found!");
        return result;
    }

    jstring jPath = env->NewStringUTF (assetPath.toRawUTF8());
    auto arr = (jobjectArray) env->CallObjectMethod (javaAssetMgr, listMid, jPath);
    env->DeleteLocalRef (jPath);

    if (arr != nullptr)
    {
        int len = env->GetArrayLength (arr);

        for (int i = 0; i < len; ++i)
        {
            auto jstr = (jstring) env->GetObjectArrayElement (arr, i);
            const char* cstr = env->GetStringUTFChars (jstr, nullptr);
            result.add (juce::String::fromUTF8 (cstr));
            env->ReleaseStringUTFChars (jstr, cstr);
            env->DeleteLocalRef (jstr);
        }

        env->DeleteLocalRef (arr);
    }

    return result;
}

juce::File SampleManager::extractAndroidAssets()
{
    ALOG ("extractAndroidAssets START");
    auto* env = juce::getEnv();
    auto appFiles = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
    auto destDir = appFiles.getChildFile ("sfz");
    ALOG ("extractAndroidAssets: destDir=%s", destDir.getFullPathName().toRawUTF8());

    // Check if already fully extracted
    if (destDir.getChildFile (SFZ_FILENAME).existsAsFile())
    {
        // Also verify samples subfolder exists to avoid stale partial extractions
        if (destDir.getChildFile ("Samples").isDirectory())
        {
            ALOG ("extractAndroidAssets: already extracted, returning %s",
                  destDir.getFullPathName().toRawUTF8());
            return destDir;
        }
        ALOG ("extractAndroidAssets: SFZ exists but Samples/ missing, re-extracting");
    }

    if (onProgress)
        onProgress (0.0f, "Counting files to extract...");

    // Get Java AssetManager — keep the Java object alive for the whole extraction
    // because we need it for AssetManager.list() calls.
    auto context = juce::getAppContext();

    if (context.get() == nullptr)
    {
        ALOG ("extractAndroidAssets: getAppContext() returned null!");
        statusMessage = "Cannot get Android app context.";
        return {};
    }
    ALOG ("extractAndroidAssets: got app context OK");

    jclass ctxClass = env->GetObjectClass (context.get());
    jmethodID getAssetsMid = env->GetMethodID (ctxClass, "getAssets",
                                               "()Landroid/content/res/AssetManager;");
    env->DeleteLocalRef (ctxClass);

    jobject javaAssetMgrLocal = env->CallObjectMethod (context.get(), getAssetsMid);

    if (javaAssetMgrLocal == nullptr)
    {
        ALOG ("extractAndroidAssets: getAssets() returned null!");
        statusMessage = "Cannot get Android AssetManager.";
        return {};
    }
    ALOG ("extractAndroidAssets: got Java AssetManager OK");

    // Promote to global ref so it survives across JNI calls during extraction
    jobject javaAssetMgr = env->NewGlobalRef (javaAssetMgrLocal);
    env->DeleteLocalRef (javaAssetMgrLocal);

    auto* nativeMgr = AAssetManager_fromJava (env, javaAssetMgr);

    if (nativeMgr == nullptr)
    {
        ALOG ("extractAndroidAssets: AAssetManager_fromJava returned null!");
        statusMessage = "Cannot get native AssetManager.";
        env->DeleteGlobalRef (javaAssetMgr);
        return {};
    }
    ALOG ("extractAndroidAssets: got native AAssetManager OK");

    ALOG ("extractAndroidAssets: counting files via Java list()...");
    int totalFiles = countAssetFiles (env, javaAssetMgr, nativeMgr, "sfz");
    ALOG ("extractAndroidAssets: totalFiles=%d", totalFiles);

    if (totalFiles <= 0)
    {
        ALOG ("extractAndroidAssets: no files found in sfz/ asset dir!");
        statusMessage = "No sample files found in APK assets.";
        env->DeleteGlobalRef (javaAssetMgr);
        return {};
    }

    destDir.createDirectory();

    int filesCopied = 0;
    copyAssetDirectory (env, javaAssetMgr, nativeMgr, "sfz", destDir, filesCopied, totalFiles);

    env->DeleteGlobalRef (javaAssetMgr);

    if (onProgress)
        onProgress (1.0f, "Extraction complete!");

    ALOG ("extractAndroidAssets: done, %d files copied", filesCopied);
    return destDir;
}

int SampleManager::countAssetFiles (JNIEnv* env, jobject javaAssetMgr,
                                     AAssetManager* nativeMgr, const juce::String& assetPath)
{
    int count = 0;

    // Use Java list() — returns both file names AND subdirectory names
    auto entries = listAssetEntries (env, javaAssetMgr, assetPath);

    for (const auto& entry : entries)
    {
        auto childPath = assetPath + "/" + entry;

        // Try opening as a file via the NDK
        auto* asset = AAssetManager_open (nativeMgr, childPath.toRawUTF8(), AASSET_MODE_UNKNOWN);

        if (asset != nullptr)
        {
            AAsset_close (asset);
            ++count;
        }
        else
        {
            // It's a subdirectory — recurse
            count += countAssetFiles (env, javaAssetMgr, nativeMgr, childPath);
        }
    }

    return count;
}

void SampleManager::copyAssetDirectory (JNIEnv* env, jobject javaAssetMgr,
                                         AAssetManager* nativeMgr,
                                         const juce::String& assetPath,
                                         const juce::File& destDir,
                                         int& filesCopied, int totalFiles)
{
    destDir.createDirectory();

    auto entries = listAssetEntries (env, javaAssetMgr, assetPath);

    for (const auto& entry : entries)
    {
        auto childAssetPath = assetPath + "/" + entry;
        auto destChild = destDir.getChildFile (entry);

        auto* asset = AAssetManager_open (nativeMgr, childAssetPath.toRawUTF8(),
                                          AASSET_MODE_STREAMING);

        if (asset != nullptr)
        {
            // It's a file — copy if missing or size differs
            auto length = (juce::int64) AAsset_getLength64 (asset);

            if (! destChild.existsAsFile() || destChild.getSize() != length)
            {
                juce::FileOutputStream output (destChild);

                if (output.openedOk())
                {
                    constexpr int bufSize = 131072; // 128 KB chunks
                    juce::HeapBlock<char> buf (bufSize);
                    int bytesRead;

                    while ((bytesRead = AAsset_read (asset, buf.get(), (size_t) bufSize)) > 0)
                        output.write (buf.get(), (size_t) bytesRead);
                }
                else
                {
                    DBG ("copyAssetDirectory: failed to open output: " + destChild.getFullPathName());
                }
            }

            AAsset_close (asset);
            ++filesCopied;

            if (onProgress && totalFiles > 0)
            {
                float progress = static_cast<float> (filesCopied) / static_cast<float> (totalFiles);
                juce::String msg = "Extracting... " + juce::String (filesCopied)
                                   + "/" + juce::String (totalFiles)
                                   + " (" + entry + ")";
                onProgress (progress, msg);
            }
        }
        else
        {
            // It's a subdirectory — recurse
            ALOG ("copyAssetDirectory: recursing into %s", childAssetPath.toRawUTF8());
            copyAssetDirectory (env, javaAssetMgr, nativeMgr, childAssetPath,
                                destChild, filesCopied, totalFiles);
        }
    }
}

#endif
