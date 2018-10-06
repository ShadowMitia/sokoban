#ifndef COLLISIONS_H
#define COLLISIONS_H

#include <vector>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

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

	if (d < closestDistance && !std::isnan(d)) {
	  closestNormal = n;
	  closestDistance = d;
	  closestIndex = (i+1) % simplex.size();
	}
      }

      Vec2 p = collision::support(shape1, shape2, closestNormal);
      float dist = glm::dot(p, closestNormal);
      constexpr float epsilon = std::numeric_limits<float>::epsilon();
	    
      if (dist > -epsilon && dist < epsilon) {
	iterations = false;
      } else {
	simplex.insert(simplex.begin() + closestIndex, p);
      }

      return {closestNormal, closestDistance};
    }

  }

}


#endif /* COLLISIONS_H */
