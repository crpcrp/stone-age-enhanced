#pragma once
// How the SDL host shows the game's 320x200 picture in its window.
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_render.h>
#include "machine.hpp"
#include "settings.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace stone {
// Scale2x (AdvMAME2x): each pixel becomes four, and a corner takes the colour
// of the two neighbours it lies between when they match and the pixel is on
// an edge of theirs. Staircases of pixel art become diagonals; areas of one
// colour, gradients and dithering stay as they are.
inline void scale2x(const std::vector<uint32_t>& in, unsigned width, unsigned height,
                    std::vector<uint32_t>& out) {
    out.resize(in.size() * 4);
    for (unsigned y = 0; y < height; ++y) {
        const uint32_t* row = &in[y * width];
        const uint32_t* above = &in[(y ? y - 1 : y) * width];
        const uint32_t* below = &in[(y + 1 < height ? y + 1 : y) * width];
        uint32_t* top = &out[2 * y * 2 * width];
        uint32_t* bottom = top + 2 * width;
        for (unsigned x = 0; x < width; ++x) {
            const unsigned left = x ? x - 1 : x, right = x + 1 < width ? x + 1 : x;
            const uint32_t p = row[x], b = above[x], d = row[left], f = row[right], h = below[x];
            const bool edge = b != h && d != f;
            top[2 * x] = edge && d == b ? d : p;
            top[2 * x + 1] = edge && b == f ? f : p;
            bottom[2 * x] = edge && d == h ? d : p;
            bottom[2 * x + 1] = edge && h == f ? f : p;
        }
    }
}

// Not original (ENHANCEMENTS.md, E5). With enhanced mode off the picture is
// letterboxed into the window and scaled by nearest pixel, as it always was.
// In enhanced mode the display preset (Settings::Display) decides:
// - sharp: square pixels, scaled by whole numbers only;
// - crt: 4:3, as a DOS monitor showed 320x200, so each pixel is 1.2 times as
//   tall as wide. SDL's pixel-art scaling keeps the pixels sharp at any size
//   (whole multiples by nearest pixel, smoothed only where they meet), and
//   dark scanlines are laid over it, worked out for the window's rows;
// - smooth: 4:3, the picture enlarged four times by Scale2x twice, then
//   smoothly to the window, which rounds off the staircases of the pixels.
// Screenshots are taken from the game's picture, never from the window.
class GameView {
public:
    explicit GameView(SDL_Renderer* renderer) : renderer(renderer) {
        picture = make_texture(SDL_PIXELFORMAT_XRGB8888, 320, 200);
    }
    ~GameView() {
        for (SDL_Texture* texture : {picture, enlarged, scanlines})
            if (texture)
                SDL_DestroyTexture(texture);
    }
    GameView(const GameView&) = delete;
    GameView& operator=(const GameView&) = delete;

    // A new picture from the game.
    void update(const Image& image) {
        pixels.resize(320 * 200);
        for (unsigned n = 0; n < 320 * 200; ++n)
            pixels[n] = uint32_t(image.rgb[3 * n]) << 16 | uint32_t(image.rgb[3 * n + 1]) << 8 |
                        image.rgb[3 * n + 2];
        check(SDL_UpdateTexture(picture, nullptr, pixels.data(), 320 * 4));
        enlarged_current = false;
    }

