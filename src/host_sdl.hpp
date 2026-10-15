#pragma once
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_error.h>
#include <stdexcept>
namespace stone {
inline void configure_game_output(SDL_Renderer* renderer, SDL_Texture* texture) {
    if (!SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST) ||
        !SDL_SetRenderLogicalPresentation(renderer, 320, 200, SDL_LOGICAL_PRESENTATION_LETTERBOX))
        throw std::runtime_error(SDL_GetError());
}
} // namespace stone
