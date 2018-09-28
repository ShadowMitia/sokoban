#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

#include "sdl2.hpp"

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

namespace constants {
  constexpr int tile_width{64};
  constexpr int tile_height{64};
}

enum class TextureType : int { Player = 1, Wall, Box, Goal, BoxOnGoal, Ground, Test };

std::unordered_map<TextureType, sdl2::sdl_texture_ptr> buildTextures(sdl2::sdl_renderer_ptr& renderer)
{
  auto playerSurface = sdl2::make_surface(constants::tile_width, constants::tile_height);
  sdl2::fill(playerSurface, 0, 255, 0);

  auto boxSurface = sdl2::make_surface(constants::tile_width, constants::tile_height);
  sdl2::fill(boxSurface, 255, 255, 0);

  auto wallSurface = sdl2::make_surface(constants::tile_width, constants::tile_height);
  sdl2::fill(wallSurface, 80, 80, 80);

  auto groundSurface = sdl2::make_surface(constants::tile_width, constants::tile_height);
  sdl2::fill(groundSurface, 0, 50, 0);

  auto goalSurface = sdl2::make_surface(constants::tile_width, constants::tile_height);
  sdl2::fill(goalSurface, 255, 0, 255);

  auto boxOnGoalSurface = sdl2::make_surface(constants::tile_width, constants::tile_height);
  sdl2::fill(boxOnGoalSurface, 100, 0, 100);

  auto testSurface = sdl2::make_surface(constants::tile_width, constants::tile_height);
  sdl2::fill(testSurface, 100, 0, 100);

  std::unordered_map<TextureType, sdl2::sdl_texture_ptr> textures;
  
  textures[TextureType::Player]      = sdl2::make_texture(renderer, playerSurface);
  textures[TextureType::Wall]        = sdl2::make_texture(renderer, wallSurface);
  textures[TextureType::Box]         = sdl2::make_texture(renderer, boxSurface);
  textures[TextureType::Ground]      = sdl2::make_texture(renderer, groundSurface);
  textures[TextureType::Goal]        = sdl2::make_texture(renderer, goalSurface);
  textures[TextureType::BoxOnGoal]   = sdl2::make_texture(renderer, boxOnGoalSurface);
  textures[TextureType::Test]        = sdl2::make_texture(renderer, testSurface);

  return textures;
}

enum class LevelObject : int
  {
   Player = 1,
   Wall,
   Box,
   Goal,
   Unknown
  };

Vec2 lerp(Vec2 start, Vec2 end, float percent) {
  return start + (end - start) * percent;
}

template<class C>
struct Physics {
  Vec2 position{0};
  Vec2 velocity{0};  
  Vec2 drag{10};
  float mass{1};

  void move(Vec2 acceleration, float deltaTime = 1.0f) {
    if (acceleration == Vec2(0, 0)) return;
    acceleration /= mass;

    velocity += acceleration * deltaTime;

    auto velLength = glm::length(velocity);
    if (velLength > 2) {
      velocity = glm::normalize(velocity) * 2.f;
    }

    position += velocity * deltaTime + 0.5f * (acceleration) * (deltaTime * deltaTime);
  }
};

template<class C>
struct GameObject
{
  TextureType tex;
  LevelObject type;
  Vec4 rect{0, 0, constants::tile_width, constants::tile_height};
};

struct Box : public GameObject<Box>
{
  TextureType tex = TextureType::Box;
  LevelObject type = LevelObject::Box;
  bool onGoal = false;
};


struct Player : public GameObject<Player>, public Physics<Player>
{
};

enum class KeyEvents {
		      UPKEY = 0,
		      DOWNKEY,
		      LEFTKEY,
		      RIGHTKEY,
		      // ONLY USE IT FOR MAKING ARRAYS
		      MAX
};

