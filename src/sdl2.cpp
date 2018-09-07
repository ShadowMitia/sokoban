#include "sdl2.hpp"

#include <iostream>

namespace sdl2 {

  void logError(std::string&& str) {
    std::cout << "[SDL ERROR] " << str << ' ' << SDL_GetError() << '\n';
  }
  
  sdl_window_ptr make_window(std::string&& title, int width, int height) {

    auto const windowPos = SDL_WINDOWPOS_CENTERED;
    auto const flags = SDL_WINDOW_SHOWN;
    
    sdl_window_ptr window{
			  SDL_CreateWindow(title.c_str(),
					   windowPos,
					   windowPos,
					   width,
					   height,
					   flags),
			  SDLWindowDestroyer{}
    };

    if (!window) {
      logError("Couldn't load window");
      window.reset();
    }
    
    return window;
  }

  sdl_surface_ptr make_surface(int width, int height) {
    sdl_surface_ptr surface{
			    SDL_CreateRGBSurface(0,
						 width,
						 height,
						 32,
						 0,
						 0,
						 0,
						 0),
			    SDLSurfaceDestroyer{}};

    if (!surface) {
      logError("Couldn't create surface");
      surface.reset();
    }
   
    return surface;
  }

  sdl_renderer_ptr make_renderer(sdl_window_ptr& w) {
    sdl_renderer_ptr ren{
			 SDL_CreateRenderer(w.get(),
					    -1,
					    SDL_RENDERER_ACCELERATED),
			 SDLRendererDestroyer{}};

    if (!ren) {
      logError("Couldn't create renderer");
      ren.reset();
    }

    return ren;
  }

  sdl_texture_ptr make_texture(sdl_renderer_ptr& r, sdl_surface_ptr& s) {
    sdl_texture_ptr tex{
			SDL_CreateTextureFromSurface(r.get(), s.get()),
			SDLTextureDestroyer{}
    };

    if (!tex) {
      logError("Couldn't create texture");
      tex.reset();
    }

    return tex;
  }

  std::unique_ptr<unsigned int, SDLSystemDestroyer> init(unsigned int initFlags) {
    unsigned int* systemFlags = new unsigned int(initFlags);
    
    std::unique_ptr<unsigned int, SDLSystemDestroyer> system{systemFlags, SDLSystemDestroyer{}};
    
    if (SDL_Init(initFlags) < 0) {
      logError("Couldn't load SDL");
      system.reset();
    }
    
    return system;
  }

  Uint32 RGB(SDL_Surface* sur, Uint8 r, Uint8 g, Uint8 b) {
    return SDL_MapRGB(sur->format, r, g, b);
  }

  Uint32 RGB(sdl_surface_ptr& sur, Uint8 r, Uint8 g, Uint8 b) {
    return SDL_MapRGB(sur->format, r, g, b);
  }

  void fill(sdl_surface_ptr& sur, Uint8 r, Uint8 g, Uint8 b) {
    SDL_FillRect(sur.get(), nullptr, RGB(sur, r, g, b));
  }

  void fill(sdl_renderer_ptr& ren, std::array<int, 4>& rect) {
    SDL_Rect r;
    r.x = rect[0];
    r.y = rect[1];
    r.w = rect[2];
    r.h = rect[3];
    SDL_RenderFillRect(ren.get(), &r);    
  }
  
  void fill(sdl_renderer_ptr& ren, std::array<int, 4>&& rect) {
    SDL_Rect r;
    r.x = rect[0];
    r.y = rect[1];
    r.w = rect[2];
    r.h = rect[3];
    SDL_RenderFillRect(ren.get(), &r);
  }

  void clear(sdl_renderer_ptr& renderer) {
    SDL_RenderClear(renderer.get());
  }

  void copyToRenderer(sdl_renderer_ptr& renderer, sdl_texture_ptr& texture, std::array<int, 4>& destination) {
    SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
      SDL_Rect dest{destination[0], destination[1], destination[2], destination[3]};
      SDL_RenderCopy(renderer.get(), texture.get(), nullptr, &dest);
  }

  void copyToRenderer(sdl_renderer_ptr& renderer, sdl_texture_ptr& texture, std::array<int, 4>&& destination) {
    SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
      SDL_Rect dest{destination[0], destination[1], destination[2], destination[3]};
      SDL_RenderCopy(renderer.get(), texture.get(), nullptr, &dest);
  }

  void renderScreen(sdl_renderer_ptr& renderer) {
    SDL_RenderPresent(renderer.get());
  }
	
}
