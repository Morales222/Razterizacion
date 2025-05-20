#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

struct Color {
    unsigned char r, g, b;
};

struct Point {
    int x, y;
};

struct Vertex {
    float x, y, z;
    Vertex() : x(0), y(0), z(0) {}
    Vertex(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

struct Triangle {
    int v[3];
    Color color;
};

// Canvas
const int Cw = 400;
const int Ch = 400;

// Viewport 
const float Vw = 2.0f;
const float viewportHeight = 2.0f;

// Camera distance
const float d = 1.0f;

// Translation vector for camera position or object
const Vertex T(-1.5f, 0.0f, 7.0f);

// The actual pixel buffer for the canvas
Color canvas[Cw * Ch];

//  canvas in white color
void Canvas() {
    Color white = {255, 255, 255};
    fill(canvas, canvas + Cw * Ch, white);
}

// Set pixel only if inside canvas bounds
void PutPixel(int x, int y, Color col) {
    if (x >= 0 && x < Cw && y >= 0 && y < Ch) {
        canvas[y * Cw + x] = col;
    }
}

// Convert viewport coordinates (float) to canvas pixel coordinates (int)
Point ViewportToCanvas(float x, float y) {
    int cx = int((x + Vw / 2) * (Cw / Vw));
    int cy = int((viewportHeight / 2 - y) * (Ch / viewportHeight)); // y axis flipped
    return {cx, cy};
}

// Project a 3D vertex into 2D canvas space with T and perspective projection
Point ProjectVertex(Vertex v) {
    Vertex vt(v.x + T.x, v.y + T.y, v.z + T.z);
    float px = vt.x * d / vt.z;
    float py = vt.y * d / vt.z;
    return ViewportToCanvas(px, py);
}

// Linear interpolation helper
vector<float> Interpolate(int i0, float d0, int i1, float d1) {
    vector<float> values;
    if (i0 == i1) {
        values.push_back(d0);
        return values;
    }
    float step = (d1 - d0) / (i1 - i0);
    float val = d0;
    for (int i = i0; i <= i1; i++) {
        values.push_back(val);
        val += step;
    }
    return values;
}

// Draw line between two points with the given color using interpolation
void DrawLine(Point p0, Point p1, Color col) {
    int x0 = p0.x, y0 = p0.y;
    int x1 = p1.x, y1 = p1.y;

    int dx = x1 - x0;
    int dy = y1 - y0;

    if (abs(dx) > abs(dy)) {
        if (x0 > x1) {
            swap(x0, x1);
            swap(y0, y1);
        }
        vector<float> ys = Interpolate(x0, y0, x1, y1);
        for (int x = x0; x <= x1; x++) {
            PutPixel(x, int(round(ys[x - x0])), col);
        }
    } else {
        if (y0 > y1) {
            swap(x0, x1);
            swap(y0, y1);
        }
        vector<float> xs = Interpolate(y0, x0, y1, x1);
        for (int y = y0; y <= y1; y++) {
            PutPixel(int(round(xs[y - y0])), y, col);
        }
    }
}

// Draw wireframe triangle connecting 3 points
void DrawWireframeTriangle(Point p0, Point p1, Point p2, Color col) {
    DrawLine(p0, p1, col);
    DrawLine(p1, p2, col);
    DrawLine(p2, p0, col);
}

// Render a triangle on canvas from vertices projected in 2D
void RenderTriangle(const Triangle& tri, const vector<Point>& projectedVertices) {
    DrawWireframeTriangle(projectedVertices[tri.v[0]], projectedVertices[tri.v[1]], projectedVertices[tri.v[2]], tri.color);
}

// Render all triangles of the object given its vertices and triangle list
void RenderObject(const vector<Vertex>& vertices, const vector<Triangle>& triangles) {
    vector<Point> projectedVerts;
    for (const Vertex& v : vertices) {
        projectedVerts.push_back(ProjectVertex(v));
    }
    for (const Triangle& tri : triangles) {
        RenderTriangle(tri, projectedVerts);
    }
}

// Save the canvas pixel buffer as a PPM image file
void SaveCanvasToPPM(const string& filename) {
    ofstream outFile(filename);
    outFile << "P3\n" << Cw << " " << Ch << "\n255\n";
    for (int i = 0; i < Cw * Ch; i++) {
        outFile << (int)canvas[i].r << " " << (int)canvas[i].g << " " << (int)canvas[i].b << "\n";
    }
    outFile.close();
}

int main() {
    Canvas();

    // Base cube vertices
    vector<Vertex> baseVertices = {
        { 1,  1,  1},  // 0
        {-1,  1,  1},  // 1
        {-1, -1,  1},  // 2
        { 1, -1,  1},  // 3
        { 1,  1, -1},  // 4
        {-1,  1, -1},  // 5
        {-1, -1, -1},  // 6
        { 1, -1, -1}   // 7
    };

    //  colors
    Color red    = {255, 0, 0};
    Color green  = {0, 255, 0};
    Color blue   = {0, 0, 255};
    Color yellow = {255, 255, 0};
    Color purple = {128, 0, 128};
    Color cyan   = {0, 255, 255};

    // Cube faces made of triangles
    vector<Triangle> triangles = {
        {{0, 1, 2}, red},
        {{0, 2, 3}, red},
        {{4, 0, 3}, green},
        {{4, 3, 7}, green},
        {{5, 4, 7}, blue},
        {{5, 7, 6}, blue},
        {{1, 5, 6}, yellow},
        {{1, 6, 2}, yellow},
        {{4, 5, 1}, purple},
        {{4, 1, 0}, purple},
        {{2, 6, 7}, cyan},
        {{2, 7, 3}, cyan}
    };

    // The base cube
    RenderObject(baseVertices, triangles);

    // The translated cube (shift +3 in X and +1 in Z)
    vector<Vertex> translatedVerts;
    for (const auto& v : baseVertices) {
        translatedVerts.push_back(Vertex(v.x + 3.0f, v.y, v.z + 1.0f));
    }
    RenderObject(translatedVerts, triangles);

    // The scaled cube (scale factor 1.5)
    vector<Vertex> scaledVerts;
    float scaleFactor = 1.5f;
    for (const auto& v : baseVertices) {
        scaledVerts.push_back(Vertex(v.x * scaleFactor, v.y * scaleFactor, v.z * scaleFactor));
    }
    RenderObject(scaledVerts, triangles);

    SaveCanvasToPPM("output.ppm");

    cout << "output.ppm\n";
    return 0;
}
