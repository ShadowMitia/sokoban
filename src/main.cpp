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

  void applyForce(Vec2 force, float deltaTime) {
    if (force == Vec2(0, 0)) return;
    Vec2 acceleration = force / mass;

    velocity += acceleration * deltaTime;

    auto velLength = glm::length(velocity);
    if (velLength > 2) {
      velocity = glm::normalize(velocity) * 2.f;
    }

    position += velocity * deltaTime;// + 0.5f * (acceleration) * (deltaTime * deltaTime);
  }
};

template<class C>
struct GameObject
{
  TextureType tex;
  LevelObject type;
  Vec4 rect{0, 0, constants::tile_width, constants::tile_height};
};

struct Box : public GameObject<Box>, public Physics<Box>
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

std::size_t indexOfFurthestPoint(std::vector<Vec2> shape, Vec2 direction) {
  float maxProduct = glm::dot(direction, shape[0]);
  size_t index = 0;
  for (size_t i = 1; i < shape.size(); i++) {
    float product = glm::dot(direction, shape[i]);
    if (product > maxProduct) {
      maxProduct = product;
      index = i;
    }
  }
  return index;
}

Vec2 averagePoint (std::vector<Vec2> points) {
  Vec2 avg = { 0.f, 0.f };
  for (size_t i = 0; i < points.size(); i++) {
    avg.x += points[i].x;
    avg.y += points[i].y;
  }
  avg.x /= points.size();
  avg.y /= points.size();
  return avg;
}

//-----------------------------------------------------------------------------
// Triple product expansion is used to calculate perpendicular normal vectors 
// which kinda 'prefer' pointing towards the Origin in Minkowski space

Vec2 tripleProduct (Vec2 a, Vec2 b, Vec2 c) {
  return b * (glm::dot(a, c)) - a * (glm::dot(b, c));
}

namespace collision {

  Vec2 support(std::vector<Vec2> shape1, std::vector<Vec2> shape2, Vec2 direction) {
    std::size_t i = indexOfFurthestPoint(shape1, direction);
    std::size_t j = indexOfFurthestPoint(shape2, -direction);

    return shape1[i] - shape2[j];
  }
  
  bool pointInRect(Vec2 point, Vec4 rect) {

    return (point.x >= rect.x
	    && point.y >= rect.y
	    && point.x <= rect.x + rect.z
	    && point.y <= rect.y + rect.w);
  }
  
  std::pair<std::vector<Vec2>, bool> GJK(std::vector<Vec2> const& shape1, std::vector<Vec2> const& shape2) {
    size_t index = 0; // index of current vertex of simplex
    Vec2 a, b, c, d, ao, ab, ac, abperp, acperp;
    std::vector<Vec2> simplex(3);
    
    Vec2 position1 = averagePoint (shape1); // not a CoG but
    Vec2 position2 = averagePoint (shape2); // it's ok for GJK )

    // initial direction from the center of 1st body to the center of 2nd body
    d = position1 - position2;
    
    // if initial direction is zero – set it to any arbitrary axis (we choose X)
    if ((d.x == 0) && (d.y == 0))
      d.x = 1.f;
    
    // set the first support as initial point of the new simplex
    simplex[0] = support (shape1, shape2, d);
    a = simplex[0];
    
    if (glm::dot(a, d) <= 0)
      return {simplex, false}; // no collision
    
    d = -a; // The next search direction is always towards the origin, so the next search direction is negate(a)
    
    while (true) {
        
      a = simplex[++index] = support (shape1, shape2, d);
        
      if (glm::dot(a, d) <= 0)
	return{simplex, false}; // no collision
        
      ao = -a; // from point A to Origin is just negative A
        
      // simplex has 2 points (a line segment, not a triangle yet)
      if (index < 2) {
	b = simplex[0];
	ab = b - a; // from point A to B
	d = tripleProduct (ab, ao, ab); // normal to AB towards Origin
	if (glm::length (d) == 0)
	  d = [](Vec2 v) { Vec2 p = { v.y, -v.x }; return p; } (ab);
	continue; // skip to next iteration
      }
        
      b = simplex[1];
      c = simplex[0];
      ab = b - a; // from point A to B
      ac = c - a; // from point A to C
        
      acperp = tripleProduct (ab, ac, ac);
        
      if (glm::dot(acperp, ao) >= 0) {
            
	d = acperp; // new direction is normal to AC towards Origin
            
      } else {
            
	abperp = tripleProduct (ac, ab, ab);
            
	if (glm::dot (abperp, ao) < 0)
	  return {simplex, true}; // collision
            
	simplex[0] = simplex[1]; // swap first element (point C)

	d = abperp; // new direction is normal to AB towards Origin
      }
        
      simplex[1] = simplex[2]; // swap element in the middle (point B)
      --index;
    }
    
    return {simplex, false};
  }

  std::pair<std::vector<Vec2>, bool> GJK(Vec4 shape1, Vec4 shape2) {
    return GJK(std::vector<Vec2>{Vec2{shape1.x, shape1.y},
				   Vec2{shape1.x + shape1.z, shape1.y},
				     Vec2{shape1.x, shape1.y + shape1.w},
				       Vec2{shape1.x + shape1.z, shape1.y + shape1.w}},
      std::vector<Vec2>{Vec2{shape2.x, shape2.y},
			  Vec2{shape2.x + shape2.z, shape2.y},
			    Vec2{shape2.x, shape2.y + shape2.w},
			      Vec2{shape2.x + shape2.z, shape2.y + shape2.w}});
  }

