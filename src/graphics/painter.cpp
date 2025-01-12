#include "graphics/painter.h"

#include "game/game.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "platform/renderer.h"

#include <string>
#include <unordered_map>

#include <SDL.h>

void painter::draw(SDL_Texture *texture, vec2i pos, vec2i offset, vec2i size, color color, float scale_x, float scale_y, ImgFlags flags) {
    if (!(flags & ImgFlag_Grayscale)) {
        draw_impl(texture, pos, offset, size, color, scale_x, scale_y, flags);
    } else {
        draw_grayscale(texture, pos, offset, size, scale_x, scale_y, !!(flags & ImgFlag_Alpha));
    }
}

void painter::draw_impl(SDL_Texture *texture, vec2i pos, vec2i offset, vec2i size, color color, float scale_x, float scale_y, ImgFlags flags) {
    if (texture == nullptr) {
        return;
    }

    if (!color) {
        color = COLOR_MASK_NONE;
    }

    const float x_scale_factor = scale_x * global_render_scale;
    const float y_scale_factor = scale_y * global_render_scale;
    const float overall_scale_factor = scale_x == scale_y ? x_scale_factor : this->global_render_scale;
    bool DOWNSCALED_CITY = false;
    if (this->global_render_scale < 1.0f) {
        DOWNSCALED_CITY = true;
    }

    graphics_renderer()->set_texture_scale_mode(texture, overall_scale_factor);

    SDL_SetTextureColorMod(texture,
                           (color & COLOR_CHANNEL_RED) >> COLOR_BITSHIFT_RED,
                           (color & COLOR_CHANNEL_GREEN) >> COLOR_BITSHIFT_GREEN,
                           (color & COLOR_CHANNEL_BLUE) >> COLOR_BITSHIFT_BLUE);
    SDL_SetTextureAlphaMod(texture, (color & COLOR_CHANNEL_ALPHA) >> COLOR_BITSHIFT_ALPHA);

    const bool alpha = !!(flags & ImgFlag_Alpha);
    SDL_SetTextureBlendMode(texture, alpha ? SDL_BLENDMODE_BLEND : (SDL_BlendMode)graphics_renderer()->premult_alpha());

    // uncomment here if you want save something from atlases
    int k = 0;
    if (k == 1) {
        char filename[32] = {0};
        static int index = 0;
        sprintf(filename, "%u_img.bmp", index);
        graphics_renderer()->save_texture_to_file(filename, texture);
    }

    float texture_coord_correction = 0;
    SDL_Rect texture_coords = {static_cast<int>(offset.x + texture_coord_correction),
                               static_cast<int>(offset.y + texture_coord_correction),
                               static_cast<int>(size.x - texture_coord_correction),
                               static_cast<int>(size.y - texture_coord_correction)};

    SDL_FRect screen_coords;
    if (DOWNSCALED_CITY) {
        // hack to prevent ugly dark borders around sprites -- yes, there's DEFINITELY a better way to do this,
        // but I can't be arsed to find it. I tried, I gave up.
        screen_coords = {static_cast<float>(pos.x * this->global_render_scale - 0.25),
                         static_cast<float>(pos.y * this->global_render_scale - 0.25),
                         static_cast<float>(size.x * x_scale_factor + 0.5),
                         static_cast<float>(size.y * y_scale_factor + 0.5)};
    } else {
        screen_coords = {pos.x * this->global_render_scale,
                         pos.y * this->global_render_scale,
                         size.x * x_scale_factor,
                         size.y * y_scale_factor};
    }

    if (!!(flags & ImgFlag_Mirrored)) {
        SDL_RenderCopyExF(this->renderer, texture, &texture_coords, &screen_coords, 0, nullptr, SDL_FLIP_HORIZONTAL);
    } else {
        SDL_RenderCopyExF(this->renderer, texture, &texture_coords, &screen_coords, 0, nullptr, SDL_FLIP_NONE);
    }
}

std::unordered_map<uint64_t, SDL_Texture *> grayscaled_txs;

