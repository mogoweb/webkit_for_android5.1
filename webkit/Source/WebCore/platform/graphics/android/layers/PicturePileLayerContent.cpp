#define LOG_TAG "PicturePileLayerContent"
#define LOG_NDEBUG 1

#include "config.h"
#include "PicturePileLayerContent.h"

#include "AndroidLog.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#if !ENABLE(OLD_SKIA)
#include "SkPictureRecorder.h"
#endif

namespace WebCore {

PicturePileLayerContent::PicturePileLayerContent(const PicturePile& picturePile)
    : m_picturePile(picturePile)
    , m_maxZoomScale(picturePile.maxZoomScale())
    , m_hasContent(!picturePile.isEmpty())
{
}

void PicturePileLayerContent::draw(SkCanvas* canvas)
{
    TRACE_METHOD();
    android::Mutex::Autolock lock(m_drawLock);
    m_picturePile.draw(canvas);

    if (CC_UNLIKELY(!m_hasContent))
        ALOGW("Warning: painting PicturePile without content!");
}

void PicturePileLayerContent::serialize(SkWStream* stream)
{
    if (!stream)
       return;
#if ENABLE(OLD_SKIA)
    SkPicture picture;
    draw(picture.beginRecording(width(), height(),
                                SkPicture::kUsePathBoundsForClip_RecordingFlag));
    picture.endRecording();
    picture.serialize(stream);
#else
    SkPictureRecorder recorder;
    draw(recorder.beginRecording(width(), height()));
    SkPicture* picture = recorder.endRecording();
    picture->serialize(stream);
#endif
}

PrerenderedInval* PicturePileLayerContent::prerenderForRect(const IntRect& dirty)
{
    return m_picturePile.prerenderedInvalForArea(dirty);
}

void PicturePileLayerContent::clearPrerenders()
{
    m_picturePile.clearPrerenders();
}

} // namespace WebCore