struct Level {
  Level(unsigned int levelWidth, unsigned int levelHeight, std::string levelDescription)
    : width(levelWidth), height(levelHeight)
  {
    float i = 0;
    float j = 0;
    for (auto object : levelDescription) {
      auto obj = static_cast<LevelObject>(std::atoi(&object));
      if (obj == LevelObject::Box) {
	boxes.emplace_back();
	boxes.back().rect.x = i * constants::tile_width;
	boxes.back().rect.y = j * constants::tile_height;
	level.emplace_back(LevelObject::Unknown);
      } else {
	level.emplace_back(obj);
      }
      i++;
      if (static_cast<unsigned int>(i) >= width) {
	i = 0.f;
	j += 1.0f;
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
  
  Player player;
  
  Vec2 playerStart = level.findPlayerPosition();
  player.rect.x = playerStart.x * constants::tile_width;
  player.rect.y = playerStart.y * constants::tile_height;
  
  SDL_Event event;
  bool running = true;

  std::vector<bool> keys(static_cast<int>(KeyEvents::MAX), false);

  while (running) {

    int player_x = static_cast<int>(player.position.x) / constants::tile_width;
    int player_y = static_cast<int>(player.position.y) / constants::tile_height;
    int next_player_x = 0;
    int next_player_y = 0;
      
    /* EVENTS */
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT: {
	running = false;
      } break;
      case SDL_KEYDOWN: {
	switch (event.key.keysym.sym) {
	case SDLK_UP: {
	  keys[static_cast<int>(KeyEvents::UPKEY)] = true;
	} break;
	case SDLK_DOWN: {	
	  keys[static_cast<int>(KeyEvents::DOWNKEY)] = true;
	} break;
	case SDLK_LEFT:{
	  keys[static_cast<int>(KeyEvents::LEFTKEY)] = true;
	} break;
	case SDLK_RIGHT:{
	  keys[static_cast<int>(KeyEvents::RIGHTKEY)] = true;
	} break;	
	}
      } break;
      case SDL_KEYUP: {
	switch (event.key.keysym.sym) {
	case SDLK_UP: {
	  keys[static_cast<int>(KeyEvents::UPKEY)] = false;
	} break;
	case SDLK_DOWN: {	
	  keys[static_cast<int>(KeyEvents::DOWNKEY)] = false;
	} break;
	case SDLK_LEFT:{
	  keys[static_cast<int>(KeyEvents::LEFTKEY)] = false;
	} break;
	case SDLK_RIGHT:{
	  keys[static_cast<int>(KeyEvents::RIGHTKEY)] = false;
	} break;
	case SDLK_ESCAPE:{
	  running = false;
	}break;
	}	
      } break;
      }
    }

    if (keys[static_cast<int>(KeyEvents::UPKEY)]) {
      next_player_y -= 1;
    }
    
    if (keys[static_cast<int>(KeyEvents::DOWNKEY)]) {
      next_player_y += 1;
    }

    if (keys[static_cast<int>(KeyEvents::RIGHTKEY)]) {
      next_player_x += 1;
    }

    if (keys[static_cast<int>(KeyEvents::LEFTKEY)]) {
      next_player_x -= 1;
    }

    /* UPDATE */

    Vec2 accel{next_player_x, next_player_y};
    
    player.move(accel);

    // auto nextStep = (player_y + next_player_y) * level.width + (player_x + next_player_x);	  
    
    // if (std::none_of(immovableObjects.begin(),
    // 		     immovableObjects.end(),
    // 		     [&](auto obj){ return obj == (level.level[nextStep]);} )) {     

    //   GameObject nextPlayer = player;
    //   nextPlayer.move(next_player_x * constants::tile_width,
    // 		      next_player_y * constants::tile_height);
      
    //   auto collider = collisionCheck(nextPlayer, level.boxes);
      
    //   if (collider) {

    // 	int collider_x = static_cast<int>(collider->rect.x) / constants::tile_width;
    // 	int collider_y = static_cast<int>(collider->rect.y) / constants::tile_height;
    // 	GameObject nextCollider = *collider;
    // 	nextCollider.move(next_player_x * constants::tile_width,
    // 		      next_player_y * constants::tile_height);
	
    // 	auto nextNextStep = (collider_y + next_player_y) * level.width + (collider_x + next_player_x);
	
    // 	if (!collisionCheck(nextCollider, level.boxes) && std::none_of(immovableObjects.begin(),
    // 			 immovableObjects.end(),
    // 			 [&](auto obj){ return obj == (level.level[nextNextStep]);} )) {
    // 	  collider->onGoal =  (level.level[nextNextStep] == LevelObject::Goal);

    // 	  collider->move(next_player_x * constants::tile_width,
    // 			 next_player_y * constants::tile_height);
    // 	  player.move(next_player_x * constants::tile_width,
    // 		      next_player_y * constants::tile_height);
    // 	}
    //   } else {
    // 	player.move(next_player_x * constants::tile_width, next_player_y * constants::tile_height);	
    //   } 
    // }
    
     
    /* DRAWING */
    
    sdl2::clear(renderer);

    sdl2::copyToRenderer(renderer, textures[TextureType::Ground], {0, 0, 0, 0});
    
    glm::vec2 position{0, 0};

    for (auto object : level.level) {
      switch (object)
	{
	case LevelObject::Wall:
	  sdl2::copyToRenderer(renderer, textures[TextureType::Wall], {static_cast<int>(position.x),
								       static_cast<int>(position.y),
								       constants::tile_width,
								       constants::tile_height});
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
								       constants::tile_width,
								       constants::tile_height});
	  break;
	default:
	  break;	  
	}      

      position.x += constants::tile_width;
      if (position.x >= static_cast<float>(constants::tile_width * level.width)) {
	position.x = 0;
	position.y += constants::tile_height;
      }
     
    }

    for (auto box : level.boxes) {
      auto boxPosition = box.rect;
      if (box.onGoal) {
	sdl2::copyToRenderer(renderer, textures[TextureType::BoxOnGoal], {static_cast<int>(boxPosition.x),
									  static_cast<int>(boxPosition.y),
									  constants::tile_width,
									  constants::tile_height});
      } else {
	sdl2::copyToRenderer(renderer, textures[TextureType::Box], {static_cast<int>(boxPosition.x),
								    static_cast<int>(boxPosition.y),
								    constants::tile_width,
								    constants::tile_height});
      }
    }

    sdl2::copyToRenderer(renderer, textures[TextureType::Test], {
								 player_x * constants::tile_width,
								 player_y * constants::tile_height,
								 static_cast<int>(player.rect.z),
								 static_cast<int>(player.rect.w)
      });
    
    sdl2::copyToRenderer(renderer, textures[TextureType::Player], {static_cast<int>(player.position.x - player.rect.z / 2),
								   static_cast<int>(player.position.y - player.rect.w),
								   static_cast<int>(player.rect.z),
								   static_cast<int>(player.rect.w)});
    
    sdl2::renderScreen(renderer);

    SDL_Delay(10);
  }


  std::cout << "Bye, bye\n"; 
  
  return EXIT_SUCCESS;
}