  std::pair<Vec2, float> EPA(std::vector<Vec2> const& shape1, std::vector<Vec2> const& shape2, std::vector<Vec2> simplex) {
    std::cout << "Collision resolution\n";	 
    bool iterations = true;
    while (iterations) {	    
      int closestIndex = -1;
      Vec2 closestNormal;
      float closestDistance = std::numeric_limits<float>::max();
	  
      for (std::size_t i = 0; i < simplex.size(); ++i) {
	Vec2 a = simplex[i];
	Vec2 b = i >= simplex.size()-1 ? simplex[0] : simplex[i+1];
	Vec2 e = b - a;
	Vec2 oa = a;
	Vec2 n = tripleProduct(e, oa, e);
	n = glm::normalize(n);
	float d = glm::dot(a, n);

	std::cout << "a: " << a.x << ' ' << a.y << '\n';
	std::cout << "b: " << b.x << ' ' << b.y << '\n';
	std::cout << "e: " << e.x << ' ' << e.y << '\n';
	std::cout << "oa: " << oa.x << ' ' << oa.y << '\n';
	std::cout << "n: " << n.x << ' ' << n.y << '\n';
	std::cout << "d: " << d << '\n';
	std::cout << "------\n";

	if (d < closestDistance && !std::isnan(d)) {
	  closestNormal = n;
	  closestDistance = d;
	  closestIndex = (i+1) % simplex.size();
	}
      }

      std::cout << "SELECTED\n"; std::cout << "n: " << closestNormal.x << ' ' << closestNormal.y << '\n'; std::cout << "dist: " << closestDistance << '\n'; std::cout << "index: " << closestIndex << '\n'; std::cout << "=====\n";

      Vec2 p = collision::support(shape1, shape2, closestNormal);
      float dist = glm::dot(p, closestNormal);
      std::cout << p.x << ' ' << p.y << ' ' << dist << ' ' << closestNormal.x << ' ' << closestNormal.y << '\n';
      constexpr float epsilon = std::numeric_limits<float>::epsilon();
	    
      if (dist > -epsilon && dist < epsilon) {
	std::cout << "Done!\n";
	iterations = false;
      } else {
	simplex.insert(simplex.begin() + closestIndex, p);
      }

      return {closestNormal, closestDistance};
    }

  }

}

int main()
{

  /*
  std::vector<Vec2> shape1{Vec2{4, 11}, Vec2{9, 9}, Vec2{4, 5}};
  std::vector<Vec2> shape2{Vec2{5, 7}, Vec2{12, 7}, Vec2{10, 2}, Vec2{7, 3}};
  std::vector<Vec2> edges{Vec2(4, 2), Vec2(-8, -2), Vec2(-1, -2)};

  collision::EPA(shape1, shape2, edges);

  return 42;
  */
  
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
  
  player.position.x = playerStart.x * constants::tile_width;
  player.position.y = playerStart.y * constants::tile_height;
  
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

    currentSlice += lastFt;
    
    /* UPDATE */

    Vec4 playerShape;
    playerShape.x = player.position.x - player.rect.z / 2;
    playerShape.y = player.position.y - player.rect.w;
    playerShape.z = player.rect.z;
    playerShape.w = player.rect.w;

    std::vector<Vec2> playerShapeVec{
				     {playerShape.x, playerShape.y},
				     {playerShape.x + playerShape.z, playerShape.y},
				     {playerShape.x + playerShape.z, playerShape.y + playerShape.w},
				     {playerShape.x, playerShape.y + playerShape.w}
				     
    };
    
    for (;currentSlice >= ftSlice; currentSlice -= ftSlice) {
      Vec2 accel{next_player_x, next_player_y};    
      player.applyForce(accel, frame_period{1}.count());

      for (auto obj : level.boxes) {

	auto res = collision::GJK(playerShape, obj.rect);
	
	if (res.second) {

	  std::vector<Vec2> objectVec{
				     {obj.rect.x, obj.rect.y},
				     {obj.rect.x + obj.rect.z, obj.rect.y},
				     {obj.rect.x + obj.rect.z, obj.rect.y + obj.rect.w},
				     {obj.rect.x, obj.rect.y + obj.rect.w}
				     
    };
    
	  
	  auto res2 = collision::EPA(playerShapeVec, objectVec, res.first);

	  player.position -= res2.first * res2.second;
	  
	}
      }
    }
      
      /* DRAWING */
    
      sdl2::clear(renderer);

      sdl2::copyToRenderer(renderer, textures[TextureType::Ground], {0, 0, 0, 0});
    
      Vec2 position{0, 0};

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

      auto endFrame = clock::now();
      auto elapsedTime{endFrame - beginFrame};

      float ft{std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(elapsedTime).count()};

      lastFt = ft;

      auto ftSeconds(ft / 1000.f);
      auto fps(1.f / ftSeconds);

      //std::cout << "FT: " + std::to_string(ft) + "\nFPS: " + std::to_string(fps) + "\n";
      SDL_Delay(10);
    }


    std::cout << "Bye, bye\n"; 
  
    return EXIT_SUCCESS;
  }
