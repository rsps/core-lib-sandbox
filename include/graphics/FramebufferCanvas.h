#ifndef FRAMEBUFFERCANVAS_H
#define FRAMEBUFFERCANVAS_H

#include <linux/fb.h>

#include "graphics/BufferedCanvas.h"
#include "graphics/primitives/Canvas.h"

namespace rsp::graphics
{

class Framebuffer : public BufferedCanvas
{
  public:
    Framebuffer();
    virtual ~Framebuffer();

    inline void SetPixel(const Point &aPoint, const Color aColor)
    {
        if (!IsInsideScreen(aPoint)) {
            return;
        }
        long location = (aPoint.mX + mVariableInfo.xoffset) * (mVariableInfo.bits_per_pixel / 8) + aPoint.mY * mFixedInfo.line_length;
        *(reinterpret_cast<uint32_t *>(mpBackBuffer + location)) = aColor;
    }

    uint32_t GetPixel(const Point &aPoint, const bool aFront = false) const;

    void SwapBuffer(const SwapOperations aSwapOp = SwapOperations::Copy);

  protected:
    int mFramebufferFile;
    int mTtyFb = 0;
    struct fb_fix_screeninfo mFixedInfo;
    struct fb_var_screeninfo mVariableInfo;

    void clear();
    void copy();
};

} // namespace rsp::graphics
#endif // FRAMEBUFFERCANVAS_H
