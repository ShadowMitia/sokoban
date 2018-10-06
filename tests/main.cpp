#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "../src/collisions.hpp"

TEST_CASE("Collisions", "[collisions]") {
  std::vector<Vec2> shape1{Vec2{4, 11}, Vec2{9, 9}, Vec2{4, 5}};
  std::vector<Vec2> shape2{Vec2{5, 7}, Vec2{12, 7}, Vec2{10, 2}, Vec2{7, 3}};
  std::vector<Vec2> shape3{Vec2{-4, 11}, Vec2{-9, 9}, Vec2{-4, 5}};

  SECTION("Has collision") {
    auto response = collision::GJK(shape1, shape2);

    REQUIRE( response.second == true );
  }

  SECTION("Doesn't have collision") {
    auto response = collision::GJK(shape1, shape3);

    REQUIRE(response.second == false);
  }
}
