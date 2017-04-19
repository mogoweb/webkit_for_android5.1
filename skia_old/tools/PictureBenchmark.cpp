/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchLogger.h"
#include "BenchTimer.h"
#include "PictureBenchmark.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkString.h"
#include "picture_utils.h"
#include "TimerData.h"

namespace sk_tools {

PictureBenchmark::PictureBenchmark()
: fRepeats(1)
, fLogger(NULL)
, fRenderer(NULL)
, fLogPerIter(false)
, fPrintMin(false)
, fShowWallTime(false)
, fShowTruncatedWallTime(false)
, fShowCpuTime(true)
, fShowTruncatedCpuTime(false)
, fShowGpuTime(false)
, fTimeIndividualTiles(false)
{}

PictureBenchmark::~PictureBenchmark() {
    SkSafeUnref(fRenderer);
}

BenchTimer* PictureBenchmark::setupTimer() {
#if SK_SUPPORT_GPU
    if (fRenderer != NULL && fRenderer->isUsingGpuDevice()) {
        return SkNEW_ARGS(BenchTimer, (fRenderer->getGLContext()));
    }
#endif
    return SkNEW_ARGS(BenchTimer, (NULL));
}

void PictureBenchmark::logProgress(const char msg[]) {
    if (fLogger != NULL) {
        fLogger->logProgress(msg);
    }
}

PictureRenderer* PictureBenchmark::setRenderer(sk_tools::PictureRenderer* renderer) {
    SkRefCnt_SafeAssign(fRenderer, renderer);
    return renderer;
}

void PictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (NULL == pict) {
        return;
    }

    SkASSERT(fRenderer != NULL);
    if (NULL == fRenderer) {
        return;
    }

    fRenderer->init(pict);

    // We throw this away to remove first time effects (such as paging in this program)
    fRenderer->setup();
    fRenderer->render(NULL);
    fRenderer->resetState(true);

    bool usingGpu = false;
#if SK_SUPPORT_GPU
    usingGpu = fRenderer->isUsingGpuDevice();
#endif

    if (fTimeIndividualTiles) {
        TiledPictureRenderer* tiledRenderer = fRenderer->getTiledRenderer();
        SkASSERT(tiledRenderer);
        if (NULL == tiledRenderer) {
            return;
        }
        int xTiles, yTiles;
        if (!tiledRenderer->tileDimensions(xTiles, yTiles)) {
            return;
        }

        // Insert a newline so that each tile is reported on its own line (separate from the line
        // that describes the skp being run).
        this->logProgress("\n");

        int x, y;
        while (tiledRenderer->nextTile(x, y)) {
            // There are two timers, which will behave slightly differently:
            // 1) longRunningTimer, along with perTileTimerData, will time how long it takes to draw
            // one tile fRepeats times, and take the average. As such, it will not respect the
            // logPerIter or printMin options, since it does not know the time per iteration. It
            // will also be unable to call flush() for each tile.
            // The goal of this timer is to make up for a system timer that is not precise enough to
            // measure the small amount of time it takes to draw one tile once.
            //
            // 2) perTileTimer, along with perTileTimerData, will record each run separately, and
            // then take the average. As such, it supports logPerIter and printMin options.
            SkAutoTDelete<BenchTimer> longRunningTimer(this->setupTimer());
            TimerData longRunningTimerData(tiledRenderer->getPerIterTimeFormat(),
                                           tiledRenderer->getNormalTimeFormat());
            SkAutoTDelete<BenchTimer> perTileTimer(this->setupTimer());
            TimerData perTileTimerData(tiledRenderer->getPerIterTimeFormat(),
                                       tiledRenderer->getNormalTimeFormat());
            longRunningTimer->start();
            for (int i = 0; i < fRepeats; ++i) {
                perTileTimer->start();
                tiledRenderer->drawCurrentTile();
                perTileTimer->truncatedEnd();
                tiledRenderer->resetState(false);
                perTileTimer->end();
                perTileTimerData.appendTimes(perTileTimer.get(), fRepeats - 1 == i);
            }
            longRunningTimer->truncatedEnd();
            tiledRenderer->resetState(true);
            longRunningTimer->end();
            longRunningTimerData.appendTimes(longRunningTimer.get(), true);

            SkString configName = tiledRenderer->getConfigName();
            configName.appendf(": tile [%i,%i] out of [%i,%i]", x, y, xTiles, yTiles);
            SkString result = perTileTimerData.getResult(fLogPerIter, fPrintMin, fRepeats,
                                                         configName.c_str(), fShowWallTime,
                                                         fShowTruncatedWallTime, fShowCpuTime,
                                                         fShowTruncatedCpuTime,
                                                         usingGpu && fShowGpuTime);
            result.append("\n");
            this->logProgress(result.c_str());

            configName.append(" <averaged>");
            SkString longRunningResult = longRunningTimerData.getResult(false, false, fRepeats,
                    configName.c_str(), fShowWallTime, fShowTruncatedWallTime,
                    fShowCpuTime, fShowTruncatedCpuTime, usingGpu && fShowGpuTime);
            longRunningResult.append("\n");
            this->logProgress(longRunningResult.c_str());
        }
    } else {
        SkAutoTDelete<BenchTimer> timer(this->setupTimer());
        TimerData timerData(fRenderer->getPerIterTimeFormat(), fRenderer->getNormalTimeFormat());
        for (int i = 0; i < fRepeats; ++i) {
            fRenderer->setup();

            timer->start();
            fRenderer->render(NULL);
            timer->truncatedEnd();

            // Finishes gl context
            fRenderer->resetState(true);
            timer->end();

            timerData.appendTimes(timer.get(), fRepeats - 1 == i);
        }

        SkString configName = fRenderer->getConfigName();
        SkString result = timerData.getResult(fLogPerIter, fPrintMin, fRepeats,
                                              configName.c_str(), fShowWallTime,
                                              fShowTruncatedWallTime, fShowCpuTime,
                                              fShowTruncatedCpuTime, usingGpu && fShowGpuTime);
        result.append("\n");
        this->logProgress(result.c_str());
    }

    fRenderer->end();
}

}
