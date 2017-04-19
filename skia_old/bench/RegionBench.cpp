
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkString.h"

static bool union_proc(SkRegion& a, SkRegion& b) {
    SkRegion result;
    return result.op(a, b, SkRegion::kUnion_Op);
}

static bool sect_proc(SkRegion& a, SkRegion& b) {
    SkRegion result;
    return result.op(a, b, SkRegion::kIntersect_Op);
}

static bool diff_proc(SkRegion& a, SkRegion& b) {
    SkRegion result;
    return result.op(a, b, SkRegion::kDifference_Op);
}

static bool diffrect_proc(SkRegion& a, SkRegion& b) {
    SkRegion result;
    return result.op(a, b.getBounds(), SkRegion::kDifference_Op);
}

static bool diffrectbig_proc(SkRegion& a, SkRegion& b) {
    SkRegion result;
    return result.op(a, a.getBounds(), SkRegion::kDifference_Op);
}

static bool containsrect_proc(SkRegion& a, SkRegion& b) {
    SkIRect r = a.getBounds();
    r.inset(r.width()/4, r.height()/4);
    (void)a.contains(r);

    r = b.getBounds();
    r.inset(r.width()/4, r.height()/4);
    return b.contains(r);
}

static bool sectsrgn_proc(SkRegion& a, SkRegion& b) {
    return a.intersects(b);
}

static bool sectsrect_proc(SkRegion& a, SkRegion& b) {
    SkIRect r = a.getBounds();
    r.inset(r.width()/4, r.height()/4);
    return a.intersects(r);
}

static bool containsxy_proc(SkRegion& a, SkRegion& b) {
    const SkIRect& r = a.getBounds();
    const int dx = r.width() / 8;
    const int dy = r.height() / 8;
    for (int y = r.fTop; y < r.fBottom; y += dy) {
        for (int x = r.fLeft; x < r.fRight; x += dx) {
            (void)a.contains(x, y);
        }
    }
    return true;
}

class RegionBench : public SkBenchmark {
public:
    typedef bool (*Proc)(SkRegion& a, SkRegion& b);

    SkRegion fA, fB;
    Proc     fProc;
    SkString fName;
    int      fLoopMul;

    enum {
        W = 1024,
        H = 768,
        N = SkBENCHLOOP(2000)
    };

    SkIRect randrect(SkRandom& rand) {
        int x = rand.nextU() % W;
        int y = rand.nextU() % H;
        int w = rand.nextU() % W;
        int h = rand.nextU() % H;
        return SkIRect::MakeXYWH(x, y, w >> 1, h >> 1);
    }

    RegionBench(void* param, int count, Proc proc, const char name[], int mul = 1) : INHERITED(param) {
        fProc = proc;
        fName.printf("region_%s_%d", name, count);
        fLoopMul = mul;

        SkRandom rand;
        for (int i = 0; i < count; i++) {
            fA.op(randrect(rand), SkRegion::kXOR_Op);
            fB.op(randrect(rand), SkRegion::kXOR_Op);
        }
        fIsRendering = false;
    }

protected:
    virtual const char* onGetName() { return fName.c_str(); }

    virtual void onDraw(SkCanvas* canvas) {
        Proc proc = fProc;
        int n = fLoopMul * N;
        for (int i = 0; i < n; ++i) {
            proc(fA, fB);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

#define SMALL   16

static SkBenchmark* gF0(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, union_proc, "union")); }
static SkBenchmark* gF1(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, sect_proc, "intersect")); }
static SkBenchmark* gF2(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, diff_proc, "difference")); }
static SkBenchmark* gF3(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, diffrect_proc, "differencerect")); }
static SkBenchmark* gF4(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, diffrectbig_proc, "differencerectbig")); }
static SkBenchmark* gF5(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, containsrect_proc, "containsrect", 100)); }
static SkBenchmark* gF6(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, sectsrgn_proc, "intersectsrgn", 10)); }
static SkBenchmark* gF7(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, sectsrect_proc, "intersectsrect", 200)); }
static SkBenchmark* gF8(void* p) { return SkNEW_ARGS(RegionBench, (p, SMALL, containsxy_proc, "containsxy")); }

static BenchRegistry gR0(gF0);
static BenchRegistry gR1(gF1);
static BenchRegistry gR2(gF2);
static BenchRegistry gR3(gF3);
static BenchRegistry gR4(gF4);
static BenchRegistry gR5(gF5);
static BenchRegistry gR6(gF6);
static BenchRegistry gR7(gF7);
static BenchRegistry gR8(gF8);
