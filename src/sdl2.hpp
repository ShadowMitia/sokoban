#ifndef SDL2_H
#define SDL2_H

#include <memory>
#include <vector>
#include <string>
#include <array>

#include <SDL2/SDL.h>

namespace sdl2
{

  void logError(std::string&& str);
  
  struct SDLWindowDestroyer
  {
    void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
  };

  struct SDLSurfaceDestroyer
  {
    void operator()(SDL_Surface* s) const { SDL_FreeSurface(s); }
  };

  struct SDLSystemDestroyer
  {
    void operator()(unsigned int* i) const { delete i; SDL_Quit(); }
  };

  struct SDLTextureDestroyer
  {
    void operator()(SDL_Texture* t) const { SDL_DestroyTexture(t); }
  };

  struct SDLRendererDestroyer
  {
    void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
  };

  using sdl_window_ptr = std::unique_ptr<SDL_Window, SDLWindowDestroyer>;
  using sdl_surface_ptr = std::unique_ptr<SDL_Surface, SDLSurfaceDestroyer>;
  using sdl_renderer_ptr = std::unique_ptr<SDL_Renderer, SDLRendererDestroyer>;
  using sdl_texture_ptr = std::unique_ptr<SDL_Texture, SDLTextureDestroyer>;
  
  sdl_window_ptr make_window(std::string&& title, int width, int height);

  sdl_surface_ptr make_surface(int width, int height);

  sdl_renderer_ptr make_renderer(sdl_window_ptr& w);

  sdl_texture_ptr make_texture(sdl_renderer_ptr& r, sdl_surface_ptr& s);

  std::unique_ptr<unsigned int, SDLSystemDestroyer> init(unsigned int initFlags = SDL_INIT_VIDEO);

  Uint32 RGB(SDL_Surface* sur, Uint8 r, Uint8 g, Uint8 b);

  Uint32 RGB(sdl_surface_ptr& sur, Uint8 r, Uint8 g, Uint8 b);

  void fill(sdl_surface_ptr& sur, Uint8 r, Uint8 g, Uint8 b);

  void fill(sdl_renderer_ptr& ren, std::array<int, 4>& rect);
  
  void fill(sdl_renderer_ptr& ren, std::array<int, 4>&& rect);

  void clear(sdl_renderer_ptr& renderer);

  void copyToRenderer(sdl_renderer_ptr& renderer, sdl_texture_ptr& textures, std::array<int, 4>& destination);
  void copyToRenderer(sdl_renderer_ptr& renderer, sdl_texture_ptr& textures, std::array<int, 4>&& destination);  

  void renderScreen(sdl_renderer_ptr& renderer);

}

#endif /* SDL2_H */