    // The window, drawn and shown.
    void present(bool enhanced, unsigned display) {
        const int mode = enhanced ? int(std::min(display, Settings::displays - 1)) : -1;
        if (mode != configured) {
            configured = mode;
            check(SDL_SetRenderLogicalPresentation(renderer, mode < 0 ? 320 : 0, mode < 0 ? 200 : 0,
                                                   mode < 0 ? SDL_LOGICAL_PRESENTATION_LETTERBOX
                                                            : SDL_LOGICAL_PRESENTATION_DISABLED));
            check(SDL_SetTextureScaleMode(picture, mode == Settings::crt ? SDL_SCALEMODE_PIXELART
                                                                         : SDL_SCALEMODE_NEAREST));
        }
        check(SDL_SetRenderDrawColor(renderer, 16, 16, 16, 255));
        check(SDL_RenderClear(renderer));
        if (mode < 0)
            check(SDL_RenderTexture(renderer, picture, nullptr, nullptr));
        else {
            int width = 0, height = 0;
            check(SDL_GetCurrentRenderOutputSize(renderer, &width, &height));
            const SDL_FRect area = place(Settings::Display(mode), width, height);
            if (mode == Settings::sharp)
                check(SDL_RenderTexture(renderer, picture, nullptr, &area));
            else if (mode == Settings::smooth) {
                enlarge();
                check(SDL_RenderTexture(renderer, enlarged, nullptr, &area));
            } else
                draw_crt(area);
        }
        SDL_RenderPresent(renderer);
    }

private:
    static void check(bool ok) {
        if (!ok)
            throw std::runtime_error(SDL_GetError());
    }
    SDL_Texture* make_texture(SDL_PixelFormat format, int width, int height) {
        SDL_Texture* texture =
            SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!texture)
            throw std::runtime_error(SDL_GetError());
        return texture;
    }

    // Where in a window of this many pixels the picture goes, centred.
    static SDL_FRect place(Settings::Display display, int width, int height) {
        float w, h;
        if (display == Settings::sharp) {
            const int whole = std::min(width / 320, height / 200);
            if (whole >= 1) {
                w = 320.0f * whole;
                h = 200.0f * whole;
            } else { // a window smaller than the picture: as much as fits
                const float scale = std::min(width / 320.0f, height / 200.0f);
                w = 320 * scale;
                h = 200 * scale;
            }
        } else {
            const float scale = std::min(width / 320.0f, height / 240.0f);
            w = std::round(320 * scale);
            h = std::round(240 * scale);
        }
        return SDL_FRect{std::floor((width - w) / 2), std::floor((height - h) / 2), w, h};
    }

    void enlarge() {
        if (enlarged_current)
            return;
        if (!enlarged) {
            enlarged = make_texture(SDL_PIXELFORMAT_XRGB8888, 1280, 800);
            check(SDL_SetTextureScaleMode(enlarged, SDL_SCALEMODE_LINEAR));
        }
        scale2x(pixels, 320, 200, doubled);
        scale2x(doubled, 640, 400, quadrupled);
        check(SDL_UpdateTexture(enlarged, nullptr, quadrupled.data(), 1280 * 4));
        enlarged_current = true;
    }

    // The picture, brightened, and over it a column of the window's rows
    // multiplying it: bright in the middle of each of the game's rows, dark
    // where two meet. Each window row takes the average over its height, so
    // the lines stay even at any size. Below three window rows to a game row
    // there is no room for lines, and the picture is shown without them.
    void draw_crt(const SDL_FRect& area) {
        const int rows = int(area.h);
        const float per_line = area.h / 200;
        if (per_line < 3) {
            check(SDL_RenderTexture(renderer, picture, nullptr, &area));
            return;
        }
        if (rows != scanline_rows) {
            if (scanlines)
                SDL_DestroyTexture(scanlines);
            scanlines = make_texture(SDL_PIXELFORMAT_XRGB8888, 1, rows);
            check(SDL_SetTextureScaleMode(scanlines, SDL_SCALEMODE_NEAREST));
            check(SDL_SetTextureBlendMode(scanlines, SDL_BLENDMODE_MOD));
            std::vector<uint32_t> column(rows);
            const int samples = 8;
            for (int y = 0; y < rows; ++y) {
                float sum = 0;
                for (int s = 0; s < samples; ++s) {
                    const float line = (y + (s + 0.5f) / samples) / per_line;
                    const float within = std::sin(3.14159265f * (line - std::floor(line)));
                    sum += 1 - depth + depth * within * within;
                }
                const uint32_t level = uint32_t(std::lround(255 * sum / samples));
                column[y] = level << 16 | level << 8 | level;
            }
            check(SDL_UpdateTexture(scanlines, nullptr, column.data(), 4));
            scanline_rows = rows;
        }
        check(SDL_SetRenderColorScale(renderer, brighten));
        check(SDL_RenderTexture(renderer, picture, nullptr, &area));
        check(SDL_SetRenderColorScale(renderer, 1));
        const SDL_FRect lines{area.x, area.y, area.w, float(rows)};
        check(SDL_RenderTexture(renderer, scanlines, nullptr, &lines));
    }

    // How dark the lines get between rows, and how much brighter the picture
    // is drawn to make up for them.
    static constexpr float depth = 0.45f, brighten = 1.2f;
    SDL_Renderer* renderer;
    SDL_Texture* picture = nullptr;
    SDL_Texture* enlarged = nullptr; // smooth
    SDL_Texture* scanlines = nullptr; // crt
    int configured = -2, scanline_rows = 0;
    bool enlarged_current = false;
    std::vector<uint32_t> pixels, doubled, quadrupled;
};
} // namespace stone
