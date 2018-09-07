#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include "sdl2.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

namespace constants {
  constexpr int object_width{64};
  constexpr int object_height{64};
}

enum class TextureType : int { Player = 1, Wall = 2, Box, Ground, Goal };

struct GameObject
{ 
  TextureType type;
  Vec3 colour;
  Vec4 rect{0, 0, 64, 64};
};

std::unordered_map<TextureType, sdl2::sdl_texture_ptr> buildTextures(sdl2::sdl_renderer_ptr& renderer)
{
  auto playerSurface = sdl2::make_surface(constants::object_width, constants::object_height);
  sdl2::fill(playerSurface, 0, 255, 0);

  auto boxSurface = sdl2::make_surface(constants::object_width, constants::object_height);
  sdl2::fill(boxSurface, 255, 255, 0);

  auto wallSurface = sdl2::make_surface(constants::object_width, constants::object_height);
  sdl2::fill(wallSurface, 80, 80, 80);

  auto groundSurface = sdl2::make_surface(constants::object_width, constants::object_height);
  sdl2::fill(groundSurface, 0, 50, 0);

  auto goalSurface = sdl2::make_surface(constants::object_width, constants::object_height);
  sdl2::fill(goalSurface, 255, 0, 255);

  std::unordered_map<TextureType, sdl2::sdl_texture_ptr> textures;  
  textures[TextureType::Player] = sdl2::make_texture(renderer, playerSurface);
  textures[TextureType::Wall]   = sdl2::make_texture(renderer, wallSurface);
  textures[TextureType::Box]    = sdl2::make_texture(renderer, boxSurface);
  textures[TextureType::Ground] = sdl2::make_texture(renderer, groundSurface);
  textures[TextureType::Goal]   = sdl2::make_texture(renderer, goalSurface);

  return textures;
}

struct Level {
  int level_width;
  int level_height;
  std::string objects;
};

Level level1{8, 3,
	     "22222222"
	     "20103052"
	     "22222222"};

int main()
{

  if (!sdl2::init()) {
    return EXIT_FAILURE;
  }
  
  auto window = sdl2::make_window("SOKOBAN", 800, 600);

  if (!window) {
    return EXIT_FAILURE;
  }
 
  auto renderer = sdl2::make_renderer(window);
  
  auto textures = buildTextures(renderer); 

  GameObject player;
  player.rect.x = constants::object_width;
  player.rect.y = constants::object_height;

  GameObject box;
  box.rect.x = constants::object_width*2;
  box.rect.y = constants::object_height;
  
  SDL_Event event;
  bool running = true;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT: {
	running = false;
      } break;
      case SDL_KEYDOWN: {
	switch (event.key.keysym.sym) {
	case SDLK_UP: {
	  player.rect.y -= 64;
	} break;
	case SDLK_DOWN: {
	  player.rect.y += 64;	  
	} break;
	case SDLK_LEFT:{
	  player.rect.x -= 64;
	} break;
	case SDLK_RIGHT:{
	  player.rect.x += 64;
	} break;
	case SDLK_ESCAPE:{
	  running = false;
	}break;
	}
      }	break;
      }
    }
    
    sdl2::clear(renderer);

    glm::vec2 position{0, 0};

    for (auto object : level1.objects) {
      switch (static_cast<TextureType>(std::atoi(&object)))
	{
	case TextureType::Wall:
	  sdl2::copyToRenderer(renderer, textures[TextureType::Wall], {static_cast<int>(position.x),
								       static_cast<int>(position.y),
								       constants::object_width,
								       constants::object_height});
	  break;
	case TextureType::Player:
	  sdl2::copyToRenderer(renderer, textures[TextureType::Player], {static_cast<int>(player.rect.x),
									 static_cast<int>(player.rect.y),
									 static_cast<int>(player.rect.z),
									 static_cast<int>(player.rect.w)});
	  break;
	case TextureType::Box:
	  sdl2::copyToRenderer(renderer, textures[TextureType::Box], {static_cast<int>(position.x),
								       static_cast<int>(position.y),
								       constants::object_width,
								       constants::object_height});
	  break;
	case TextureType::Goal:
	  sdl2::copyToRenderer(renderer, textures[TextureType::Goal], {static_cast<int>(position.x),
								       static_cast<int>(position.y),
								       constants::object_width,
								       constants::object_height});
	  break;
	default:
	  std::cout << "toto\n";
	  break;
	  
      }

      position.x += constants::object_width;
      if (position.x >= constants::object_width * level1.level_width) {
	position.x = 0;
	position.y += constants::object_height;
      }
     
    }
    
    sdl2::renderScreen(renderer);

    SDL_Delay(10);
  }


  std::cout << "Bye, bye\n"; 
  
  return EXIT_SUCCESS;
}
