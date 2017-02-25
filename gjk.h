//
// Created by Martin Wickham on 2/25/2017.
//

#ifndef COLISION2D_GJK_H
#define COLISION2D_GJK_H

#include <glm/glm.hpp>
#include <vector>

struct Collider2D {
    virtual glm::vec2 findSupport(glm::vec2 direction) = 0;
};

struct AddCollider2D : public Collider2D {
    Collider2D *a;
    Collider2D *b;
    glm::vec2 findSupport(glm::vec2 direction) override;
};

struct PolygonCollider2D : public Collider2D {
    std::vector<glm::vec2> points;
    glm::vec2 findSupport(glm::vec2 direction) override;
};

struct CircleCollider2D : public Collider2D {
    glm::vec2 center;
    float radius;
    glm::vec2 findSupport(glm::vec2 direction) override;
};

void findBounds(Collider2D *collider, std::vector<glm::vec2> &bounds, float epsilon);

bool intersects(Collider2D *a, Collider2D *b);

#endif //COLISION2D_GJK_H
