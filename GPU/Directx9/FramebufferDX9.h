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

#include <list>
#include <set>
#include <map>

#include "d3d9.h"

// Keeps track of allocated FBOs.
// Also provides facilities for drawing and later converting raw
// pixel data.


#include "Globals.h"
#include "GPU/GPUCommon.h"
#include "GPU/Common/FramebufferCommon.h"
#include "Core/Config.h"
#include "ext/native/thin3d/thin3d.h"

namespace DX9 {

class TextureCacheDX9;
class DrawEngineDX9;
class ShaderManagerDX9;

class FramebufferManagerDX9 : public FramebufferManagerCommon {
public:
	FramebufferManagerDX9(Draw::DrawContext *draw);
	~FramebufferManagerDX9();

	void SetTextureCache(TextureCacheDX9 *tc);
	void SetShaderManager(ShaderManagerDX9 *sm) {
		shaderManager_ = sm;
	}
	void SetDrawEngine(DrawEngineDX9 *td) {
		drawEngine_ = td;
	}

	virtual void DrawPixels(VirtualFramebuffer *vfb, int dstX, int dstY, const u8 *srcPixels, GEBufferFormat srcPixelFormat, int srcStride, int width, int height) override;
	virtual void DrawFramebufferToOutput(const u8 *srcPixels, GEBufferFormat srcPixelFormat, int srcStride, bool applyPostShader) override;
	
	void DrawActiveTexture(float x, float y, float w, float h, float destW, float destH, float u0, float v0, float u1, float v1, int uvRotation);

	void DestroyAllFBOs(bool forceDelete);

	void EndFrame();
	void Resized() override;
	void DeviceLost();
	void CopyDisplayToOutput();
	void ReformatFramebufferFrom(VirtualFramebuffer *vfb, GEBufferFormat old) override;

	void BlitFramebufferDepth(VirtualFramebuffer *src, VirtualFramebuffer *dst) override;

	void BindFramebufferColor(int stage, VirtualFramebuffer *framebuffer, int flags);

	void ReadFramebufferToMemory(VirtualFramebuffer *vfb, bool sync, int x, int y, int w, int h) override;
	void DownloadFramebufferForClut(u32 fb_address, u32 loadBytes) override;

	std::vector<FramebufferInfo> GetFramebufferList();

	virtual bool NotifyStencilUpload(u32 addr, int size, bool skipZero = false) override;

	bool GetCurrentFramebuffer(GPUDebugBuffer &buffer, GPUDebugFramebufferType type, int maxRes);
	bool GetCurrentDepthbuffer(GPUDebugBuffer &buffer);
	bool GetCurrentStencilbuffer(GPUDebugBuffer &buffer);
	bool GetOutputFramebuffer(GPUDebugBuffer &buffer);

	virtual void RebindFramebuffer() override;

	LPDIRECT3DSURFACE9 GetOffscreenSurface(LPDIRECT3DSURFACE9 similarSurface, VirtualFramebuffer *vfb);
	LPDIRECT3DSURFACE9 GetOffscreenSurface(D3DFORMAT fmt, u32 w, u32 h);

protected:
	void DisableState() override;
	void ClearBuffer(bool keepState = false) override;
	void FlushBeforeCopy() override;
	void DecimateFBOs() override;

	// Used by ReadFramebufferToMemory and later framebuffer block copies
	void BlitFramebuffer(VirtualFramebuffer *dst, int dstX, int dstY, VirtualFramebuffer *src, int srcX, int srcY, int w, int h, int bpp) override;

	bool CreateDownloadTempBuffer(VirtualFramebuffer *nvfb) override;
	void UpdateDownloadTempBuffer(VirtualFramebuffer *nvfb) override;

private:
	void MakePixelTexture(const u8 *srcPixels, GEBufferFormat srcPixelFormat, int srcStride, int width, int height);
	void CompileDraw2DProgram();
	void DestroyDraw2DProgram();

	void PackFramebufferDirectx9_(VirtualFramebuffer *vfb, int x, int y, int w, int h);
	void PackDepthbuffer(VirtualFramebuffer *vfb, int x, int y, int w, int h);
	static bool GetRenderTargetFramebuffer(LPDIRECT3DSURFACE9 renderTarget, LPDIRECT3DSURFACE9 offscreen, int w, int h, GPUDebugBuffer &buffer);

	LPDIRECT3DDEVICE9 device_;

	// Used by DrawPixels
	LPDIRECT3DTEXTURE9 drawPixelsTex_;
	int drawPixelsTexW_;
	int drawPixelsTexH_;

	LPDIRECT3DVERTEXSHADER9 pFramebufferVertexShader;
	LPDIRECT3DPIXELSHADER9 pFramebufferPixelShader;
	LPDIRECT3DVERTEXDECLARATION9 pFramebufferVertexDecl;

	u8 *convBuf;

	int plainColorLoc_;
	LPDIRECT3DPIXELSHADER9 stencilUploadPS_;
	LPDIRECT3DVERTEXSHADER9 stencilUploadVS_;
	bool stencilUploadFailed_;

	TextureCacheDX9 *textureCacheDX9_;
	ShaderManagerDX9 *shaderManager_;
	DrawEngineDX9 *drawEngine_;
	
	// Used by post-processing shader
	std::vector<Draw::Framebuffer *> extraFBOs_;

	bool resized_;

	struct TempFBO {
		Draw::Framebuffer *fbo;
		int last_frame_used;
	};
	struct OffscreenSurface {
		LPDIRECT3DSURFACE9 surface;
		int last_frame_used;
	};

	std::map<u64, TempFBO> tempFBOs_;
	std::map<u64, OffscreenSurface> offscreenSurfaces_;

#if 0
	AsyncPBO *pixelBufObj_; //this isn't that large
	u8 currentPBO_;
#endif
};

};
