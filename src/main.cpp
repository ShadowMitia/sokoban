#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

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

enum class TextureType : int { Player = 1, Wall, Box, Goal, BoxOnGoal, Ground };

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

  auto boxOnGoalSurface = sdl2::make_surface(constants::object_width, constants::object_height);
  sdl2::fill(boxOnGoalSurface, 100, 0, 100);

  std::unordered_map<TextureType, sdl2::sdl_texture_ptr> textures;
  
  textures[TextureType::Player]      = sdl2::make_texture(renderer, playerSurface);
  textures[TextureType::Wall]        = sdl2::make_texture(renderer, wallSurface);
  textures[TextureType::Box]         = sdl2::make_texture(renderer, boxSurface);
  textures[TextureType::Ground]      = sdl2::make_texture(renderer, groundSurface);
  textures[TextureType::Goal]        = sdl2::make_texture(renderer, goalSurface);
  textures[TextureType::BoxOnGoal]   = sdl2::make_texture(renderer, boxOnGoalSurface);

  return textures;
}

struct GameObject
{
  TextureType tex;
  Vec4 rect{0, 0, constants::object_width, constants::object_height};

  void move(int deltaX, int deltaY) {
    rect.x += deltaX;
    rect.y += deltaY;
  }
};

enum class LevelObject : int
  {
   Player = 1,
   Wall,
   Box,
   Goal,
   Unknown
  };

struct Box : public GameObject
{
  TextureType tex = TextureType::Box;  
  bool onGoal = false;
};

struct Level {
  Level(unsigned int levelWidth, unsigned int levelHeight, std::string levelDescription)
    : width(levelWidth), height(levelHeight)
  {
    // TODO: clear this up
    int i = 0;
    int j = 0;
    for (auto object : levelDescription) {
      auto obj = static_cast<LevelObject>(std::atoi(&object));
      if (obj == LevelObject::Box) {
	boxes.emplace_back();
	boxes.back().rect.x = i * constants::object_width;
	boxes.back().rect.y = j * constants::object_height;
	level.emplace_back(LevelObject::Unknown);
      } else {
	level.emplace_back(obj);
      }
      i++;
      if (i >= width) {
	i = 0;
	j++;
      }
    }
  }
  
  unsigned int width;
  unsigned int height;
  Vec2 playerStartPosition;
  std::vector<LevelObject> level;
  std::vector<Box>         boxes;

  // TODO: change this to be done in constructor + memorise start position of boxes
  Vec2 findPlayerPosition() {
    for (std::size_t i = 0; i < width; ++i) {
      for (std::size_t j = 0; j < height; ++j) {
	if (level[j * width + i] == LevelObject::Player) {
	  return {i, j};
	}
      }
    }
    return {-1, -1};
  }
};

bool pointInRect(Vec2 point, Vec4 rect) {

  return (point.x >= rect.x
	  && point.y >= rect.y
	  && point.x <= rect.x + rect.z
	  && point.y <= rect.y + rect.w);
}

template<class T>
T* collisionCheck(GameObject player, std::vector<T>& objects) {
  Vec2 playerCenter{
		    player.rect.x + player.rect.z / 2,
		    player.rect.y + player.rect.w / 2};
  
  for (auto& obj : objects) {
    if (pointInRect(playerCenter, obj.rect)) {
      return &obj;
    }
  }
  
  return nullptr;
}


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

  Level level{10, 9,
	      "2222222222"
	      "2000000002"
	      "2000000002"
	      "2000304002"
	      "2010304002"
	      "2000000002"
	      "2000000002"
	      "2000000002"
	      "2222222222"};
  
  GameObject player;
  Vec2 playerStart = level.findPlayerPosition();
  player.rect.x = playerStart.x * constants::object_width;
  player.rect.y = playerStart.y * constants::object_height;

  auto immovableObjects = {LevelObject::Wall};	
  
  SDL_Event event;
  bool running = true;
  while (running) {

    unsigned int player_x = static_cast<unsigned int>(player.rect.x) / constants::object_width;
    unsigned int player_y = static_cast<unsigned int>(player.rect.y) / constants::object_height;
    unsigned int next_player_x = 0;
    unsigned int next_player_y = 0;
    
    // TODO: create a table for pressed keys?
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT: {
	running = false;
      } break;
      case SDL_KEYDOWN: {

	// TODO: clear up this mess
	
	switch (event.key.keysym.sym) {
	case SDLK_UP: {
	  next_player_y = -1;	  
	} break;
	case SDLK_DOWN: {
	  next_player_y = 1;
	} break;
	case SDLK_LEFT:{
	  next_player_x = -1;
	} break;
	case SDLK_RIGHT:{
	  next_player_x = 1;
	  }
	} break;
	case SDLK_ESCAPE:{
	  running = false;
	}break;
	}	
      }	break;
    }

    auto nextStep = (player_y + next_player_y) * level.width + (player_x + next_player_x);	  
    
    if (std::none_of(immovableObjects.begin(),
		     immovableObjects.end(),
		     [&](auto obj){ return obj == (level.level[nextStep]);} )) {

      player.move(next_player_x * constants::object_width, next_player_y * constants::object_height);
      
      auto collider = collisionCheck(player, level.boxes);
      
      if (collider) {

	auto nextNextStep = (player_y + next_player_y * 2) * level.width + (player_x + next_player_x*2);
	
	if (std::none_of(immovableObjects.begin(),
			 immovableObjects.end(),
			 [&](auto obj){ return obj == (level.level[nextNextStep]);} )) {
	  if (level.level[nextNextStep] == LevelObject::Goal) {
	    collider->onGoal = true;
	    collider->move(next_player_x * constants::object_width, next_player_y * constants::object_height);

	  } else if (level.level[nextNextStep] != LevelObject::Goal) {
	    collider->onGoal = false;
	    collider->move(next_player_x * constants::object_width, next_player_y * constants::object_height);
	  }
	  
	}  else {
	          player.move(-next_player_x * constants::object_width, -next_player_y * constants::object_height);
	  }
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
	  // done afterwards because dynamic
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
      if (position.x >= static_cast<float>(constants::object_width * level.width)) {
	position.x = 0;
	position.y += constants::object_height;
      }
     
    }

    for (auto box : level.boxes) {
      auto position = box.rect;
      if (box.onGoal) {
	sdl2::copyToRenderer(renderer, textures[TextureType::BoxOnGoal], {static_cast<int>(position.x),
									  static_cast<int>(position.y),
									  constants::object_width,
									  constants::object_height});
      } else {
      sdl2::copyToRenderer(renderer, textures[TextureType::Box], {static_cast<int>(position.x),
								  static_cast<int>(position.y),
								  constants::object_width,
								  constants::object_height});
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