SDL_Texture* painter::convertToGrayscale(SDL_Texture *tx, vec2i offset, vec2i size) {
    if (!tx) {
        return nullptr;
    }

    uint64_t hash = ((uint64_t)tx & 0xffffffff) | ((uint64_t)offset.x << 32) | ((uint64_t)offset.y << 48);

    auto it = grayscaled_txs.find(hash);
    if (it != grayscaled_txs.end()) {
        return it->second;
    }

    auto gray_tx = grayscaled_txs.insert({ hash , nullptr });

    uint32_t format;
    int w, h;

    /* Get information about texture we want to save */
    int st = SDL_QueryTexture(tx, &format, NULL, &w, &h);
    if (st != 0) {
        return nullptr;
    }

    if (SDL_BYTESPERPIXEL(format) != 4) {
        return nullptr;
    }

    SDL_Texture* ren_tex = SDL_CreateTexture(this->renderer, format, SDL_TEXTUREACCESS_TARGET, size.x, size.y);
    if (!ren_tex) {
        return nullptr;
    }

    struct RenTexDeleter {
        SDL_Texture *tx;
        ~RenTexDeleter() { SDL_DestroyTexture(tx); }
    } _renTexDeleter{ ren_tex };

    /*
     * Initialize our canvas, then copy texture to a target whose pixel data we
     * can access
     */
    SDL_Texture *old_target = SDL_GetRenderTarget(this->renderer);
    st = SDL_SetRenderTarget(this->renderer, ren_tex);
    if (st != 0) {
        return nullptr;
    }

    SDL_SetRenderDrawColor(this->renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(this->renderer);

    SDL_Rect srcrect{ offset.x, offset.y, size.x, size.y };
    st = SDL_RenderCopy(this->renderer, tx, &srcrect, NULL);
    if (st != 0) {
        return nullptr;
    }

    /* Create buffer to hold texture data and load it */
    std::vector<uint32_t> pixels(size.x * size.y);
    st = SDL_RenderReadPixels(this->renderer, NULL, format, pixels.data(), size.x * SDL_BYTESPERPIXEL(format));

    const SDL_PixelFormat *pxformat = SDL_AllocFormat(format);
    for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {
            Uint32 *pixel = pixels.data() + y * size.x + x;
            Uint8 r, g, b, a;
            SDL_GetRGBA(*pixel, pxformat, &r, &g, &b, &a);
            Uint8 gray = static_cast<Uint8>(0.3 * r + 0.59 * g + 0.11 * b);
            *pixel = SDL_MapRGBA(pxformat, gray, gray, gray, a);
        }
    }

    /* Copy pixel data over to surface */
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(), size.x, size.y, SDL_BITSPERPIXEL(format), size.x * SDL_BYTESPERPIXEL(format), format);
    if (!surface) {
        return nullptr;
    }

    SDL_Texture *gray_tx_ptr = SDL_CreateTextureFromSurface(this->renderer, surface);

    gray_tx.first->second = gray_tx_ptr;
    SDL_FreeSurface(surface);

    SDL_SetRenderTarget(this->renderer, old_target);

    return gray_tx_ptr;
}

void painter::draw_grayscale(SDL_Texture *texture, vec2i pos, vec2i offset, vec2i size, float scale_x, float scale_y, ImgFlags flags) {
    if (texture == nullptr) {
        return;
    }

    SDL_Texture *grtx = convertToGrayscale(texture, offset, size);
    if (!grtx) { 
        return;
    }

    draw_impl(grtx, pos, vec2i{ 0, 0 }, size, COLOR_WHITE, scale_x, scale_y, flags);
}

void painter::draw(const sprite &spr, vec2i pos, color color_mask, float scale_x, float scale_y, ImgFlags flags) {
    if (spr.img == nullptr) {
        return;
    }

    vec2i offset = spr.img->atlas.offset;
    vec2i size = spr.img->size();
    draw(spr.img->atlas.p_atlas->texture, pos, offset, size, color_mask, scale_x, scale_y, flags);
}

sprite_resource_icon::sprite_resource_icon(e_resource res) {
    int image_id = image_id_resource_icon(res);
    img = image_get(image_id);
}
