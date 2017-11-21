
#ifndef JavaBitmap_h
#define JavaBitmap_h

#include <jni.h>

#include "SkBitmap.h"

namespace android {

enum BitmapFormat {
    BITMAP_FORMAT_NO_CONFIG,
    BITMAP_FORMAT_ALPHA_8,
    BITMAP_FORMAT_ARGB_4444,
    BITMAP_FORMAT_ARGB_8888,
    BITMAP_FORMAT_RGB_565,
};

// A size has width and height values.
class Size {
public:
    Size() : width_(0), height_(0) {}
    Size(int width, int height)
            : width_(width < 0 ? 0 : width), height_(height < 0 ? 0 : height) {}
    ~Size() {}
    int width() const { return width_; }
    int height() const { return height_; }
    void SetSize(int width, int height) {
        width_ = width;
        height_ = height;
    }
private:
    int width_;
    int height_;
};

// This class wraps a JNI AndroidBitmap object to make it easier to use. It
// handles locking and unlocking of the underlying pixels, along with wrapping
// various JNI methods.
class JavaBitmap {
public:
    explicit JavaBitmap(jobject bitmap);
    ~JavaBitmap();

    inline void* pixels() { return pixels_; }
    inline const void* pixels() const { return pixels_; }
    inline const Size& size() const { return size_; }
    // Formats are in android/bitmap.h; e.g. ANDROID_BITMAP_FORMAT_RGBA_8888
    inline int format() const { return format_; }
    inline uint32_t stride() const { return stride_; }

    // Registers methods with JNI and returns true if succeeded.
    static bool RegisterJavaBitmap(JNIEnv* env);

 private:
    jobject bitmap_;
    void* pixels_;
    Size size_;
    int format_;
    uint32_t stride_;

private:
    JavaBitmap() {}
};

// Allocates a Java-backed bitmap (android.graphics.Bitmap) with the given
// (non-empty!) size and color type.
jobject CreateJavaBitmap(int width, int height, SkBitmap::Config config);

// Converts |skbitmap| to a Java-backed bitmap (android.graphics.Bitmap).
// Note: |skbitmap| is assumed to be non-null, non-empty and one of RGBA_8888 or
// RGB_565 formats.
jobject ConvertToJavaBitmap(const SkBitmap* skbitmap);

// Converts |bitmap| to an SkBitmap of the same size and format.
// Note: |jbitmap| is assumed to be non-null, non-empty and of format RGBA_8888.
SkBitmap CreateSkBitmapFromJavaBitmap(const JavaBitmap& jbitmap);

}  // namespace android

#endif  // JavaBitmap_h_
