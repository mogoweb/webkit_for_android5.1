#include "config.h"
#include "JavaBitmap.h"

#include <android/bitmap.h>
#include "AndroidLog.h"
#include <JNIHelp.h>
#include <JNIUtility.h>

namespace android {

JavaBitmap::JavaBitmap(jobject bitmap)
        : bitmap_(bitmap),
          pixels_(NULL)
{
    JNIEnv* env = JSC::Bindings::getJNIEnv();
    int err = AndroidBitmap_lockPixels(env, bitmap_, &pixels_);
    ASSERT(!err);
    ASSERT(pixels_);

    AndroidBitmapInfo info;
    err = AndroidBitmap_getInfo(env, bitmap_, &info);
    ASSERT(!err);
    size_ = Size(info.width, info.height);
    format_ = info.format;
    stride_ = info.stride;
}

JavaBitmap::~JavaBitmap()
{
    int err = AndroidBitmap_unlockPixels(JSC::Bindings::getJNIEnv(), bitmap_);
    ASSERT(!err);
}

jobject CreateJavaBitmap(int width, int height, SkBitmap::Config config)
{
    ASSERT(width > 0);
    ASSERT(height > 0);
    int java_bitmap_config = static_cast<int>(config);

    // get the application context using JNI
    JNIEnv* env = JSC::Bindings::getJNIEnv();
    jclass helperClass = env->FindClass("android/webkit/BitmapHelper");
    jmethodID createBitmapMethod = env->GetStaticMethodID(helperClass, "createBitmap", "(III)Landroid/graphics/Bitmap;");
    jobject result = env->CallStaticObjectMethod(helperClass, createBitmapMethod, width, height, java_bitmap_config);
    env->DeleteLocalRef(helperClass);
    return result;
}

jobject ConvertToJavaBitmap(const SkBitmap* skbitmap)
{
    ASSERT(skbitmap);
    ASSERT(!skbitmap->isNull());
    SkBitmap::Config config = skbitmap->getConfig();
    jobject jbitmap = CreateJavaBitmap(skbitmap->width(), skbitmap->height(), config);
    SkAutoLockPixels src_lock(*skbitmap);
    JavaBitmap dst_lock(jbitmap);
    void* src_pixels = skbitmap->getPixels();
    void* dst_pixels = dst_lock.pixels();
    memcpy(dst_pixels, src_pixels, skbitmap->getSize());

    return jbitmap;
}

SkBitmap CreateSkBitmapFromJavaBitmap(const JavaBitmap& jbitmap)
{
    ASSERT(!jbitmap.size().IsEmpty());
    ASSERT(jbitmap.stride() > 0U);
    ASSERT(jbitmap.pixels());

    Size src_size = jbitmap.size();

    SkBitmap skbitmap;
    switch (jbitmap.format()) {
        case ANDROID_BITMAP_FORMAT_RGBA_8888:
            skbitmap.setConfig(SkBitmap::kARGB_8888_Config, src_size.width(), src_size.height());
            break;
        case ANDROID_BITMAP_FORMAT_RGB_565:
            skbitmap.setConfig(SkBitmap::kRGB_565_Config, src_size.width(), src_size.height());
            break;
        case ANDROID_BITMAP_FORMAT_A_8:
            skbitmap.setConfig(SkBitmap::kA8_Config, src_size.width(), src_size.height());
            break;
        default:
        ALOGE("Invalid Java bitmap format: %d", jbitmap.format());
        break;
    }
    skbitmap.allocPixels();
    const void* src_pixels = jbitmap.pixels();
    void* dst_pixels = skbitmap.getPixels();
    memcpy(dst_pixels, src_pixels, skbitmap.getSize());

    return skbitmap;
}

}
