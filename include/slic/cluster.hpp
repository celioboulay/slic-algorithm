#pragma once

struct Cluster { // better than class for data
    float L, a, b; // CIELAB colors
    int x, y; // center coords
    int label;
    Cluster(int label_, float L_, float a_, float b_, int x_, int y_)
        : label(label_), L(L_), a(a_), b(b_), x(x_), y(y_) {}
};