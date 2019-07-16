#ifndef BEZIER_H
#define BEZIER_H

#include <vector>

#include <glm/glm.hpp>

#define BEZIER_ROUGH 1.2f
#define BEZIER_UNEVEN 1.1f
#define BEZIER_FLAT 1.01f
#define BEZIER_FLATTER 1.001f
#define BEZIER_FLATTEST 1.0001f

#define BEZIER_TOO_SHORT 0.00001f

struct posAndDir {
    glm::vec3 position;
    glm::vec3 direction;
};

class Bezier {
public:
    Bezier() = default;
    Bezier(glm::vec3 _c0, glm::vec3 _c1, glm::vec3 _c2, glm::vec3 _c3);
    Bezier(glm::vec3 _c0, glm::vec3 _c1, glm::vec3 _c2, glm::vec3 _c3, float _flatFactor);

    void setFlatFactor(float _flatFactor);

    posAndDir pointAt(float t);
    float getLength() { return length; }
    bool tooShort();

    Bezier subCurve(float end);
    Bezier subCurve(float start, float end);

    std::vector<glm::vec3> getVertices();
    std::vector<glm::vec3> getDirections();
    std::vector<float> getDistances();

private:
    glm::vec3 c0{glm::vec3{0.0}}, c1{glm::vec3{0.0}}, c2{glm::vec3{0.0}}, c3{glm::vec3{0.0}};
    std::vector<glm::vec3> verts{}, dirs{};
    std::vector<float> dists{};
    float flatFactor{BEZIER_FLAT}, length{0}, minDistance{0.2f};
    bool vertsGenerated{false};

    void subdivide();
    void deCasteljau(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float startDist, float endDist);
};

#endif //BEZIER_H
