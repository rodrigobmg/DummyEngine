#pragma once

#include <Ren/Texture.h>

#include "BaseElement.h"

namespace Gui {
class Image : public BaseElement {
protected:
    Ren::TextureRegionRef   tex_;
    Vec2f                   uvs_px_[2];
public:
    Image(const Ren::TextureRegionRef &tex, const Vec2f &pos, const Vec2f &size, const BaseElement *parent);
    Image(Ren::Context &ctx, const char *tex_name, const Vec2f &pos, const Vec2f &size, const BaseElement *parent);

    void set_uvs(const Vec2f uvs[2]) {
        uvs_px_[0] = uvs[0];
        uvs_px_[1] = uvs[1];
    }

    void Draw(Renderer *r) override;
};
}
