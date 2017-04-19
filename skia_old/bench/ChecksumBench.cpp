/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkChecksum.h"
#include "SkCityHash.h"
#include "SkMD5.h"
#include "SkRandom.h"
#include "SkSHA1.h"

template<typename T> inline void sk_ignore_unused(const T&) { }

enum ChecksumType {
    kChecksum_ChecksumType,
    kMD5_ChecksumType,
    kSHA1_ChecksumType,
    kCityHash32,
    kCityHash64
};

class ComputeChecksumBench : public SkBenchmark {
    enum {
        U32COUNT  = 256,
        SIZE      = U32COUNT * 4,
        N         = SkBENCHLOOP(100000),
    };
    uint32_t    fData[U32COUNT];
    ChecksumType fType;

public:
    ComputeChecksumBench(void* param, ChecksumType type) : INHERITED(param), fType(type) {
        SkRandom rand;
        for (int i = 0; i < U32COUNT; ++i) {
            fData[i] = rand.nextU();
        }
        fIsRendering = false;
    }

protected:
    virtual const char* onGetName() {
        switch (fType) {
            case kChecksum_ChecksumType: return "compute_checksum";
            case kMD5_ChecksumType: return "compute_md5";
            case kSHA1_ChecksumType: return "compute_sha1";
            case kCityHash32: return "compute_cityhash32";
            case kCityHash64: return "compute_cityhash64";
            default: SK_CRASH(); return "";
        }
    }

    virtual void onDraw(SkCanvas* canvas) {
        switch (fType) {
            case kChecksum_ChecksumType: {
                for (int i = 0; i < N; i++) {
                    volatile uint32_t result = SkChecksum::Compute(fData, sizeof(fData));
                    sk_ignore_unused(result);
                }
            } break;
            case kMD5_ChecksumType: {
                for (int i = 0; i < N; i++) {
                    SkMD5 md5;
                    md5.update(reinterpret_cast<uint8_t*>(fData), sizeof(fData));
                    SkMD5::Digest digest;
                    md5.finish(digest);
                    sk_ignore_unused(digest);
                }
            } break;
            case kSHA1_ChecksumType: {
                for (int i = 0; i < N; i++) {
                    SkSHA1 sha1;
                    sha1.update(reinterpret_cast<uint8_t*>(fData), sizeof(fData));
                    SkSHA1::Digest digest;
                    sha1.finish(digest);
                    sk_ignore_unused(digest);
                }
            } break;
            case kCityHash32: {
                for (int i = 0; i < N; i++) {
                    volatile uint32_t result = SkCityHash::Compute32(reinterpret_cast<char*>(fData), sizeof(fData));
                    sk_ignore_unused(result);
                }
            } break;
            case kCityHash64: {
                for (int i = 0; i < N; i++) {
                    volatile uint64_t result = SkCityHash::Compute64(reinterpret_cast<char*>(fData), sizeof(fData));
                    sk_ignore_unused(result);
                }
            } break;
        }

    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact0(void* p) { return new ComputeChecksumBench(p, kChecksum_ChecksumType); }
static SkBenchmark* Fact1(void* p) { return new ComputeChecksumBench(p, kMD5_ChecksumType); }
static SkBenchmark* Fact2(void* p) { return new ComputeChecksumBench(p, kSHA1_ChecksumType); }
static SkBenchmark* Fact3(void* p) { return new ComputeChecksumBench(p, kCityHash32); }
static SkBenchmark* Fact4(void* p) { return new ComputeChecksumBench(p, kCityHash64); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);
static BenchRegistry gReg3(Fact3);
static BenchRegistry gReg4(Fact4);
