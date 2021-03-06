// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#pragma once

#include <map>

#include "gfx_es2/gpu_features.h"
#include "gfx/gl_common.h"
#include "Globals.h"
#include "GPU/GPUInterface.h"
#include "GPU/GPUState.h"
#include "GPU/GLES/TextureScalerGLES.h"
#include "GPU/Common/TextureCacheCommon.h"

struct VirtualFramebuffer;
class FramebufferManagerGLES;
class DepalShaderCacheGLES;
class ShaderManagerGLES;
class DrawEngineGLES;

inline bool UseBGRA8888() {
	// TODO: Other platforms?  May depend on vendor which is faster?
#ifdef _WIN32
	return gl_extensions.EXT_bgra;
#endif
	return false;
}

class TextureCacheGLES : public TextureCacheCommon {
public:
	TextureCacheGLES(Draw::DrawContext *draw);
	~TextureCacheGLES();

	void SetTexture(bool force = false);
	virtual bool SetOffsetTexture(u32 offset) override;

	void Clear(bool delete_them);
	void StartFrame();
	void Invalidate(u32 addr, int size, GPUInvalidationType type) override;
	void InvalidateAll(GPUInvalidationType type) override;
	void ClearNextFrame();

	void SetFramebufferManager(FramebufferManagerGLES *fbManager) {
		framebufferManager_ = fbManager;
	}
	void SetDepalShaderCache(DepalShaderCacheGLES *dpCache) {
		depalShaderCache_ = dpCache;
	}
	void SetShaderManager(ShaderManagerGLES *sm) {
		shaderManager_ = sm;
	}
	void SetDrawEngine(DrawEngineGLES *td) {
		drawEngine_ = td;
	}

	size_t NumLoadedTextures() const {
		return cache.size();
	}

	void ForgetLastTexture() override {
		lastBoundTexture = -1;
		gstate_c.Dirty(DIRTY_TEXTURE_PARAMS);
	}

	u32 AllocTextureName();

	// Only used by Qt UI?
	bool DecodeTexture(u8 *output, const GPUgstate &state);

	void SetFramebufferSamplingParams(u16 bufferWidth, u16 bufferHeight);

	void ApplyTexture();

protected:
	void DownloadFramebufferForClut(u32 clutAddr, u32 bytes) override;

private:
	void Decimate();  // Run this once per frame to get rid of old textures.
	void DeleteTexture(TexCache::iterator it);
	void UpdateSamplingParams(TexCacheEntry &entry, bool force);
	void LoadTextureLevel(TexCacheEntry &entry, ReplacedTexture &replaced, int level, bool replaceImages, int scaleFactor, GLenum dstFmt);
	GLenum GetDestFormat(GETextureFormat format, GEPaletteFormat clutFormat) const;
	void *DecodeTextureLevelOld(GETextureFormat format, GEPaletteFormat clutformat, int level, GLenum dstFmt, int scaleFactor, int *bufw = 0);
	TexCacheEntry::Status CheckAlpha(const u32 *pixelData, GLenum dstFmt, int stride, int w, int h);
	u32 GetCurrentClutHash();
	void UpdateCurrentClut(GEPaletteFormat clutFormat, u32 clutBase, bool clutIndexIsSimple);
	bool AttachFramebuffer(TexCacheEntry *entry, u32 address, VirtualFramebuffer *framebuffer, u32 texaddrOffset = 0) override;
	void SetTextureFramebuffer(TexCacheEntry *entry, VirtualFramebuffer *framebuffer);
	void ApplyTextureFramebuffer(TexCacheEntry *entry, VirtualFramebuffer *framebuffer);

	bool CheckFullHash(TexCacheEntry *const entry, bool &doDelete);
	bool HandleTextureChange(TexCacheEntry *const entry, const char *reason, bool initialMatch, bool doDelete);
	void BuildTexture(TexCacheEntry *const entry, bool replaceImages);

	std::vector<u32> nameCache_;
	TexCache secondCache;
	u32 secondCacheSizeEstimate_;

	bool clearCacheNextFrame_;
	bool lowMemoryMode_;

	TextureScalerGLES scaler;

	u32 clutHash_;

	u32 lastBoundTexture;
	float maxAnisotropyLevel;

	int decimationCounter_;
	int texelsScaledThisFrame_;
	int timesInvalidatedAllThisFrame_;

	FramebufferManagerGLES *framebufferManager_;
	DepalShaderCacheGLES *depalShaderCache_;
	ShaderManagerGLES *shaderManager_;
	DrawEngineGLES *drawEngine_;

	const char *nextChangeReason_;
	bool nextNeedsRehash_;
	bool nextNeedsChange_;
	bool nextNeedsRebuild_;
};

GLenum getClutDestFormat(GEPaletteFormat format);
