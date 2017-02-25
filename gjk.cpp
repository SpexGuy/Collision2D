//
// Created by Martin Wickham on 2/25/2017.
//

#include "gjk.h"

using namespace glm;
using namespace std;

vec2 AddCollider2D::findSupport(vec2 direction) {
    return a->findSupport(direction) + b->findSupport(direction);
}

vec2 CircleCollider2D::findSupport(vec2 direction) {
    return center + radius * normalize(direction);
}

vec2 PolygonCollider2D::findSupport(vec2 direction) {
    float max_dot = -numeric_limits<float>::infinity();
    vec2 max_val;
    for (vec2 &point : points) {
        float distance = dot(direction, point);
        if (distance > max_dot) {
            max_dot = distance;
            max_val = point;
        }
    }
    return max_val;
}

// recursively finds points on the collider's surface in ccw order, defining it to within epsilon of its mathematical definition
static void findBounds(Collider2D *collider, vec2 right, vec2 left, vector<vec2> &bounds, float epsilon) {
    vec2 edge = left - right; // the ccw direction around the triangle
    vec2 out = vec2(edge.y, -edge.x); // the direction towards the unknown
    vec2 supp = collider->findSupport(out);
    if (supp == left || supp == right)
        return;

    vec2 suppDir = supp - right;
    float dist = abs(dot(suppDir, normalize(out)));
    if (dist <= epsilon)
        return;

    findBounds(collider, right, supp, bounds, epsilon);
    bounds.push_back(supp);
    findBounds(collider, supp, left, bounds, epsilon);
}

void findBounds(Collider2D *collider, vector<vec2> &bounds, float epsilon) {
    vec2 top = collider->findSupport(vec2(0, 1));
    vec2 bottom = collider->findSupport(vec2(0, -1));
    if (top == bottom) {
        bounds.push_back(top);
        return;
    }

    bounds.push_back(top);
    findBounds(collider, top, bottom, bounds, epsilon);
    bounds.push_back(bottom);
    findBounds(collider, bottom, top, bounds, epsilon);
}

bool intersects(Collider2D *a, Collider2D *b) {
    return false;
}
