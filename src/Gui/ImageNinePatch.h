#pragma once

#include "Image.h"

namespace Gui {
class ImageNinePatch : public Image {
protected:
    Vec2f offset_px_;
    float frame_scale_;
public:
    ImageNinePatch(const Ren::TextureRegionRef &tex, const Vec2f &offset_px, float frame_scale,
          const Vec2f &pos, const Vec2f &size, const BaseElement *parent);
    ImageNinePatch(Ren::Context &ctx, const char *tex_name, const Vec2f &offset_px, float frame_scale,
          const Vec2f &pos, const Vec2f &size, const BaseElement *parent);

    Ren::TextureRegionRef &tex() {
        return tex_;
    }

    void set_frame_scale(float scale) { frame_scale_ = scale; }

    void Draw(Renderer *r) override;
};
}
