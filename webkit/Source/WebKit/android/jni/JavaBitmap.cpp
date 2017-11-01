#include "config.h"
#include "JavaBitmap.h"

#include <android/bitmap.h>
#include "AndroidLog.h"
#include <JNIHelp.h>
#include <JNIUtility.h>

namespace android {

JavaBitmap::JavaBitmap(jobject bitmap)
        : bitmap_(bitmap),
          pixels_(NULL) {

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

JavaBitmap::~JavaBitmap() {
    int err = AndroidBitmap_unlockPixels(JSC::Bindings::getJNIEnv(), bitmap_);
    ASSERT(!err);
}

SkBitmap CreateSkBitmapFromJavaBitmap(const JavaBitmap& jbitmap) {
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
