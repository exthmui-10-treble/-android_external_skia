/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageCacherator_DEFINED
#define SkImageCacherator_DEFINED

#include "SkImageGenerator.h"
#include "SkMutex.h"
#include "SkTemplates.h"

class GrContext;
class SkBitmap;

/*
 *  Internal class to manage caching the output of an ImageGenerator.
 */
class SkImageCacherator {
public:
    // Takes ownership of the generator
    static SkImageCacherator* NewFromGenerator(SkImageGenerator*, const SkIRect* subset = nullptr);

    const SkImageInfo& info() const { return fInfo; }
    uint32_t uniqueID() const { return fUniqueID; }

    /**
     *  On success (true), bitmap will point to the pixels for this generator. If this returns
     *  false, the bitmap will be reset to empty.
     */
    bool lockAsBitmap(SkBitmap*);

    /**
     *  Returns a ref() on the texture produced by this generator. The caller must call unref()
     *  when it is done. Will return NULL on failure.
     *
     *  The caller is responsible for calling texture->unref() when they are done.
     */
    GrTexture* lockAsTexture(GrContext*, SkImageUsageType);

    /**
     *  If the underlying src naturally is represented by an encoded blob (in SkData), this returns
     *  a ref to that data. If not, it returns null.
     */
    SkData* refEncoded();

private:
    SkImageCacherator(SkImageGenerator*, const SkImageInfo&, const SkIPoint&, uint32_t uniqueID);

    bool generateBitmap(SkBitmap*);
    bool tryLockAsBitmap(SkBitmap*);

    class ScopedGenerator {
        SkImageCacherator* fCacher;
    public:
        ScopedGenerator(SkImageCacherator* cacher) : fCacher(cacher) {
            fCacher->fMutexForGenerator.acquire();
        }
        ~ScopedGenerator() {
            fCacher->fMutexForGenerator.release();
        }
        SkImageGenerator* operator->() const { return fCacher->fNotThreadSafeGenerator; }
        operator SkImageGenerator*() const { return fCacher->fNotThreadSafeGenerator; }
    };

    SkMutex                         fMutexForGenerator;
    SkAutoTDelete<SkImageGenerator> fNotThreadSafeGenerator;

    const SkImageInfo   fInfo;
    const SkIPoint      fOrigin;
    const uint32_t      fUniqueID;
};

#endif
