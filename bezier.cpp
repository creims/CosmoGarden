#include <iostream>

#include "bezier.h"

using glm::vec3;

// Helpers
static inline float len(vec3 a, vec3 b) {
    return length(a - b);
}

static inline float totalLength(vec3 a, vec3 b, vec3 c, vec3 d) {
    return len(a, b) + len(b, c) + len(c, d);
}

static inline bool isFlatEnough(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float flatFactor) {
    return totalLength(p0, p1, p2, p3) < flatFactor * len(p0, p3);
}

Bezier::Bezier(vec3 _c0, vec3 _c1, vec3 _c2, vec3 _c3)
        : c0{_c0}, c1{_c1}, c2{_c2}, c3{_c3} {
    length = totalLength(c0, c1, c2, c3);
}

Bezier::Bezier(vec3 _c0, vec3 _c1, vec3 _c2, vec3 _c3, float _flatFactor)
        : c0{_c0}, c1{_c1}, c2{_c2}, c3{_c3}, flatFactor{_flatFactor} {
    length = totalLength(c0, c1, c2, c3);
}

void Bezier::setFlatFactor(float _flatFactor) {
    flatFactor = _flatFactor;
    vertsGenerated = false;
}

// Returns a bezier that is a subcurve from 0 to end
Bezier Bezier::subCurve(float end) {
    return subCurve(0.0f, end);
}

// Returns a bezier that is a subcurve from start to end
Bezier Bezier::subCurve(float start, float end) {
    vec3 q0 = mix(c0, c1, start);
    vec3 q1 = mix(c1, c2, start);
    vec3 q2 = mix(c2, c3, start);

    vec3 r0 = mix(q0, q1, start);
    vec3 r1 = mix(q1, q2, start);

    vec3 s = mix(r0, r1, start);

    // Bezier from start to end is now s r1 q2 c3
    vec3 q10 = mix(s, r1, end);
    vec3 q11 = mix(r1, q2, end);
    vec3 q12 = mix(q2, c3, end);

    vec3 r10 = mix(q10, q11, end);
    vec3 r11 = mix(q11, q12, end);

    vec3 s1 = mix(r10, r11, end);

    return Bezier{s, q10, r10, s1};
}

posAndDir Bezier::pointAt(float t) {
    posAndDir ret{};

    vec3 c10 = mix(c0, c1, t);
    vec3 c11 = mix(c1, c2, t);
    vec3 c12 = mix(c2, c3, t);

    vec3 c20 = mix(c10, c11, t);
    vec3 c21 = mix(c11, c12, t);

    vec3 p = mix(c20, c21, t);

    ret.position = p;
    ret.direction = normalize(c21 - p);

    // TODO: reconsider special cases where len(direction) == 0.0
    // Also applies to normals added during deCasteljau
    if(all(isnan(ret.direction))) {
        ret.direction = normalize(p - mix(c2, c3, 0.999f));
    }

    return ret;
}

void Bezier::subdivide() {
    verts.push_back(c0); // First control point is endpt
    dirs.push_back(normalize(c1 - c0));
    dists.push_back(0.0f);
    deCasteljau(c0, c1, c2, c3, 0.0f, 1.0f);
    vertsGenerated = true;
}

void Bezier::deCasteljau(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float startDist, float endDist) {
    endDist = std::min(endDist, 1.0f);
    if (isFlatEnough(p0, p1, p2, p3, flatFactor) && endDist - dists.back() < minDistance) {
        verts.push_back(p3);
        dirs.push_back(normalize(p3 - p2));
        dists.push_back(endDist);
    } else {
        vec3 q0 = mix(p0, p1, 0.5f);
        vec3 q1 = mix(p1, p2, 0.5f);
        vec3 q2 = mix(p2, p3, 0.5f);

        vec3 r0 = mix(q0, q1, 0.5f);
        vec3 r1 = mix(q1, q2, 0.5f);

        vec3 s0 = mix(r0, r1, 0.5f);

        // Early curve first to maintain order
        float halfDist = (endDist - startDist) * 0.5f + startDist;
        deCasteljau(p0, q0, r0, s0, startDist, halfDist);
        deCasteljau(s0, r1, q2, p3, halfDist, endDist);
    }
}

std::vector<vec3> Bezier::getVertices() {
    if(tooShort()) return verts;

    if(!vertsGenerated) subdivide();
    return verts;
}

std::vector<vec3> Bezier::getDirections() {
    if(tooShort()) return dirs;

    if(!vertsGenerated) subdivide();
    return dirs;
}

bool Bezier::tooShort() {
    return length < BEZIER_TOO_SHORT;
}

std::vector<float> Bezier::getDistances() {
    if(tooShort()) return dists;

    if(!vertsGenerated) subdivide();
    return dists;
}
