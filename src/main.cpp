#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include "sdl2.hpp"

int main()
{
  
  std::cout << "Hello, World!\n";

  if (!sdl2::init()) {
    return EXIT_FAILURE;
  }
  
  auto window = sdl2::make_window("SOKOBAN", 800, 600);

  if (!window) {
    return EXIT_FAILURE;
  }

  auto renderer = sdl2::make_renderer(window);
  
  auto player = sdl2::make_surface(64, 64);
  sdl2::fill(player, 0, 255, 0);

  auto box = sdl2::make_surface(64, 64);
  sdl2::fill(box, 255, 255, 0);

  auto playerTex = sdl2::make_texture(renderer, player);

  std::vector<sdl2::sdl_texture_ptr> textures;
  textures.push_back(sdl2::make_texture(renderer, player));

  sdl2::fill(renderer, {100, 100, 100, 100});
  
  SDL_Event event;
  bool running = true;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT:
	{
	  running = false;
	} break;
      }
    }
    
    sdl2::clear(renderer);
    sdl2::copyToRenderer(renderer, textures);
    sdl2::renderScreen(renderer);
  }


  std::cout << "Bye, bye\n"; 
  
  return EXIT_SUCCESS;
}
