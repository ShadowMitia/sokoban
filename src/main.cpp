#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <thread>
#include <limits>

#include "sdl2.hpp"

#include "collisions.hpp"

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

enum class TextureType : int { Player = 1, Wall, Box, Goal, BoxOnGoal, Ground, Test, Red, Green, Blue };

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

  auto redSurface = sdl2::make_surface(constants::tile_width, constants::tile_height);
  sdl2::fill(redSurface, 255, 0, 0);

  auto greenSurface = sdl2::make_surface(constants::tile_width, constants::tile_height);
  sdl2::fill(greenSurface, 0, 255, 0);

  auto blueSurface = sdl2::make_surface(constants::tile_width, constants::tile_height);
  sdl2::fill(blueSurface, 0, 0, 255);
  

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

Vec2 lerp(Vec2 start, Vec2 end, float percent) {
  return start + (end - start) * percent;
}

template<class C>
struct Physics {
  Vec2 velocity{0};  
  Vec2 drag{10};
  float mass{1};

  void applyForce(Vec2 force, float deltaTime) {
    if (force == Vec2(0, 0)) return;
    Vec2 acceleration = force / mass;

    velocity += acceleration * deltaTime;

    auto velLength = glm::length(velocity);
    if (velLength > 2) {
      velocity = glm::normalize(velocity) * 2.f;
    }


    derived.rect.x += velocity.x * deltaTime;
    derived.rect.y += velocity.y * deltaTime;
  }

private:
  Physics(){};
  C& derived = static_cast<C&>(*this);
  friend C;
};

struct GameObject : public Physics<GameObject>
{

  GameObject(TextureType texT) : tex(texT) {
    
  }
  
  TextureType tex;
  Vec4 rect{0, 0, constants::tile_width, constants::tile_height};
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

  enum class LevelObject : int
    {
     Player = 1,
     Wall,
     Box,
     Goal,
     Unknown
    };
  
  Level(unsigned int levelWidth, unsigned int levelHeight, std::string levelDescription)
    : width(levelWidth), height(levelHeight)
  {
    for (std::size_t i = 0; i < width; ++i) {
      for (std::size_t j = 0; j < height; ++j) {

	auto piece = levelDescription[i + width * j];
	auto obj = static_cast<LevelObject>(std::atoi(&piece));
	switch (obj) {
	case LevelObject::Box:
	  objects.emplace_back(TextureType::Box);
	  objects.back().rect.x = i * constants::tile_width;
	  objects.back().rect.y = j * constants::tile_height;
	  break;
	case LevelObject::Wall:
	  objects.emplace_back(TextureType::Wall);
	  objects.back().rect.x = i * constants::tile_width;
	  objects.back().rect.y = j * constants::tile_height;
	  break;
	case LevelObject::Player:
	  playerStartPosition = Vec2(i, j);
	  break;
	}
      }
    }     
  }
    
  
  std::size_t width;
  std::size_t height;
  Vec2 playerStartPosition;

  std::vector<GameObject> objects;
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
  
  GameObject player(TextureType::Player);
  
  Vec2 playerStart = level.playerStartPosition;

  player.rect.x = playerStart.x * constants::tile_width;
  player.rect.y = playerStart.y * constants::tile_height;
  
  SDL_Event event;
  bool running = true;

  std::vector<bool> keys(static_cast<int>(KeyEvents::MAX), false);

  using clock = std::chrono::high_resolution_clock;
  using frame_period = std::chrono::duration<long long, std::ratio<1, 60>>;
 

  constexpr float ftStep{1.f};
  constexpr float ftSlice{1.f};

  float lastFt{0.f};
  float currentSlice{0.f};
  
  while (running) {

    auto beginFrame = clock::now();
    
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

    currentSlice += lastFt;
    
    /* UPDATE */

    Vec4 playerShape;
    playerShape.x = player.rect.x - player.rect.z / 2;
    playerShape.y = player.rect.y - player.rect.w;
    playerShape.z = player.rect.z;
    playerShape.w = player.rect.w;

    std::vector<Vec2> playerShapeVec{
				     {playerShape.x,  playerShape.y},
				     {playerShape.x + playerShape.z,  playerShape.y},
				     {playerShape.x + playerShape.z,  playerShape.y + playerShape.w},
				     {playerShape.x,  playerShape.y + playerShape.w}
				     
    };
    
    for (;currentSlice >= ftSlice; currentSlice -= ftSlice) {
      
      Vec2 accel{next_player_x, next_player_y};    
      player.applyForce(accel, frame_period{1}.count());
      
      for (auto& obj : level.objects) {

	auto res = collision::GJK(playerShape, obj.rect);
	
	if (res.second) {

	  std::vector<Vec2> objectVec{
				     {obj.rect.x, obj.rect.y},
				     {obj.rect.x + obj.rect.z, obj.rect.y},
				     {obj.rect.x + obj.rect.z, obj.rect.y + obj.rect.w},
				     {obj.rect.x, obj.rect.y + obj.rect.w}
				     
	  };
    
	  
	  auto res2 = collision::EPA(playerShapeVec, objectVec, res.first);

	  player.rect.x -= (res2.first.x) * (res2.second);
	  player.rect.y -= (res2.first.y) * (res2.second);

	  // make player slide against wall
	  player.velocity -= glm::normalize(res2.first) * glm::dot(player.velocity, glm::normalize(res2.first));
	  
	}
      }
      
    }
      
      /* DRAWING */
    
      sdl2::clear(renderer);

      sdl2::copyToRenderer(renderer, textures[TextureType::Ground], {0, 0, 0, 0});
    
      Vec2 rect{0, 0};

      for (auto& object : level.objects) {
	auto objectPosition = object.rect;
	  sdl2::copyToRenderer(renderer, textures[object.tex], {static_cast<int>(objectPosition.x),
								static_cast<int>(objectPosition.y),
								constants::tile_width,
								constants::tile_height});
      }
    
      sdl2::copyToRenderer(renderer, textures[TextureType::Player], {static_cast<int>(player.rect.x - player.rect.z / 2),
								     static_cast<int>(player.rect.y - player.rect.w),
								     static_cast<int>(player.rect.z),
								     static_cast<int>(player.rect.w)});
    
      sdl2::renderScreen(renderer);

      auto endFrame = clock::now();
      auto elapsedTime{endFrame - beginFrame};

      float ft{std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(elapsedTime).count()};

      lastFt = ft;
      /*
      auto ftSeconds(ft / 1000.f);
      auto fps(1.f / ftSeconds);

      //std::cout << "FT: " + std::to_string(ft) + "\nFPS: " + std::to_string(fps) + "\n";
      */
      SDL_Delay(1);
    }


    std::cout << "Bye, bye\n"; 
  
    return EXIT_SUCCESS;
  }
