#pragma once

#include <initializer_list>

struct vec2i {
    int x = 0;
    int y = 0;

    vec2i() {}
    
    vec2i(int _x, int _y) : x(_x), y(_y) {}

    inline vec2i operator-(vec2i rhs) const { return {x - rhs.x, y - rhs.y}; }
    inline vec2i operator-=(vec2i rhs) { *this = *this - rhs; return *this; }
    inline vec2i operator/(float v) const { return vec2i(this->x / v, this->y / v); }
    inline vec2i operator+(vec2i rhs) const { return {x + rhs.x, y + rhs.y}; }
    inline vec2i operator+=(vec2i rhs) { *this = *this + rhs; return *this; }
    inline bool operator==(vec2i rhs) const { return (x == rhs.x && y == rhs.y); }
    inline bool operator!=(vec2i rhs) const { return !(*this == rhs); }
};

struct rect {
    vec2i mn;
    vec2i mx;

    // Constructor
    rect(const vec2i& topL, const vec2i& bottomR) : mn(topL), mx(bottomR) {}

    inline int w() const { return mx.x - mn.x; }
    inline int h() const { return mx.y - mn.y; }
    inline bool inside(const vec2i& p) const { return (p.x >= mn.x && p.x <= mx.x && p.y >= mn.y && p.y <= mx.y); }
};
