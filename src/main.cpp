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

enum class TextureType : int { Player = 1, Wall, Box, Goal, Ground };

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

struct Player
{
  TextureType tex;
  Vec4 rect{0, 0, 64, 64};
};

enum class LevelObject
  {
   Player = 1,
   Wall,
   Box,
   Goal,
   Unknown
  };

struct Level {
  Level(int levelWidth, int levelHeight, std::string levelDescription)
    : width(levelWidth), height(levelHeight)
  {
    for (auto object : levelDescription) {
      level.emplace_back(static_cast<LevelObject>(std::atoi(&object)));
    }
  }
  
  int width;
  int height;
  std::vector<LevelObject> level;

  Vec2 findPlayerPosition() {
    for (std::size_t i = 0; i < width; ++i) {
      for (std::size_t j = 0; j < height; ++j) {
	if (level[j * width + i] == LevelObject::Player) {
	  std::cout << "i " << i << " j " << j << '\n';
	  return {i, j};
	}
      }
    }
    return {-1, -1};
  }
};


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



  Level level{8, 3,
	      "22222222"
	      "20103042"
	      "22222222"};
  
  Player player;
  Vec2 playerStart = level.findPlayerPosition();
  player.rect.x = playerStart.x * constants::object_width;
  player.rect.y = playerStart.y * constants::object_height;
  
  SDL_Event event;
  bool running = true;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT: {
	running = false;
      } break;
      case SDL_KEYDOWN: {
	int player_x = static_cast<int>(player.rect.x) / constants::object_width;
	int player_y = static_cast<int>(player.rect.y) / constants::object_height;

	std::cout << player_x << ' ' << player_y << '\n';
	
	switch (event.key.keysym.sym) {
	case SDLK_UP: {
	  if (level.level[(player_y - 1) * level.width + (player_x)] != LevelObject::Wall)
	  player.rect.y -= 64;
	} break;
	case SDLK_DOWN: {
	  if (level.level[(player_y + 1) * level.width + (player_x)] != LevelObject::Wall)
	  player.rect.y += 64;	  
	} break;
	case SDLK_LEFT:{
	  if (level.level[(player_y) * level.width + (player_x - 1)] != LevelObject::Wall)
	  player.rect.x -= 64;
	} break;
	case SDLK_RIGHT:{
	  if (level.level[(player_y) * level.width + (player_x + 1)] != LevelObject::Wall)
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

    sdl2::copyToRenderer(renderer, textures[TextureType::Ground], {0, 0, 0, 0});
    
    glm::vec2 position{0, 0};

    for (auto object : level.level) {
      switch (object)
	{
	case LevelObject::Wall:
	  sdl2::copyToRenderer(renderer, textures[TextureType::Wall], {static_cast<int>(position.x),
								       static_cast<int>(position.y),
								       constants::object_width,
								       constants::object_height});
	  break;
	case LevelObject::Player:
	  // done afterwards so that player always looks above some of the objects
	  break;
	case LevelObject::Box:
	  sdl2::copyToRenderer(renderer, textures[TextureType::Box], {static_cast<int>(position.x),
								       static_cast<int>(position.y),
								       constants::object_width,
								       constants::object_height});
	  break;
	case LevelObject::Goal:
	  sdl2::copyToRenderer(renderer, textures[TextureType::Goal], {static_cast<int>(position.x),
								       static_cast<int>(position.y),
								       constants::object_width,
								       constants::object_height});
	  break;
	default:
	  break;	  
	}      

      position.x += constants::object_width;
      if (position.x >= constants::object_width * level.width) {
	position.x = 0;
	position.y += constants::object_height;
      }
     
    }

    sdl2::copyToRenderer(renderer, textures[TextureType::Player], {static_cast<int>(player.rect.x),
								   static_cast<int>(player.rect.y),
								   static_cast<int>(player.rect.z),
								   static_cast<int>(player.rect.w)});
    
    sdl2::renderScreen(renderer);

    SDL_Delay(10);
  }


  std::cout << "Bye, bye\n"; 
  
  return EXIT_SUCCESS;
}
