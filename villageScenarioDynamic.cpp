#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdio>

// ============================================================
// CONSTANTS AND CONFIGURATION
// ============================================================

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define PI 3.14159265358979323846
#define DEG_TO_RAD (PI / 180.0)
#define TIMER_INTERVAL 16

#define MAX_RAIN_DROPS 2000
#define MAX_RIPPLES 200
#define MAX_CLOUDS 15
#define MAX_BIRDS 12
#define MAX_LEAVES 50
#define MAX_FIREFLIES 80
#define MAX_STARS 200
#define MAX_PUDDLES 20
#define MAX_LIGHTNING 5
#define MAX_SMOKE_PARTICLES 100
#define MAX_FISH 8
#define MAX_BUTTERFLIES 10
#define MAX_DRAGONFLIES 6
#define MAX_WATER_PARTICLES 30

// ============================================================
// DATA STRUCTURES
// ============================================================

struct Color {
    float r, g, b, a;
    Color() : r(0), g(0), b(0), a(1) {}
    Color(float r, float g, float b) : r(r), g(g), b(b), a(1) {}
    Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
};

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    float length() const { return sqrt(x * x + y * y); }
};

struct RainDrop {
    float x, y, speedX, speedY, length, alpha;
    bool active;
};

struct Ripple {
    float x, y, radius, maxRadius, alpha;
    bool active;
};

struct Cloud {
    float x, y, width, height, speed, darkness, alpha;
    int type;
    bool active;
    // Enhanced: many more sub-puffs for volumetric realistic shape
    float puffOffsets[12][3]; // x, y, radius offsets for sub-puffs
    int numPuffs;
    float internalVariation[12]; // brightness variation per puff
    float edgeSoftness;
    float silverLiningIntensity;
};

struct Bird {
    float x, y, speed, wingAngle, wingSpeed, amplitude, phase, scale;
    int direction;
    bool active;
};

struct Leaf {
    float x, y, speedX, speedY, rotation, rotSpeed, scale;
    Color color;
    bool active;
};

struct Firefly {
    float x, y, speedX, speedY, brightness, brightnessDir, phase;
    bool active;
};

struct Star {
    float x, y, brightness, twinkleSpeed, phase, size;
    bool active;
};

struct Puddle {
    float x, y, width, height, ripplePhase;
    bool active;
};

struct Lightning {
    float x, brightness, duration, timer;
    bool active;
    std::vector<Vec2> segments;
};

struct SmokeParticle {
    float x, y, speedX, speedY, alpha, size, life;
    bool active;
};

struct Fish {
    float x, y, speed, tailAngle, tailSpeed, amplitude, phase, scale;
    Color color;
    int direction;
    bool active;
};

struct Butterfly {
    float x, y, targetX, targetY, wingAngle, wingSpeed, speed, phase;
    Color color1, color2;
    bool active;
};

struct Dragonfly {
    float x, y, speed, wingAngle, amplitude, phase, scale;
    bool active;
};

struct WaterParticle {
    float x, y, speedX, speedY, alpha, life;
    bool active;
};

struct Rickshaw {
    float x, y, speed, wheelAngle, bodyBounce, bouncePhase, pedalAngle;
    int direction;
    bool isMoving;
    Color bodyColor, canopyColor;
};

struct Boat {
    float x, y, speed, bobPhase, bobAmplitude, rockAngle, rockPhase, sailAngle;
    int type, direction;
    bool isMoving;
};

struct Person {
    float x, y, speed, walkPhase, scale;
    int type, direction;
    bool isWalking;
    Color shirtColor, pantsColor, skinColor, hairColor;
};

struct KiteData {
    float x, y, ownerX, ownerY, angle, swayPhase, height;
    Color color1, color2;
};

struct Chicken {
    float x, y, speed, peckPhase, walkPhase, peckTimer;
    int direction;
    bool isPecking, active;
};

// ============================================================
// GLOBAL STATE
// ============================================================

bool isRainyMode = false;
float modeTransition = 0.0f;
float transitionSpeed = 0.02f;
float globalTime = 0.0f;
float deltaTime = 0.016f;
int windowWidth = WINDOW_WIDTH;
int windowHeight = WINDOW_HEIGHT;
bool isFullscreen = true;

float worldLeft = 0.0f, worldRight = 1920.0f;
float worldBottom = 0.0f, worldTop = 1080.0f;

float sunX = 1400.0f, sunY = 500.0f, sunRadius = 60.0f;
float waterLevel = 300.0f, waterWavePhase = 0.0f;
float windStrength = 0.0f, windDirection = 1.0f, targetWindStrength = 0.3f;
float rainIntensity = 0.0f, targetRainIntensity = 0.0f;
float thunderTimer = 0.0f, thunderInterval = 8.0f;
bool screenFlash = false;
float screenFlashAlpha = 0.0f;
float ambientDarkness = 0.0f;
float palmSwayAngle = 0.0f;
float screenShakeX = 0.0f, screenShakeY = 0.0f; // lightning screen-shake

// Entities
RainDrop rainDrops[MAX_RAIN_DROPS];
Ripple ripples[MAX_RIPPLES];
Cloud clouds[MAX_CLOUDS];
Bird birds[MAX_BIRDS];
Leaf leaves[MAX_LEAVES];
Firefly fireflies[MAX_FIREFLIES];
Star stars[MAX_STARS];
Puddle puddles[MAX_PUDDLES];
Lightning lightnings[MAX_LIGHTNING];
SmokeParticle smokeParticles[MAX_SMOKE_PARTICLES];
Fish fishes[MAX_FISH];
Butterfly butterflies[MAX_BUTTERFLIES];
Dragonfly dragonflies[MAX_DRAGONFLIES];
WaterParticle waterParticles[MAX_WATER_PARTICLES];

Rickshaw rickshaw1, rickshaw2;
Boat boat1, boat2, boat3;
Person persons[10];
KiteData kite;
Chicken chickens[5];

// Sky colors
Color skyTopNormal(0.96f, 0.45f, 0.15f);
Color skyMidNormal(0.98f, 0.65f, 0.20f);
Color skyBottomNormal(0.99f, 0.85f, 0.35f);
Color skyTopRainy(0.08f, 0.08f, 0.12f);
Color skyMidRainy(0.12f, 0.12f, 0.18f);
Color skyBottomRainy(0.15f, 0.15f, 0.22f);

Color waterTopNormal(0.15f, 0.55f, 0.75f);
Color waterBottomNormal(0.05f, 0.30f, 0.55f);
Color waterTopRainy(0.06f, 0.15f, 0.22f);
Color waterBottomRainy(0.03f, 0.08f, 0.12f);

Color grassNormal(0.20f, 0.60f, 0.15f);
Color grassDarkNormal(0.15f, 0.50f, 0.10f);
Color grassRainy(0.08f, 0.25f, 0.06f);
Color grassDarkRainy(0.05f, 0.18f, 0.04f);

Color roadNormal(0.55f, 0.50f, 0.42f);
Color roadRainy(0.20f, 0.18f, 0.15f);

// ============================================================
// UTILITY FUNCTIONS
// ============================================================

float lerp(float a, float b, float t) { return a + (b - a) * t; }
Color lerpColor(Color a, Color b, float t) {
    return Color(lerp(a.r, b.r, t), lerp(a.g, b.g, t), lerp(a.b, b.b, t), lerp(a.a, b.a, t));
}
float clamp(float val, float mn, float mx) { return val < mn ? mn : (val > mx ? mx : val); }
float randomFloat(float mn, float mx) { return mn + (float)rand() / RAND_MAX * (mx - mn); }
int randomInt(int mn, int mx) { return mn + rand() % (mx - mn + 1); }
float smoothstep(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// ============================================================
// BRESENHAM'S LINE ALGORITHM - Draw pixels as small quads
// ============================================================

void bresenhamLine(float x0f, float y0f, float x1f, float y1f, float thickness = 1.0f) {
    int x0 = (int)round(x0f), y0 = (int)round(y0f);
    int x1 = (int)round(x1f), y1 = (int)round(y1f);
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    float halfT = thickness * 0.5f;

    glBegin(GL_QUADS);
    while (true) {
        glVertex2f(x0 - halfT, y0 - halfT);
        glVertex2f(x0 + halfT, y0 - halfT);
        glVertex2f(x0 + halfT, y0 + halfT);
        glVertex2f(x0 - halfT, y0 + halfT);

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
    glEnd();
}

// ============================================================
// DDA LINE ALGORITHM
// ============================================================

void ddaLine(float x0, float y0, float x1, float y1, float thickness = 1.0f) {
    float dx = x1 - x0, dy = y1 - y0;
    int steps = (int)(fmax(fabs(dx), fabs(dy)));
    if (steps == 0) steps = 1;
    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;
    float halfT = thickness * 0.5f;

    float x = x0, y = y0;
    glBegin(GL_QUADS);
    for (int i = 0; i <= steps; i++) {
        glVertex2f(x - halfT, y - halfT);
        glVertex2f(x + halfT, y - halfT);
        glVertex2f(x + halfT, y + halfT);
        glVertex2f(x - halfT, y + halfT);
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// ============================================================
// MIDPOINT CIRCLE ALGORITHM
// ============================================================

void midpointCirclePlotPoints(float cx, float cy, int x, int y, float ps) {
    float halfP = ps * 0.5f;
    float pts[][2] = {
        {cx + x, cy + y}, {cx - x, cy + y}, {cx + x, cy - y}, {cx - x, cy - y},
        {cx + y, cy + x}, {cx - y, cy + x}, {cx + y, cy - x}, {cx - y, cy - x}
    };
    glBegin(GL_QUADS);
    for (int i = 0; i < 8; i++) {
        glVertex2f(pts[i][0] - halfP, pts[i][1] - halfP);
        glVertex2f(pts[i][0] + halfP, pts[i][1] - halfP);
        glVertex2f(pts[i][0] + halfP, pts[i][1] + halfP);
        glVertex2f(pts[i][0] - halfP, pts[i][1] + halfP);
    }
    glEnd();
}

void midpointCircle(float cx, float cy, float radius, float pointSize = 1.5f) {
    int r = (int)round(radius);
    int x = 0, y = r;
    int d = 1 - r;
    midpointCirclePlotPoints(cx, cy, x, y, pointSize);
    while (x < y) {
        if (d < 0) {
            d += 2 * x + 3;
        }
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        midpointCirclePlotPoints(cx, cy, x, y, pointSize);
    }
}

// Filled midpoint circle using scanlines
void midpointCircleFilled(float cx, float cy, float radius) {
    int r = (int)round(radius);
    int x = 0, y = r;
    int d = 1 - r;

    auto drawScanLines = [&](int px, int py) {
        glBegin(GL_LINES);
        glVertex2f(cx - px, cy + py); glVertex2f(cx + px, cy + py);
        glVertex2f(cx - px, cy - py); glVertex2f(cx + px, cy - py);
        glVertex2f(cx - py, cy + px); glVertex2f(cx + py, cy + px);
        glVertex2f(cx - py, cy - px); glVertex2f(cx + py, cy - px);
        glEnd();
        };

    drawScanLines(x, y);
    while (x < y) {
        if (d < 0) d += 2 * x + 3;
        else { d += 2 * (x - y) + 5; y--; }
        x++;
        drawScanLines(x, y);
    }
}

// Midpoint ellipse algorithm for drawing ellipse outlines
void midpointEllipseOutline(float cx, float cy, float rx, float ry, float pointSize = 1.5f) {
    float rxSq = rx * rx, rySq = ry * ry;
    float x = 0, y = ry;
    float halfP = pointSize * 0.5f;

    // Region 1
    float d1 = rySq - rxSq * ry + 0.25f * rxSq;
    float dx = 2 * rySq * x, dy = 2 * rxSq * y;

    glBegin(GL_QUADS);
    while (dx < dy) {
        // Plot 4 symmetric points
        float pts[][2] = { {cx + x, cy + y}, {cx - x, cy + y}, {cx + x, cy - y}, {cx - x, cy - y} };
        for (int i = 0; i < 4; i++) {
            glVertex2f(pts[i][0] - halfP, pts[i][1] - halfP);
            glVertex2f(pts[i][0] + halfP, pts[i][1] - halfP);
            glVertex2f(pts[i][0] + halfP, pts[i][1] + halfP);
            glVertex2f(pts[i][0] - halfP, pts[i][1] + halfP);
        }
        if (d1 < 0) { x++; dx += 2 * rySq; d1 += dx + rySq; }
        else { x++; y--; dx += 2 * rySq; dy -= 2 * rxSq; d1 += dx - dy + rySq; }
    }

    // Region 2
    float d2 = rySq * (x + 0.5f) * (x + 0.5f) + rxSq * (y - 1) * (y - 1) - rxSq * rySq;
    while (y >= 0) {
        float pts[][2] = { {cx + x, cy + y}, {cx - x, cy + y}, {cx + x, cy - y}, {cx - x, cy - y} };
        for (int i = 0; i < 4; i++) {
            glVertex2f(pts[i][0] - halfP, pts[i][1] - halfP);
            glVertex2f(pts[i][0] + halfP, pts[i][1] - halfP);
            glVertex2f(pts[i][0] + halfP, pts[i][1] + halfP);
            glVertex2f(pts[i][0] - halfP, pts[i][1] + halfP);
        }
        if (d2 > 0) { y--; dy -= 2 * rxSq; d2 += rxSq - dy; }
        else { x++; y--; dx += 2 * rySq; dy -= 2 * rxSq; d2 += dx - dy + rxSq; }
    }
    glEnd();
}

// ============================================================
// BASIC GL DRAWING PRIMITIVES
// ============================================================

void setColor(Color c) { glColor4f(c.r, c.g, c.b, c.a); }
void setColor(float r, float g, float b, float a = 1.0f) { glColor4f(r, g, b, a); }

void drawCircle(float cx, float cy, float r, int segments = 36) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * i / segments;
        glVertex2f(cx + cos(angle) * r, cy + sin(angle) * r);
    }
    glEnd();
}

void drawCircleOutline(float cx, float cy, float r, int segments = 36) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * PI * i / segments;
        glVertex2f(cx + cos(angle) * r, cy + sin(angle) * r);
    }
    glEnd();
}

void drawEllipse(float cx, float cy, float rx, float ry, int segments = 36) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * i / segments;
        glVertex2f(cx + cos(angle) * rx, cy + sin(angle) * ry);
    }
    glEnd();
}

void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + w, y);
    glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1); glVertex2f(x2, y2); glVertex2f(x3, y3);
    glEnd();
}

void drawQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1); glVertex2f(x2, y2); glVertex2f(x3, y3); glVertex2f(x4, y4);
    glEnd();
}

void drawLine(float x1, float y1, float x2, float y2, float width = 1.0f) {
    glLineWidth(width);
    glBegin(GL_LINES); glVertex2f(x1, y1); glVertex2f(x2, y2); glEnd();
    glLineWidth(1.0f);
}

void drawFilledArc(float cx, float cy, float r, float startAngle, float endAngle, int seg = 20) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; i++) {
        float angle = startAngle + (endAngle - startAngle) * i / seg;
        glVertex2f(cx + cos(angle) * r, cy + sin(angle) * r);
    }
    glEnd();
}

void drawGradientRect(float x, float y, float w, float h, Color bottom, Color top) {
    glBegin(GL_QUADS);
    glColor4f(bottom.r, bottom.g, bottom.b, bottom.a);
    glVertex2f(x, y); glVertex2f(x + w, y);
    glColor4f(top.r, top.g, top.b, top.a);
    glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

void drawThickLine(float x1, float y1, float x2, float y2, float thickness) {
    float dx = x2 - x1, dy = y2 - y1;
    float len = sqrt(dx * dx + dy * dy);
    if (len < 0.001f) return;
    float nx = -dy / len * thickness * 0.5f;
    float ny = dx / len * thickness * 0.5f;
    glBegin(GL_QUADS);
    glVertex2f(x1 + nx, y1 + ny); glVertex2f(x1 - nx, y1 - ny);
    glVertex2f(x2 - nx, y2 - ny); glVertex2f(x2 + nx, y2 + ny);
    glEnd();
}

void drawRoundedRect(float x, float y, float w, float h, float radius, int seg = 8) {
    drawRect(x + radius, y, w - 2 * radius, h);
    drawRect(x, y + radius, radius, h - 2 * radius);
    drawRect(x + w - radius, y + radius, radius, h - 2 * radius);
    drawFilledArc(x + radius, y + radius, radius, PI, 1.5f * PI, seg);
    drawFilledArc(x + w - radius, y + radius, radius, 1.5f * PI, 2.0f * PI, seg);
    drawFilledArc(x + w - radius, y + h - radius, radius, 0, 0.5f * PI, seg);
    drawFilledArc(x + radius, y + h - radius, radius, 0.5f * PI, PI, seg);
}

void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    while (*text) { glutBitmapCharacter(font, *text); text++; }
}

// ============================================================
// 2D TRANSFORMATIONS
// ============================================================

// ---------- Shear matrix ----------
// Multiplies the current matrix by a shear:
//   x' = x + shx*y   y' = y + shy*x
// (OpenGL expects column-major layout)
void applyShearMatrix(float shx, float shy = 0.0f) {
    float m[16] = {
        1.0f, shy,  0.0f, 0.0f,   // col 0
        shx,  1.0f, 0.0f, 0.0f,   // col 1
        0.0f, 0.0f, 1.0f, 0.0f,   // col 2
        0.0f, 0.0f, 0.0f, 1.0f    // col 3
    };
    glMultMatrixf(m);
}

// ---------- Shadow projection via shear ----------
// Returns how many units the shadow tip shifts horizontally
// per unit of object height, based on the current sun position.
float getShadowShearFactor(float objX) {
    float vis = 1.0f - modeTransition;
    if (vis < 0.05f) return 0.0f;
    float sunAlt = fmax(sunY - 480.0f, 12.0f); // sun height above ground plane
    float shx    = (objX - sunX) / sunAlt;      // negative = sun is to the right
    return clamp(shx, -2.5f, 2.5f) * vis;
}

// Draw a soft elliptical shadow on the ground.
// The ellipse is stretched in the direction the sun casts shadows.
void drawGroundShadow(float objX, float groundY,
                      float objW, float objH, float alpha = 0.22f) {
    float vis = 1.0f - modeTransition;
    if (vis < 0.05f) return;
    float shx  = getShadowShearFactor(objX);
    float tipX = objX + shx * objH;                  // where top of object casts shadow
    float cx   = (objX + tipX) * 0.5f;               // shadow ellipse centre X
    float hLen = fabs(tipX - objX) * 0.5f + objW * 0.35f;
    float hWid = objW * 0.18f + 2.5f;
    setColor(0.02f, 0.02f, 0.02f, alpha * vis);
    drawEllipse(cx, groundY + 2.0f, hLen, hWid, 14);
}

void drawAllShadows() {
    float vis = 1.0f - modeTransition;
    if (vis < 0.05f) return;

    // -- House shadows --
    drawGroundShadow(130.0f,  520.0f, 100.0f, 110.0f);
    drawGroundShadow(315.0f,  530.0f, 143.0f, 140.0f);
    drawGroundShadow(472.0f,  540.0f,  72.0f,  95.0f);
    drawGroundShadow(1605.0f, 500.0f, 105.0f, 120.0f);
    drawGroundShadow(1748.0f, 490.0f,  68.0f,  95.0f);

    // -- Palm tree shadows --
    drawGroundShadow( 120.0f, 540.0f, 38.0f, 200.0f, 0.15f);
    drawGroundShadow( 300.0f, 560.0f, 42.0f, 220.0f, 0.15f);
    drawGroundShadow( 480.0f, 555.0f, 34.0f, 180.0f, 0.15f);
    drawGroundShadow( 650.0f, 550.0f, 32.0f, 190.0f, 0.15f);
    drawGroundShadow(1500.0f, 510.0f, 38.0f, 210.0f, 0.15f);
    drawGroundShadow(1700.0f, 500.0f, 30.0f, 170.0f, 0.15f);
    drawGroundShadow(1850.0f, 510.0f, 36.0f, 200.0f, 0.15f);

    // -- Banyan tree shadows --
    drawGroundShadow( 130.0f, 535.0f,  80.0f, 220.0f, 0.18f);
    drawGroundShadow( 320.0f, 548.0f,  92.0f, 253.0f, 0.18f);
    drawGroundShadow( 470.0f, 548.0f,  68.0f, 190.0f, 0.18f);
    drawGroundShadow(1600.0f, 510.0f,  84.0f, 230.0f, 0.18f);
    drawGroundShadow(1750.0f, 498.0f,  72.0f, 200.0f, 0.18f);

    // -- Person shadows --
    for (int i = 0; i < 10; i++) {
        if (persons[i].x < -500.0f) continue;
        float h = (persons[i].type == 2) ? 44.0f : 75.0f;
        drawGroundShadow(persons[i].x, persons[i].y,
                         14.0f * persons[i].scale, h * persons[i].scale, 0.18f);
    }
}

// ---------- Water scene reflection ----------
// Uses a Y-flip scale transform (y' = 2*waterLevel - y) plus an
// oscillating horizontal shear to simulate wave distortion.
void drawWaterSceneReflection() {
    float reflAlpha = 0.22f * (1.0f - modeTransition * 0.55f);
    if (reflAlpha < 0.02f) return;

    glPushMatrix();

    // Core 2D reflection transform: y' = 2*waterLevel - y
    glTranslatef(0.0f, 2.0f * waterLevel, 0.0f);
    glScalef(1.0f, -1.0f, 1.0f);

    // Wave distortion: oscillating horizontal shear
    float waveShear = sin(waterWavePhase * 0.9f)  * 0.018f
                    + sin(waterWavePhase * 1.6f + 1.2f) * 0.009f;
    applyShearMatrix(waveShear, 0.0f);

    Color wt = lerpColor(waterTopNormal, waterTopRainy, modeTransition);

    // Reflected grass/ground strip (world y=480 -> screen y'=120 in water)
    setColor(0.14f, wt.g * 0.60f, 0.09f, reflAlpha * 0.65f);
    drawRect(0, 480.0f, worldRight, 30.0f);

    // Reflected distant tree canopies (world y~510 -> screen y'~90)
    for (int i = 0; i < 10; i++) {
        float treeX = 50.0f + i * 180.0f;
        setColor(0.09f, wt.g * 0.48f, 0.06f, reflAlpha * 0.45f);
        drawEllipse(treeX, 510.0f, 20.0f, 14.0f, 8);
    }

    // Reflected house silhouettes (world y=490-560 -> screen y'=40-110)
    float rHX[] = {80,250,430,1550,1700};
    float rHY[] = {520,530,540,500,490};
    float rHW[] = {100,143,72,105,68};
    for (int i = 0; i < 5; i++) {
        setColor(wt.r*0.50f, wt.g*0.38f, wt.b*0.28f, reflAlpha * 0.32f);
        drawRect(rHX[i], rHY[i], rHW[i], 75.0f);
    }

    // Reflected tree trunk bases (world y=498-560 -> screen y'=40-102)
    float rTX[] = {130,320,470,1600,1750,120,300,480,650,1500};
    float rTY[] = {535,548,548,510,498,540,560,555,550,510};
    for (int i = 0; i < 10; i++) {
        setColor(0.22f, wt.g * 0.28f, 0.08f, reflAlpha * 0.28f);
        drawRect(rTX[i]-5.0f, rTY[i], 10.0f, 45.0f);
    }

    // Sun reflection column (sunset mode)
    if (modeTransition < 0.4f) {
        float sunVis = clamp(1.0f - modeTransition * 2.5f, 0.0f, 1.0f);
        for (int i = 0; i < 6; i++) {
            float colY   = waterLevel + 5.0f + i * 28.0f; // screen y'=295,267,...
            float colW   = 10.0f + i * 6.0f;
            float colOff = sin(waterWavePhase + i * 0.7f) * 5.0f;
            setColor(1.0f, 0.75f, 0.25f,
                     reflAlpha * sunVis * (0.60f - i * 0.08f));
            drawRect(sunX - colW * 0.5f + colOff, colY, colW, 22.0f);
        }
    }

    glPopMatrix();
}

// ---------- Screen shake (translation) ----------
// Applies a damped random translation on lightning strikes.
void updateScreenShake() {
    if (screenFlash && screenFlashAlpha > 0.15f) {
        screenShakeX = randomFloat(-5.5f, 5.5f) * screenFlashAlpha;
        screenShakeY = randomFloat(-3.0f, 3.0f) * screenFlashAlpha;
    } else {
        screenShakeX *= 0.78f;
        screenShakeY *= 0.78f;
        if (fabs(screenShakeX) < 0.15f) screenShakeX = 0.0f;
        if (fabs(screenShakeY) < 0.15f) screenShakeY = 0.0f;
    }
}

// ---------- Depth-based perspective scale ----------
// Objects further left on the road appear slightly smaller,
// simulating a gentle vanishing-point perspective.
float getRoadDepthScale(float worldX) {
    float t = clamp(worldX / worldRight, 0.0f, 1.0f);
    return lerp(0.88f, 1.04f, t);
}

// ============================================================
// INITIALIZATION
// ============================================================

void initClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        clouds[i].active = true;
        clouds[i].x = randomFloat(-200, worldRight + 200);
        clouds[i].y = randomFloat(720, worldTop - 50);
        clouds[i].width = randomFloat(180, 400);
        clouds[i].height = randomFloat(50, 120);
        clouds[i].speed = randomFloat(0.15f, 0.6f);
        clouds[i].darkness = 0.0f;
        clouds[i].alpha = randomFloat(0.6f, 0.95f);
        clouds[i].type = randomInt(0, 3);
        clouds[i].edgeSoftness = randomFloat(0.6f, 1.0f);
        clouds[i].silverLiningIntensity = randomFloat(0.3f, 0.8f);
        // Generate sub-puffs for realistic volumetric cloud shape
        clouds[i].numPuffs = randomInt(8, 12);
        for (int j = 0; j < clouds[i].numPuffs; j++) {
            clouds[i].puffOffsets[j][0] = randomFloat(-0.45f, 0.45f); // relative x
            clouds[i].puffOffsets[j][1] = randomFloat(-0.2f, 0.5f);   // relative y
            clouds[i].puffOffsets[j][2] = randomFloat(0.15f, 0.5f);   // relative radius
            clouds[i].internalVariation[j] = randomFloat(-0.08f, 0.08f);
        }
    }
}

void initAll() {
    srand((unsigned)time(NULL));

    // Rain drops
    for (int i = 0; i < MAX_RAIN_DROPS; i++) {
        rainDrops[i].active = false;
        rainDrops[i].x = randomFloat(0, worldRight);
        rainDrops[i].y = randomFloat(0, worldTop);
        rainDrops[i].speedX = randomFloat(-2.0f, -1.0f);
        rainDrops[i].speedY = randomFloat(-15.0f, -8.0f);
        rainDrops[i].length = randomFloat(10.0f, 25.0f);
        rainDrops[i].alpha = randomFloat(0.3f, 0.7f);
    }
    for (int i = 0; i < MAX_RIPPLES; i++) ripples[i].active = false;

    initClouds();

    // Birds
    for (int i = 0; i < MAX_BIRDS; i++) {
        birds[i].active = true;
        birds[i].x = randomFloat(-100, worldRight + 100);
        birds[i].y = randomFloat(650, worldTop - 100);
        birds[i].speed = randomFloat(1.5f, 4.0f);
        birds[i].wingAngle = 0;
        birds[i].wingSpeed = randomFloat(3.0f, 6.0f);
        birds[i].amplitude = randomFloat(5.0f, 20.0f);
        birds[i].phase = randomFloat(0, 2 * PI);
        birds[i].scale = randomFloat(0.6f, 1.2f);
        birds[i].direction = (i < 8) ? 1 : ((rand() % 2 == 0) ? 1 : -1);
    }

    for (int i = 0; i < MAX_LEAVES; i++) leaves[i].active = false;

    // Fireflies
    for (int i = 0; i < MAX_FIREFLIES; i++) {
        fireflies[i].active = false;
        fireflies[i].x = randomFloat(50, worldRight - 50);
        fireflies[i].y = randomFloat(320, 700);
        fireflies[i].speedX = randomFloat(-0.5f, 0.5f);
        fireflies[i].speedY = randomFloat(-0.3f, 0.3f);
        fireflies[i].brightness = randomFloat(0, 1);
        fireflies[i].brightnessDir = randomFloat(0.01f, 0.03f);
        fireflies[i].phase = randomFloat(0, 2 * PI);
    }

    // Stars
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].active = true;
        stars[i].x = randomFloat(0, worldRight);
        stars[i].y = randomFloat(600, worldTop);
        stars[i].brightness = randomFloat(0.1f, 0.6f);
        stars[i].twinkleSpeed = randomFloat(1.0f, 4.0f);
        stars[i].phase = randomFloat(0, 2 * PI);
        stars[i].size = randomFloat(1.0f, 3.0f);
    }

    for (int i = 0; i < MAX_PUDDLES; i++) {
        puddles[i].active = false;
        puddles[i].x = randomFloat(100, worldRight - 100);
        puddles[i].y = randomFloat(320, 480);
        puddles[i].width = randomFloat(30, 80);
        puddles[i].height = randomFloat(8, 20);
        puddles[i].ripplePhase = randomFloat(0, 2 * PI);
    }

    for (int i = 0; i < MAX_LIGHTNING; i++) lightnings[i].active = false;
    for (int i = 0; i < MAX_SMOKE_PARTICLES; i++) smokeParticles[i].active = false;

    // Fish
    for (int i = 0; i < MAX_FISH; i++) {
        fishes[i].active = true;
        fishes[i].x = randomFloat(100, worldRight - 100);
        fishes[i].y = randomFloat(80, waterLevel - 50);
        fishes[i].speed = randomFloat(0.5f, 2.0f);
        fishes[i].tailAngle = 0; fishes[i].tailSpeed = randomFloat(3, 6);
        fishes[i].amplitude = randomFloat(3, 10); fishes[i].phase = randomFloat(0, 2 * PI);
        fishes[i].scale = randomFloat(0.5f, 1.5f);
        fishes[i].direction = (rand() % 2 == 0) ? 1 : -1;
        fishes[i].color = Color(randomFloat(0.6f, 1), randomFloat(0.3f, 0.7f), randomFloat(0.1f, 0.4f));
    }

    // Butterflies
    for (int i = 0; i < MAX_BUTTERFLIES; i++) {
        butterflies[i].active = true;
        butterflies[i].x = randomFloat(100, worldRight - 100);
        butterflies[i].y = randomFloat(400, 700);
        butterflies[i].targetX = randomFloat(100, worldRight - 100);
        butterflies[i].targetY = randomFloat(400, 700);
        butterflies[i].wingAngle = 0; butterflies[i].wingSpeed = randomFloat(5, 10);
        butterflies[i].speed = randomFloat(0.5f, 1.5f); butterflies[i].phase = randomFloat(0, 2 * PI);
        float hue = randomFloat(0, 1);
        butterflies[i].color1 = Color(0.5f + 0.5f * sin(hue * 6.28f), 0.5f + 0.5f * sin(hue * 6.28f + 2.09f), 0.5f + 0.5f * sin(hue * 6.28f + 4.18f));
        butterflies[i].color2 = Color(0.5f + 0.5f * sin(hue * 6.28f + 1), 0.5f + 0.5f * sin(hue * 6.28f + 3.09f), 0.5f + 0.5f * sin(hue * 6.28f + 5.18f));
    }

    // Dragonflies
    for (int i = 0; i < MAX_DRAGONFLIES; i++) {
        dragonflies[i].active = true;
        dragonflies[i].x = randomFloat(200, worldRight - 200);
        dragonflies[i].y = randomFloat(350, 600);
        dragonflies[i].speed = randomFloat(1, 3); dragonflies[i].wingAngle = 0;
        dragonflies[i].amplitude = randomFloat(20, 60); dragonflies[i].phase = randomFloat(0, 2 * PI);
        dragonflies[i].scale = randomFloat(0.8f, 1.5f);
    }

    for (int i = 0; i < MAX_WATER_PARTICLES; i++) waterParticles[i].active = false;

    // Rickshaw 1
    rickshaw1.x = 300; rickshaw1.y = 420; rickshaw1.speed = 1.2f;
    rickshaw1.wheelAngle = 0; rickshaw1.bodyBounce = 0; rickshaw1.bouncePhase = 0;
    rickshaw1.direction = -1; rickshaw1.isMoving = true;
    rickshaw1.bodyColor = Color(0.95f, 0.7f, 0.1f);
    rickshaw1.canopyColor = Color(0.9f, 0.2f, 0.1f);
    rickshaw1.pedalAngle = 0;

    // Rickshaw 2
    rickshaw2.x = 900; rickshaw2.y = 440; rickshaw2.speed = 0.8f;
    rickshaw2.wheelAngle = 0; rickshaw2.bodyBounce = 0; rickshaw2.bouncePhase = 0;
    rickshaw2.direction = 1; rickshaw2.isMoving = true;
    rickshaw2.bodyColor = Color(0.2f, 0.6f, 0.9f);
    rickshaw2.canopyColor = Color(0.9f, 0.4f, 0.1f);
    rickshaw2.pedalAngle = 0;

    // Boats
    boat1.x = 1000; boat1.y = 200; boat1.speed = 0.3f;
    boat1.bobPhase = 0; boat1.bobAmplitude = 3; boat1.rockAngle = 0;
    boat1.rockPhase = 0; boat1.type = 0; boat1.direction = -1; boat1.sailAngle = 0; boat1.isMoving = true;

    boat2.x = 1300; boat2.y = 170; boat2.speed = 0.5f;
    boat2.bobPhase = 1.5f; boat2.bobAmplitude = 4; boat2.rockAngle = 0;
    boat2.rockPhase = 0.5f; boat2.type = 1; boat2.direction = -1; boat2.sailAngle = 0; boat2.isMoving = true;

    boat3.x = 700; boat3.y = 150; boat3.speed = 0.2f;
    boat3.bobPhase = 3; boat3.bobAmplitude = 2.5f; boat3.rockAngle = 0;
    boat3.rockPhase = 1; boat3.type = 2; boat3.direction = 1; boat3.sailAngle = 0; boat3.isMoving = true;

    // Persons with detailed features
    Color skinTones[] = {
        Color(0.72f, 0.55f, 0.38f), Color(0.65f, 0.48f, 0.32f),
        Color(0.58f, 0.42f, 0.28f), Color(0.78f, 0.60f, 0.42f)
    };

    // Woman in purple
    persons[0] = { 1450, 430, 0, 0, 1.0f, 1, -1, false,
        Color(0.6f,0.2f,0.7f), Color(0.6f,0.2f,0.7f), skinTones[0], Color(0.05f,0.03f,0.02f) };
    // Man
    persons[1] = { 1490, 430, 0, 0, 1.0f, 0, -1, false,
        Color(0.2f,0.5f,0.8f), Color(0.3f,0.3f,0.35f), skinTones[1], Color(0.08f,0.06f,0.04f) };
    // Child 1
    persons[2] = { 1530, 420, 0, 0, 0.7f, 2, -1, false,
        Color(0.9f,0.8f,0.2f), Color(0.3f,0.5f,0.3f), skinTones[2], Color(0.06f,0.04f,0.02f) };
    // Child 2
    persons[3] = { 1560, 420, 0, 0, 0.65f, 2, -1, false,
        Color(0.8f,0.3f,0.2f), Color(0.2f,0.3f,0.6f), skinTones[3], Color(0.08f,0.05f,0.03f) };
    // Woman in headscarf
    persons[4] = { 1420, 430, 0, 0, 1.0f, 1, 1, false,
        Color(0.2f,0.7f,0.5f), Color(0.2f,0.7f,0.5f), skinTones[0], Color(0.05f,0.03f,0.02f) };
    // Man 2
    persons[5] = { 1580, 430, 0, 0, 0.9f, 0, -1, false,
        Color(0.85f,0.65f,0.2f), Color(0.4f,0.3f,0.2f), skinTones[2], Color(0.07f,0.05f,0.03f) };
    // Kite boy
    persons[6] = { 680, 365, 0, 0, 0.7f, 2, 1, false,
        Color(0.2f,0.6f,0.9f), Color(0.7f,0.3f,0.1f), skinTones[1], Color(0.06f,0.04f,0.02f) };
    for (int i = 7; i < 10; i++) { persons[i].x = -1000; persons[i].isWalking = false; }

    // Kite
    kite.ownerX = 680; kite.ownerY = 400;
    kite.x = 750; kite.y = 550; kite.angle = 0; kite.swayPhase = 0; kite.height = 150;
    kite.color1 = Color(0.2f, 0.8f, 0.9f); kite.color2 = Color(0.9f, 0.8f, 0.1f);

    // Chickens
    for (int i = 0; i < 5; i++) {
        chickens[i].active = true;
        chickens[i].x = randomFloat(500, 750); chickens[i].y = randomFloat(450, 480);
        chickens[i].speed = randomFloat(0.1f, 0.5f); chickens[i].peckPhase = 0;
        chickens[i].walkPhase = randomFloat(0, 2 * PI);
        chickens[i].direction = (rand() % 2 == 0) ? 1 : -1;
        chickens[i].isPecking = (rand() % 2 == 0);
        chickens[i].peckTimer = randomFloat(0, 3);
    }
}

// ============================================================
// UPDATE FUNCTIONS
// ============================================================

void updateModeTransition() {
    if (isRainyMode) {
        modeTransition = clamp(modeTransition + transitionSpeed, 0, 1);
        targetRainIntensity = 1; targetWindStrength = 2;
    }
    else {
        modeTransition = clamp(modeTransition - transitionSpeed, 0, 1);
        targetRainIntensity = 0; targetWindStrength = 0.3f;
    }
    rainIntensity = lerp(rainIntensity, targetRainIntensity, 0.02f);
    windStrength = lerp(windStrength, targetWindStrength, 0.01f);
    ambientDarkness = modeTransition * 0.4f;
}

void updateRainDrops() {
    float windEffect = windStrength * windDirection * 3.0f;
    for (int i = 0; i < MAX_RAIN_DROPS; i++) {
        if (rainIntensity < 0.05f) { rainDrops[i].active = false; continue; }
        if (!rainDrops[i].active) {
            if (randomFloat(0, 1) < rainIntensity * 0.3f) {
                rainDrops[i].active = true;
                rainDrops[i].x = randomFloat(-50, worldRight + 50);
                rainDrops[i].y = worldTop + randomFloat(0, 100);
                rainDrops[i].speedX = randomFloat(-3, -1) + windEffect;
                rainDrops[i].speedY = randomFloat(-18, -10);
                rainDrops[i].length = randomFloat(10, 30) * rainIntensity;
                rainDrops[i].alpha = randomFloat(0.2f, 0.6f) * rainIntensity;
            }
            continue;
        }
        rainDrops[i].x += rainDrops[i].speedX;
        rainDrops[i].y += rainDrops[i].speedY;

        if (rainDrops[i].y < waterLevel && rainDrops[i].y > waterLevel - 25) {
            for (int j = 0; j < MAX_RIPPLES; j++) {
                if (!ripples[j].active) {
                    ripples[j].active = true; ripples[j].x = rainDrops[i].x;
                    ripples[j].y = rainDrops[i].y; ripples[j].radius = 1;
                    ripples[j].maxRadius = randomFloat(8, 20); ripples[j].alpha = 0.7f;
                    break;
                }
            }
            rainDrops[i].active = false;
        }
        if (rainDrops[i].y < -50 || rainDrops[i].x < -100 || rainDrops[i].x > worldRight + 100)
            rainDrops[i].active = false;
    }
}

void updateRipples() {
    for (int i = 0; i < MAX_RIPPLES; i++) {
        if (!ripples[i].active) continue;
        ripples[i].radius += 0.5f; ripples[i].alpha -= 0.02f;
        if (ripples[i].alpha <= 0 || ripples[i].radius >= ripples[i].maxRadius) ripples[i].active = false;
    }
}

void updateClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (!clouds[i].active) continue;
        clouds[i].x += clouds[i].speed * windDirection + windStrength * 0.5f;
        clouds[i].darkness = lerp(clouds[i].darkness, modeTransition * 0.7f, 0.01f);
        if (clouds[i].x > worldRight + 400) { clouds[i].x = -clouds[i].width - 50; clouds[i].y = randomFloat(720, worldTop - 50); }
        if (clouds[i].x < -clouds[i].width - 400) clouds[i].x = worldRight + 50;
    }
}

void updateBirds() {
    for (int i = 0; i < MAX_BIRDS; i++) {
        if (!birds[i].active) continue;
        if (isRainyMode && modeTransition > 0.5f) birds[i].speed = lerp(birds[i].speed, 6.0f, 0.01f);
        birds[i].x += birds[i].speed * birds[i].direction;
        birds[i].y += sin(globalTime * 2 + birds[i].phase) * 0.3f;
        birds[i].wingAngle = sin(globalTime * birds[i].wingSpeed + birds[i].phase) * 30;
        if (birds[i].direction == 1 && birds[i].x > worldRight + 100) { birds[i].x = -50; birds[i].y = randomFloat(650, worldTop - 100); }
        if (birds[i].direction == -1 && birds[i].x < -100) { birds[i].x = worldRight + 50; birds[i].y = randomFloat(650, worldTop - 100); }
    }
}

void updateLeaves() {
    if (windStrength > 0.5f) {
        for (int i = 0; i < MAX_LEAVES; i++) {
            if (!leaves[i].active && randomFloat(0, 1) < 0.02f * windStrength) {
                leaves[i].active = true;
                leaves[i].x = randomFloat(0, worldRight); leaves[i].y = randomFloat(500, 800);
                leaves[i].speedX = windStrength * windDirection * randomFloat(1, 3);
                leaves[i].speedY = randomFloat(-1, 0.5f);
                leaves[i].rotation = randomFloat(0, 360); leaves[i].rotSpeed = randomFloat(-5, 5);
                leaves[i].scale = randomFloat(0.5f, 1.5f);
                leaves[i].color = Color(randomFloat(0.2f, 0.5f), randomFloat(0.5f, 0.8f), randomFloat(0.1f, 0.3f));
                break;
            }
        }
    }
    for (int i = 0; i < MAX_LEAVES; i++) {
        if (!leaves[i].active) continue;
        leaves[i].x += leaves[i].speedX; leaves[i].y += leaves[i].speedY + sin(globalTime * 3 + i) * 0.5f;
        leaves[i].rotation += leaves[i].rotSpeed; leaves[i].speedY -= 0.01f;
        if (leaves[i].y < 0 || leaves[i].x < -50 || leaves[i].x > worldRight + 50) leaves[i].active = false;
    }
}

void updateFireflies() {
    float activity = 1.0f - modeTransition;
    for (int i = 0; i < MAX_FIREFLIES; i++) {
        if (!fireflies[i].active && activity > 0.3f && randomFloat(0, 1) < 0.005f) {
            fireflies[i].active = true; fireflies[i].x = randomFloat(50, worldRight - 50); fireflies[i].y = randomFloat(320, 600);
        }
        if (!fireflies[i].active) continue;
        if (activity < 0.2f) { fireflies[i].active = false; continue; }
        fireflies[i].x += fireflies[i].speedX + sin(globalTime * 0.5f + fireflies[i].phase) * 0.3f;
        fireflies[i].y += fireflies[i].speedY + cos(globalTime * 0.7f + fireflies[i].phase) * 0.2f;
        fireflies[i].brightness += fireflies[i].brightnessDir;
        if (fireflies[i].brightness > 1 || fireflies[i].brightness < 0) fireflies[i].brightnessDir = -fireflies[i].brightnessDir;
        if (fireflies[i].x < 50 || fireflies[i].x > worldRight - 50) fireflies[i].speedX = -fireflies[i].speedX;
        if (fireflies[i].y < 320 || fireflies[i].y > 600) fireflies[i].speedY = -fireflies[i].speedY;
    }
}

void updateStars() {
    for (int i = 0; i < MAX_STARS; i++) {
        if (!stars[i].active) continue;
        stars[i].brightness = 0.3f + 0.3f * sin(globalTime * stars[i].twinkleSpeed + stars[i].phase);
        stars[i].brightness *= modeTransition;
    }
}

void updatePuddles() {
    if (modeTransition > 0.3f) {
        for (int i = 0; i < MAX_PUDDLES; i++) {
            if (!puddles[i].active && randomFloat(0, 1) < 0.01f * modeTransition) {
                puddles[i].active = true; puddles[i].x = randomFloat(200, worldRight - 200);
                puddles[i].y = randomFloat(350, 480); puddles[i].width = randomFloat(20, 60); puddles[i].height = randomFloat(5, 15);
            }
            if (puddles[i].active) {
                puddles[i].ripplePhase += 0.1f;
                if (modeTransition > 0.5f) puddles[i].width = clamp(puddles[i].width + 0.01f, 0, 80);
            }
        }
    }
    else {
        for (int i = 0; i < MAX_PUDDLES; i++) {
            if (puddles[i].active) { puddles[i].width -= 0.05f; if (puddles[i].width < 5) puddles[i].active = false; }
        }
    }
}

void updateLightning() {
    if (modeTransition > 0.7f) {
        thunderTimer += deltaTime;
        if (thunderTimer > thunderInterval) {
            thunderTimer = 0; thunderInterval = randomFloat(5, 15);
            for (int i = 0; i < MAX_LIGHTNING; i++) {
                if (!lightnings[i].active) {
                    lightnings[i].active = true;
                    lightnings[i].x = randomFloat(200, worldRight - 200);
                    lightnings[i].brightness = 1; lightnings[i].duration = randomFloat(0.1f, 0.3f); lightnings[i].timer = 0;
                    lightnings[i].segments.clear();
                    float lx = lightnings[i].x, ly = worldTop;
                    lightnings[i].segments.push_back(Vec2(lx, ly));
                    for (int j = 0; j < randomInt(5, 12); j++) {
                        lx += randomFloat(-40, 40); ly -= randomFloat(30, 80);
                        lightnings[i].segments.push_back(Vec2(lx, ly));
                    }
                    screenFlash = true; screenFlashAlpha = 0.3f;
                    break;
                }
            }
        }
    }
    for (int i = 0; i < MAX_LIGHTNING; i++) {
        if (!lightnings[i].active) continue;
        lightnings[i].timer += deltaTime;
        lightnings[i].brightness = 1.0f - (lightnings[i].timer / lightnings[i].duration);
        if (lightnings[i].timer >= lightnings[i].duration) lightnings[i].active = false;
    }
    if (screenFlash) { screenFlashAlpha -= 0.02f; if (screenFlashAlpha <= 0) { screenFlash = false; screenFlashAlpha = 0; } }
}

void updateSmoke() {
    float rate = isRainyMode ? 0.02f : 0.05f;
    float chimneys[][2] = { {200, 640}, {380, 660}, {1600, 620}, {1750, 610} };
    for (int c = 0; c < 4; c++) {
        if (randomFloat(0, 1) < rate) {
            for (int i = 0; i < MAX_SMOKE_PARTICLES; i++) {
                if (!smokeParticles[i].active) {
                    smokeParticles[i].active = true;
                    smokeParticles[i].x = chimneys[c][0] + randomFloat(-3, 3);
                    smokeParticles[i].y = chimneys[c][1];
                    smokeParticles[i].speedX = randomFloat(-0.2f, 0.2f) + windStrength * windDirection * 0.3f;
                    smokeParticles[i].speedY = randomFloat(0.5f, 1.5f);
                    smokeParticles[i].alpha = randomFloat(0.3f, 0.6f);
                    smokeParticles[i].size = randomFloat(5, 12); smokeParticles[i].life = 1;
                    break;
                }
            }
        }
    }
    for (int i = 0; i < MAX_SMOKE_PARTICLES; i++) {
        if (!smokeParticles[i].active) continue;
        smokeParticles[i].x += smokeParticles[i].speedX;
        smokeParticles[i].y += smokeParticles[i].speedY;
        smokeParticles[i].speedX += windStrength * windDirection * 0.005f;
        smokeParticles[i].size += 0.1f; smokeParticles[i].alpha -= 0.005f; smokeParticles[i].life -= 0.008f;
        if (smokeParticles[i].life <= 0 || smokeParticles[i].alpha <= 0) smokeParticles[i].active = false;
    }
}

void updateFish() {
    for (int i = 0; i < MAX_FISH; i++) {
        if (!fishes[i].active) continue;
        fishes[i].x += fishes[i].speed * fishes[i].direction;
        fishes[i].y += sin(globalTime * 1.5f + fishes[i].phase) * 0.2f;
        fishes[i].tailAngle = sin(globalTime * fishes[i].tailSpeed + fishes[i].phase) * 20;
        if (fishes[i].direction == 1 && fishes[i].x > worldRight + 50) fishes[i].x = -30;
        if (fishes[i].direction == -1 && fishes[i].x < -50) fishes[i].x = worldRight + 30;
    }
}

void updateButterflies() {
    float activity = 1.0f - modeTransition;
    for (int i = 0; i < MAX_BUTTERFLIES; i++) {
        if (activity < 0.3f) { butterflies[i].active = false; continue; }
        if (!butterflies[i].active && activity > 0.5f) butterflies[i].active = true;
        if (!butterflies[i].active) continue;
        float dx = butterflies[i].targetX - butterflies[i].x;
        float dy = butterflies[i].targetY - butterflies[i].y;
        if (sqrt(dx * dx + dy * dy) < 20) {
            butterflies[i].targetX = randomFloat(100, worldRight - 100);
            butterflies[i].targetY = randomFloat(400, 700);
        }
        butterflies[i].x += dx * 0.01f + sin(globalTime * 2 + butterflies[i].phase) * 0.5f;
        butterflies[i].y += dy * 0.01f + cos(globalTime * 1.5f + butterflies[i].phase) * 0.3f;
        butterflies[i].wingAngle = sin(globalTime * butterflies[i].wingSpeed) * 40;
    }
}

void updateDragonflies() {
    float activity = 1.0f - modeTransition;
    for (int i = 0; i < MAX_DRAGONFLIES; i++) {
        if (activity < 0.3f) { dragonflies[i].active = false; continue; }
        if (!dragonflies[i].active && activity > 0.5f) dragonflies[i].active = true;
        if (!dragonflies[i].active) continue;
        dragonflies[i].x += sin(globalTime * 0.5f + dragonflies[i].phase) * dragonflies[i].speed;
        dragonflies[i].y += cos(globalTime * 0.7f + dragonflies[i].phase) * dragonflies[i].speed * 0.5f;
        dragonflies[i].wingAngle = sin(globalTime * 15 + i) * 30;
        dragonflies[i].x = clamp(dragonflies[i].x, 100.0f, worldRight - 100.0f);
        dragonflies[i].y = clamp(dragonflies[i].y, 330.0f, 650.0f);
    }
}

void updateWaterParticles() {
    for (int i = 0; i < MAX_WATER_PARTICLES; i++) {
        if (!waterParticles[i].active) continue;
        waterParticles[i].x += waterParticles[i].speedX;
        waterParticles[i].y += waterParticles[i].speedY;
        waterParticles[i].speedY -= 0.1f;
        waterParticles[i].alpha -= 0.02f; waterParticles[i].life -= 0.02f;
        if (waterParticles[i].life <= 0 || waterParticles[i].alpha <= 0 || waterParticles[i].y < 0)
            waterParticles[i].active = false;
    }
}

void updateRickshaws() {
    if (rickshaw1.isMoving) {
        rickshaw1.x += rickshaw1.speed * rickshaw1.direction;
        rickshaw1.wheelAngle += rickshaw1.speed * 3; rickshaw1.bouncePhase += 0.15f;
        rickshaw1.bodyBounce = sin(rickshaw1.bouncePhase) * 1.5f;
        rickshaw1.pedalAngle += rickshaw1.speed * 5;
        if (rickshaw1.x < -200) rickshaw1.x = worldRight + 200;
        if (rickshaw1.x > worldRight + 200) rickshaw1.x = -200;
    }
    if (rickshaw2.isMoving) {
        rickshaw2.x += rickshaw2.speed * rickshaw2.direction;
        rickshaw2.wheelAngle += rickshaw2.speed * 3; rickshaw2.bouncePhase += 0.12f;
        rickshaw2.bodyBounce = sin(rickshaw2.bouncePhase) * 1;
        rickshaw2.pedalAngle += rickshaw2.speed * 5;
        if (rickshaw2.x < -200) rickshaw2.x = worldRight + 200;
        if (rickshaw2.x > worldRight + 200) rickshaw2.x = -200;
    }
    if (isRainyMode) {
        rickshaw1.speed = lerp(rickshaw1.speed, 0.5f, 0.01f);
        rickshaw2.speed = lerp(rickshaw2.speed, 0.3f, 0.01f);
    }
    else {
        rickshaw1.speed = lerp(rickshaw1.speed, 1.2f, 0.01f);
        rickshaw2.speed = lerp(rickshaw2.speed, 0.8f, 0.01f);
    }
}

void updateBoats() {
    float bobExtra = isRainyMode ? 2.0f : 0.0f;
    boat1.x += boat1.speed * boat1.direction;
    boat1.bobPhase += 0.03f; boat1.rockPhase += 0.02f;
    boat1.y = 200 + sin(boat1.bobPhase) * (boat1.bobAmplitude + bobExtra);
    boat1.rockAngle = sin(boat1.rockPhase) * (3 + bobExtra);
    if (boat1.x < -200) boat1.x = worldRight + 200;
    if (boat1.x > worldRight + 200) boat1.x = -200;

    boat2.x += boat2.speed * boat2.direction;
    boat2.bobPhase += 0.025f; boat2.rockPhase += 0.018f;
    boat2.y = 170 + sin(boat2.bobPhase) * (boat2.bobAmplitude + bobExtra);
    boat2.rockAngle = sin(boat2.rockPhase) * (4 + bobExtra);
    boat2.sailAngle = sin(globalTime * 0.5f) * 5 + windStrength * windDirection * 10;
    if (boat2.x < -200) boat2.x = worldRight + 200;
    if (boat2.x > worldRight + 200) boat2.x = -200;

    boat3.x += boat3.speed * boat3.direction;
    boat3.bobPhase += 0.02f; boat3.rockPhase += 0.015f;
    boat3.y = 150 + sin(boat3.bobPhase) * (boat3.bobAmplitude + bobExtra);
    boat3.rockAngle = sin(boat3.rockPhase) * (2.5f + bobExtra);
    if (boat3.x < -200) boat3.x = worldRight + 200;
    if (boat3.x > worldRight + 200) boat3.x = -200;
}

void updateKite() {
    kite.swayPhase += 0.05f;
    kite.ownerX = persons[6].x; kite.ownerY = persons[6].y + 30;
    float sway = sin(kite.swayPhase) * 30 + windStrength * windDirection * 20;
    kite.x = kite.ownerX + 70 + sway;
    kite.y = kite.ownerY + kite.height + sin(kite.swayPhase * 0.7f) * 15;
    if (isRainyMode) kite.height = lerp(kite.height, 60, 0.005f);
    else kite.height = lerp(kite.height, 150, 0.005f);
}

void updateChickens() {
    for (int i = 0; i < 5; i++) {
        if (!chickens[i].active) continue;
        chickens[i].peckTimer -= deltaTime;
        if (chickens[i].isPecking) {
            chickens[i].peckPhase += 0.2f;
            if (chickens[i].peckTimer <= 0) {
                chickens[i].isPecking = false; chickens[i].peckTimer = randomFloat(1, 4);
                chickens[i].direction = (rand() % 2 == 0) ? 1 : -1;
            }
        }
        else {
            chickens[i].x += chickens[i].speed * chickens[i].direction;
            chickens[i].walkPhase += 0.1f;
            if (chickens[i].peckTimer <= 0) { chickens[i].isPecking = true; chickens[i].peckTimer = randomFloat(1, 3); }
            if (chickens[i].x < 450 || chickens[i].x > 800) chickens[i].direction = -chickens[i].direction;
        }
        if (isRainyMode && modeTransition > 0.5f) {
            chickens[i].speed = lerp(chickens[i].speed, 1.5f, 0.01f);
            if (chickens[i].x > 600) chickens[i].direction = -1;
            else if (chickens[i].x < 400) chickens[i].direction = 1;
        }
    }
}

void updateAll() {
    globalTime += deltaTime;
    updateModeTransition(); updateRainDrops(); updateRipples(); updateClouds();
    updateBirds(); updateLeaves(); updateFireflies(); updateStars();
    updatePuddles(); updateLightning(); updateSmoke(); updateFish();
    updateButterflies(); updateDragonflies(); updateWaterParticles();
    updateRickshaws(); updateBoats(); updateKite(); updateChickens();
    palmSwayAngle = sin(globalTime * 1.0f) * (3 + windStrength * 8);
    waterWavePhase += 0.02f;
    updateScreenShake(); // screen shake transform (lightning)
}

// ============================================================
// DRAWING - SKY, SUN, MOON (ENHANCED)
// ============================================================

void drawSky() {
    Color topColor = lerpColor(skyTopNormal, skyTopRainy, modeTransition);
    Color midColor = lerpColor(skyMidNormal, skyMidRainy, modeTransition);
    Color bottomColor = lerpColor(skyBottomNormal, skyBottomRainy, modeTransition);
    drawGradientRect(0, worldTop * 0.6f, worldRight, worldTop * 0.4f, midColor, topColor);
    drawGradientRect(0, waterLevel, worldRight, worldTop * 0.6f - waterLevel, bottomColor, midColor);
}

void drawSun() {
    float visibility = 1.0f - modeTransition;
    if (visibility < 0.05f) return;

    // Enhanced atmospheric haze around sun
    for (int i = 10; i >= 0; i--) {
        float r = sunRadius + i * 40;
        float alpha = (0.08f - i * 0.007f) * visibility;
        float warmth = (float)i / 10.0f;
        setColor(1.0f, 0.65f + warmth * 0.15f, 0.15f + warmth * 0.1f, alpha);
        drawCircle(sunX, sunY, r, 64);
    }

    // Outer corona with radial gradient
    for (int i = 8; i >= 0; i--) {
        float r = sunRadius + i * 25;
        float alpha = (0.15f - i * 0.015f) * visibility;
        setColor(1.0f, 0.80f + i * 0.02f, 0.3f + i * 0.03f, alpha);
        drawCircle(sunX, sunY, r, 64);
    }

    // Sun body with realistic color gradient (hot center to cooler edges)
    for (int ring = 30; ring >= 0; ring--) {
        float t = (float)ring / 30.0f;
        float r = sunRadius * (1.0f - t * 0.02f);
        // Center is white-yellow, edges are orange-gold
        float cr = 1.0f;
        float cg = 0.98f - t * 0.15f;
        float cb = 0.85f - t * 0.45f;
        setColor(cr, cg, cb, visibility);
        drawCircle(sunX, sunY, r - ring * sunRadius * 0.03f, 48);
    }

    // Hot white core
    setColor(1.0f, 1.0f, 0.98f, visibility * 0.95f);
    drawCircle(sunX, sunY, sunRadius * 0.4f, 32);

    // Specular highlight (slightly offset for 3D feel)
    setColor(1.0f, 1.0f, 1.0f, visibility * 0.7f);
    drawCircle(sunX - sunRadius * 0.12f, sunY + sunRadius * 0.12f, sunRadius * 0.22f, 20);

    // Dynamic sun rays with varying lengths and opacity
    for (int i = 0; i < 24; i++) {
        float angle = i * 15.0f * DEG_TO_RAD + globalTime * 0.03f;
        float innerR = sunRadius + 10;
        float lengthVariation = sin(globalTime * 0.6f + i * 0.9f) * 30 + sin(globalTime * 0.3f + i * 1.3f) * 15;
        float outerR = sunRadius + 50 + lengthVariation;
        float x1 = sunX + cos(angle) * innerR, y1 = sunY + sin(angle) * innerR;
        float x2 = sunX + cos(angle) * outerR, y2 = sunY + sin(angle) * outerR;
        float rayAlpha = (0.05f + 0.03f * sin(globalTime * 0.5f + i)) * visibility;
        setColor(1.0f, 0.90f, 0.4f, rayAlpha);
        drawThickLine(x1, y1, x2, y2, 3.0f + sin(i * 0.5f) * 1.5f);
    }

    // Thin sharp rays alternating
    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f * DEG_TO_RAD + globalTime * 0.02f + 7.5f * DEG_TO_RAD;
        float innerR = sunRadius + 5;
        float outerR = sunRadius + 90 + sin(globalTime * 0.4f + i * 1.5f) * 25;
        float x1 = sunX + cos(angle) * innerR, y1 = sunY + sin(angle) * innerR;
        float x2 = sunX + cos(angle) * outerR, y2 = sunY + sin(angle) * outerR;
        setColor(1.0f, 0.95f, 0.6f, 0.03f * visibility);
        drawThickLine(x1, y1, x2, y2, 1.5f);
    }

    // Sun reflection on water (enhanced shimmer path)
    if (sunY > waterLevel - 50) {
        float reflY = waterLevel - (sunY - waterLevel) * 0.2f;
        for (int i = 0; i < 35; i++) {
            float alpha = (0.30f - i * 0.008f) * visibility;
            float offset = sin(waterWavePhase * 1.5f + i * 0.4f) * (3 + i * 0.3f);
            float width = 3 + i * 3.0f;
            float shimmer = sin(waterWavePhase * 3 + i * 0.8f) * 0.5f + 0.5f;
            setColor(1, 0.88f, 0.45f, alpha * shimmer);
            drawRect(sunX - width + offset, reflY - i * 6, width * 2, 3);
        }
    }
}

void drawCrescentMoon() {
    if (modeTransition < 0.3f) return;
    float moonAlpha = (modeTransition - 0.3f) / 0.7f;
    float mx = 1550, my = 880, mr = 45;

    // Enhanced moon glow - multiple soft layers
    for (int i = 8; i >= 0; i--) {
        float r = mr + i * 18;
        float alpha = moonAlpha * 0.04f * (9 - i);
        setColor(0.65f, 0.70f, 0.90f, alpha);
        drawCircle(mx, my, r, 64);
    }

    // Outer atmospheric halo
    setColor(0.50f, 0.55f, 0.75f, moonAlpha * 0.08f);
    drawCircle(mx, my, mr + 60, 48);

    // Full moon disc with gradient
    for (int ring = 15; ring >= 0; ring--) {
        float t = (float)ring / 15.0f;
        float shade = 0.88f + t * 0.07f;
        setColor(shade, shade + 0.01f, shade + 0.04f, moonAlpha);
        drawCircle(mx, my, mr - ring * 0.1f, 48);
    }

    // Moon surface detail - craters with 3D shading
    // Large crater
    setColor(0.78f, 0.79f, 0.83f, moonAlpha * 0.5f);
    drawCircle(mx - 12, my + 10, 10, 16);
    setColor(0.85f, 0.86f, 0.90f, moonAlpha * 0.3f);
    drawCircle(mx - 10, my + 12, 6, 12);

    // Medium craters
    setColor(0.80f, 0.81f, 0.85f, moonAlpha * 0.4f);
    drawCircle(mx + 14, my - 6, 7, 12);
    setColor(0.86f, 0.87f, 0.91f, moonAlpha * 0.25f);
    drawCircle(mx + 15, my - 4, 4, 10);

    // Small craters
    setColor(0.82f, 0.83f, 0.87f, moonAlpha * 0.35f);
    drawCircle(mx - 5, my - 14, 5, 10);
    drawCircle(mx + 5, my + 18, 4, 10);
    drawCircle(mx - 18, my - 5, 3, 8);

    // Mare (dark areas)
    setColor(0.75f, 0.76f, 0.80f, moonAlpha * 0.2f);
    drawEllipse(mx - 8, my + 2, 18, 12, 20);
    setColor(0.77f, 0.78f, 0.82f, moonAlpha * 0.15f);
    drawEllipse(mx + 10, my + 8, 12, 8, 16);

    // Crescent shadow - dark overlay for crescent shape
    Color skyDark = lerpColor(skyTopRainy, Color(0.04f, 0.04f, 0.07f), 0.6f);
    setColor(skyDark.r, skyDark.g, skyDark.b, moonAlpha);
    drawCircle(mx + 20, my + 10, mr - 2, 48);

    // Bright edge of crescent (limb brightening)
    setColor(0.95f, 0.96f, 1.0f, moonAlpha * 0.6f);
    // Draw thin bright arc on the lit edge
    for (int i = 0; i < 30; i++) {
        float angle = -PI * 0.35f + (float)i / 29.0f * PI * 0.7f;
        float px = mx + cos(angle) * (mr - 1);
        float py = my + sin(angle) * (mr - 1);
        drawCircle(px, py, 1.5f, 6);
    }

    // Enhanced moonlight reflection on water
    for (int i = 0; i < 25; i++) {
        float y = waterLevel - i * 7;
        float wave = sin(waterWavePhase * 0.6f + i * 0.35f) * (5 + i * 0.4f);
        float alpha = (0.15f - i * 0.005f) * moonAlpha;
        float shimmer = sin(waterWavePhase * 2 + i * 0.6f) * 0.5f + 0.5f;
        setColor(0.75f, 0.82f, 0.95f, alpha * shimmer);
        drawRect(mx - 25 + wave, y - 2, 50, 3);
    }
}

void drawStars() {
    for (int i = 0; i < MAX_STARS; i++) {
        if (!stars[i].active || stars[i].brightness < 0.03f) continue;
        float b = stars[i].brightness;
        setColor(1, 1, 0.95f, b * 0.7f);
        drawCircle(stars[i].x, stars[i].y, stars[i].size, 8);
        if (b > 0.35f) {
            setColor(1, 1, 1, b * 0.3f);
            drawLine(stars[i].x - stars[i].size * 2, stars[i].y, stars[i].x + stars[i].size * 2, stars[i].y, 1);
            drawLine(stars[i].x, stars[i].y - stars[i].size * 2, stars[i].x, stars[i].y + stars[i].size * 2, 1);
        }
    }
}

// ============================================================
// DRAWING  CLOUDS
// ============================================================

void drawRealisticCloud(Cloud& cl) {
    if (!cl.active) return;
    float cx = cl.x, cy = cl.y;
    float w = cl.width, h = cl.height;
    float dark = cl.darkness;
    float alpha = cl.alpha;

    // Determine colors based on mode
    // Normal (sunset): warm whites/golds with pink/orange tints
    // Rainy (night): dark greys with bluish-purple undertones
    float baseGray = lerp(0.95f, 0.30f, dark);
    float topGray = lerp(1.0f, 0.38f, dark);
    float bottomGray = lerp(0.80f, 0.20f, dark);
    float midGray = lerp(0.90f, 0.28f, dark);

    // Sunset warm tint
    float warmR = lerp(0.08f, 0.0f, dark);   // extra red for sunset clouds
    float warmG = lerp(0.03f, 0.0f, dark);
    float warmB = lerp(-0.05f, 0.03f, dark);  // slight blue for rain clouds

    // ---- SHADOW LAYER (below cloud) ----
    setColor(bottomGray * 0.5f + warmR * 0.3f, bottomGray * 0.5f + warmG * 0.3f,
        bottomGray * 0.55f + warmB * 0.3f, alpha * 0.2f);
    drawEllipse(cx + 12, cy - h * 0.2f, w * 0.48f, h * 0.28f, 32);

    // ---- BOTTOM LAYER (darkest, representing shadow/underside) ----
    float bR = bottomGray + warmR, bG = bottomGray + warmG, bB = bottomGray + warmB;
    setColor(bR, bG, bB + 0.02f, alpha * 0.85f);
    drawEllipse(cx - w * 0.25f, cy - h * 0.05f, w * 0.32f, h * 0.30f, 28);
    drawEllipse(cx + w * 0.25f, cy - h * 0.05f, w * 0.28f, h * 0.28f, 28);
    drawEllipse(cx, cy - h * 0.1f, w * 0.38f, h * 0.25f, 30);

    // ---- MIDDLE BODY LAYER ----
    float mR = midGray + warmR, mG = midGray + warmG, mB = midGray + warmB;
    setColor(mR, mG, mB + 0.01f, alpha * 0.9f);
    drawEllipse(cx, cy + h * 0.1f, w * 0.42f, h * 0.38f, 36);
    drawEllipse(cx - w * 0.18f, cy + h * 0.12f, w * 0.35f, h * 0.36f, 32);
    drawEllipse(cx + w * 0.20f, cy + h * 0.08f, w * 0.33f, h * 0.34f, 32);

    // ---- SUB-PUFFS for complex volumetric shape ----
    for (int j = 0; j < cl.numPuffs; j++) {
        float px = cx + cl.puffOffsets[j][0] * w;
        float py = cy + cl.puffOffsets[j][1] * h;
        float pr = cl.puffOffsets[j][2] * h;
        float variation = cl.internalVariation[j];

        // Higher puffs are brighter (lit from above by sun)
        float heightFactor = (cl.puffOffsets[j][1] + 0.2f) / 0.7f;
        heightFactor = clamp(heightFactor, 0.0f, 1.0f);
        float shade = lerp(bottomGray, topGray, heightFactor) + variation;

        float sway = sin(globalTime * 0.5f + j * 0.4f) * windStrength * 1.0f;

        setColor(shade + warmR, shade + warmG, shade + warmB + 0.01f, alpha * 0.8f);
        drawEllipse(px + sway, py, pr * 1.3f, pr, 24);

        // Inner soft highlight on each puff
        if (heightFactor > 0.4f) {
            setColor(shade + 0.05f + warmR, shade + 0.05f + warmG, shade + 0.03f + warmB, alpha * 0.3f);
            drawEllipse(px + sway - pr * 0.15f, py + pr * 0.15f, pr * 0.6f, pr * 0.5f, 16);
        }
    }

    // ---- TOP HIGHLIGHT LAYER (brightest, sun-lit) ----
    float tR = topGray + warmR * 1.2f, tG = topGray + warmG * 0.8f, tB = topGray + warmB;
    float highlightAlpha = alpha * 0.55f * (1.0f - dark * 0.6f);
    setColor(tR, tG, tB, highlightAlpha);
    drawEllipse(cx - w * 0.08f, cy + h * 0.32f, w * 0.22f, h * 0.18f, 20);
    drawEllipse(cx + w * 0.12f, cy + h * 0.30f, w * 0.18f, h * 0.15f, 18);
    drawEllipse(cx - w * 0.20f, cy + h * 0.25f, w * 0.15f, h * 0.12f, 16);

    // ---- SILVER LINING (bright edge where sun backlights the cloud) ----
    float silverAlpha = alpha * cl.silverLiningIntensity * (1.0f - dark * 0.3f);
    float silverBright = lerp(1.0f, 0.50f, dark);
    setColor(silverBright, silverBright - 0.02f, silverBright - 0.08f, silverAlpha * 0.25f);

    // Draw silver lining as series of small circles along top/right edge
    for (int i = 0; i < 20; i++) {
        float t = (float)i / 19.0f;
        float angle = PI * 0.2f + t * PI * 0.6f; // arc along top
        float edgeX = cx + cos(angle) * w * 0.42f;
        float edgeY = cy + sin(angle) * h * 0.40f;
        float edgeR = 3 + sin(i * 0.8f) * 2;
        setColor(silverBright, silverBright, silverBright - 0.05f, silverAlpha * 0.15f * (1.0f - fabs(t - 0.5f) * 2.0f));
        drawCircle(edgeX, edgeY, edgeR, 8);
    }

    // ---- SUNSET COLOR TINT overlay (warm glow during sunset) ----
    if (dark < 0.5f) {
        float tintStrength = (1.0f - dark * 2.0f) * 0.1f;
        // Bottom gets orange/pink tint from sunset light
        setColor(1.0f, 0.65f, 0.35f, alpha * tintStrength);
        drawEllipse(cx, cy - h * 0.05f, w * 0.40f, h * 0.20f, 24);
        // Top gets golden tint
        setColor(1.0f, 0.90f, 0.55f, alpha * tintStrength * 0.5f);
        drawEllipse(cx, cy + h * 0.25f, w * 0.30f, h * 0.15f, 20);
    }

    // ---- RAIN MODE: dark underbelly effect ----
    if (dark > 0.3f) {
        float rainDark = (dark - 0.3f) / 0.7f;
        setColor(0.15f, 0.15f, 0.20f, alpha * rainDark * 0.3f);
        drawEllipse(cx, cy - h * 0.15f, w * 0.40f, h * 0.20f, 24);
    }

    // ---- EDGE SOFTNESS: subtle translucent edge ----
    setColor(baseGray + warmR, baseGray + warmG, baseGray + warmB + 0.02f, alpha * 0.15f * cl.edgeSoftness);
    drawEllipse(cx, cy + h * 0.05f, w * 0.50f, h * 0.45f, 36);
}

void drawAllClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) drawRealisticCloud(clouds[i]);
}

// ============================================================
// DRAWING - LANDSCAPE
// ============================================================

void drawGrass() {
    Color grass = lerpColor(grassNormal, grassRainy, modeTransition);
    Color grassDark = lerpColor(grassDarkNormal, grassDarkRainy, modeTransition);
    setColor(grass); drawRect(0, 480, worldRight, 200);
    setColor(grass); drawRect(0, waterLevel, worldRight, 90);

    for (int i = 0; i < 200; i++) {
        float gx = fmod(i * 9.7f + sin(i * 0.3f) * 20, worldRight);
        float gy = waterLevel + 10 + sin(i * 0.5f) * 30;
        float sway = sin(globalTime * 2 + i * 0.1f) * windStrength * 3;
        float height = 8 + sin(i * 0.7f) * 5;
        setColor(grassDark.r, grassDark.g, grassDark.b, 0.7f);
        drawLine(gx, gy, gx + sway, gy + height, 1.5f);
    }
    for (int i = 0; i < 150; i++) {
        float gx = fmod(i * 13.0f + sin(i * 0.4f) * 15, worldRight);
        float gy = 490 + sin(i * 0.3f) * 20;
        float sway = sin(globalTime * 2.5f + i * 0.15f) * windStrength * 3;
        float height = 10 + sin(i * 0.8f) * 6;
        setColor(grassDark.r, grassDark.g + 0.05f, grassDark.b, 0.6f);
        drawLine(gx, gy, gx + sway, gy + height, 1.5f);
    }
}

void drawRoad() {
    Color road = lerpColor(roadNormal, roadRainy, modeTransition);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 100; i++) {
        float t = (float)i / 100;
        float x = t * worldRight;
        float yCenter = 440 + sin(t * PI * 0.5f) * 20 - t * 20;
        float roadWidth = 50 + t * 10;
        setColor(road.r - 0.05f, road.g - 0.05f, road.b - 0.05f);
        glVertex2f(x, yCenter - roadWidth / 2);
        setColor(road.r + 0.05f, road.g + 0.05f, road.b + 0.05f);
        glVertex2f(x, yCenter + roadWidth / 2);
    }
    glEnd();

    if (modeTransition > 0.3f) {
        float reflAlpha = modeTransition * 0.15f;
        for (int i = 0; i < 50; i++) {
            float t = randomFloat(0, 1);
            float x = t * worldRight;
            float yCenter = 440 + sin(t * PI * 0.5f) * 20 - t * 20;
            setColor(0.4f, 0.45f, 0.55f, reflAlpha);
            drawRect(x - 10, yCenter - 15, 20, 3);
        }
    }
}

void drawWater() {
    Color waterTop = lerpColor(waterTopNormal, waterTopRainy, modeTransition);
    Color waterBottom = lerpColor(waterBottomNormal, waterBottomRainy, modeTransition);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 100; i++) {
        float t = (float)i / 100;
        float x = t * worldRight;
        float waveHeight = sin(waterWavePhase + t * 10) * 5 + sin(waterWavePhase * 1.5f + t * 15) * 3;
        glColor4f(waterTop.r, waterTop.g, waterTop.b, 0.9f);
        glVertex2f(x, waterLevel + waveHeight);
        glColor4f(waterBottom.r, waterBottom.g, waterBottom.b, 1);
        glVertex2f(x, 0);
    }
    glEnd();

    // Water surface highlights
    for (int i = 0; i < 60; i++) {
        float t = (float)i / 60;
        float x = t * worldRight;
        float waveHeight = sin(waterWavePhase + t * 10) * 5 + sin(waterWavePhase * 1.5f + t * 15) * 3;
        float brightness = 0.1f + 0.05f * sin(waterWavePhase * 2 + t * 20);
        brightness *= (1.0f - modeTransition * 0.5f);
        setColor(1, 0.95f, 0.7f, brightness);
        drawRect(x, waterLevel + waveHeight - 2, worldRight / 60.0f, 3);
    }

    // Ripples
    for (int i = 0; i < MAX_RIPPLES; i++) {
        if (!ripples[i].active) continue;
        setColor(0.7f, 0.85f, 0.95f, ripples[i].alpha);
        drawCircleOutline(ripples[i].x, ripples[i].y, ripples[i].radius, 16);
    }
}

void drawGrassland() {
    float darken = modeTransition * 0.15f;
    setColor(0.20f - darken, 0.55f - darken, 0.18f - darken);
    drawRect(0, 480, worldRight, 30);
}

void drawDistantTrees() {
    float darken = modeTransition * 0.1f;
    for (int i = 0; i < 25; i++) {
        float tx = 50 + i * 80 + sin(i * 2.3f) * 30;
        float ty = 490 + sin(i * 0.4f) * 5;
        float scale = 0.3f + sin(i * 0.6f) * 0.1f;
        if (tx > worldRight) continue;
        setColor(0.35f - darken, 0.25f - darken, 0.12f - darken);
        drawRect(tx - 2 * scale, ty, 4 * scale, 25 * scale);
        setColor(0.15f - darken + (i % 3) * 0.03f, 0.45f - darken + (i % 3) * 0.05f, 0.10f - darken);
        drawCircle(tx, ty + 30 * scale, 15 * scale, 12);
        drawCircle(tx - 8 * scale, ty + 25 * scale, 10 * scale, 10);
        drawCircle(tx + 8 * scale, ty + 25 * scale, 10 * scale, 10);
    }
}

void drawRiverBank() {
    float darken = modeTransition * 0.1f;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 50; i++) {
        float t = (float)i / 50;
        float x = t * worldRight;
        setColor(0.45f - darken, 0.38f - darken, 0.22f - darken);
        glVertex2f(x, waterLevel + 15 + sin(t * 10 + 1) * 5);
        setColor(0.35f - darken, 0.28f - darken, 0.15f - darken);
        glVertex2f(x, waterLevel - 5 + sin(t * 10 + 1) * 3);
    }
    glEnd();
}

// ============================================================
// DRAWING - HOUSES WITH GL_POLYGON AND BRESENHAM'S
// ============================================================

void drawHouseWithPolygon(float x, float y, float wallW, float wallH, float roofH, float overhang, float scale) {
    float d = modeTransition * 0.15f;

    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1);

    // Wall using GL_POLYGON
    setColor(0.75f - d, 0.68f - d, 0.55f - d);
    glBegin(GL_POLYGON);
    glVertex2f(0, 0);
    glVertex2f(wallW, 0);
    glVertex2f(wallW, wallH);
    glVertex2f(0, wallH);
    glEnd();

    // Wall texture using Bresenham's lines
    setColor(0.60f - d, 0.52f - d, 0.38f - d, 0.3f);
    for (int i = 0; i < (int)(wallW / 15); i++) {
        float lx = i * 15 + 5;
        bresenhamLine(lx, 0, lx, wallH, 1.0f);
    }
    for (int i = 0; i < (int)(wallH / 12); i++) {
        float ly = i * 12 + 6;
        bresenhamLine(0, ly, wallW, ly, 0.8f);
    }

    // Left shadow
    setColor(0.55f - d, 0.48f - d, 0.35f - d, 0.3f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 0); glVertex2f(wallW * 0.08f, 0);
    glVertex2f(wallW * 0.08f, wallH); glVertex2f(0, wallH);
    glEnd();

    // Roof using GL_POLYGON (thatched)
    setColor(0.55f - d, 0.42f - d, 0.22f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-overhang, wallH);
    glVertex2f(wallW + overhang, wallH);
    glVertex2f(wallW / 2, wallH + roofH);
    glEnd();

    // Roof texture lines using Bresenham's
    setColor(0.50f - d, 0.38f - d, 0.18f - d, 0.6f);
    for (int i = 1; i < 8; i++) {
        float t = (float)i / 8;
        float ly = wallH + roofH * t;
        float leftX = -overhang + (overhang + wallW / 2.0f) * t;
        float rightX = wallW + overhang - (overhang + wallW / 2.0f) * t;
        bresenhamLine(leftX, ly, rightX, ly, 1.5f);
    }

    // Roof outline using Bresenham's
    setColor(0.45f - d, 0.35f - d, 0.18f - d);
    bresenhamLine(-overhang, wallH, wallW / 2, wallH + roofH, 2.0f);
    bresenhamLine(wallW + overhang, wallH, wallW / 2, wallH + roofH, 2.0f);
    bresenhamLine(-overhang, wallH, wallW + overhang, wallH, 2.0f);

    // Door using GL_POLYGON
    float doorW = wallW * 0.22f, doorH = wallH * 0.65f;
    float doorX = wallW * 0.4f;
    setColor(0.35f - d, 0.25f - d, 0.15f - d);
    glBegin(GL_POLYGON);
    glVertex2f(doorX - 3, 0); glVertex2f(doorX + doorW + 3, 0);
    glVertex2f(doorX + doorW + 3, doorH + 3); glVertex2f(doorX - 3, doorH + 3);
    glEnd();
    setColor(0.50f - d, 0.35f - d, 0.20f - d);
    glBegin(GL_POLYGON);
    glVertex2f(doorX, 0); glVertex2f(doorX + doorW, 0);
    glVertex2f(doorX + doorW, doorH); glVertex2f(doorX, doorH);
    glEnd();
    setColor(1.0f, 0.9f, 0.5f, 0.1f + modeTransition * 0.3f);
    glBegin(GL_POLYGON);
    glVertex2f(doorX + 2, 0); glVertex2f(doorX + doorW - 2, 0);
    glVertex2f(doorX + doorW - 2, doorH - 2); glVertex2f(doorX + 2, doorH - 2);
    glEnd();
    setColor(0.70f, 0.65f, 0.30f);
    midpointCircleFilled(doorX + doorW * 0.8f, doorH * 0.5f, 2);

    // Window using GL_POLYGON
    float winW = wallW * 0.15f, winH = wallH * 0.35f;
    float winX = wallW * 0.72f, winY = wallH * 0.45f;

    setColor(0.40f - d, 0.30f - d, 0.18f - d);
    glBegin(GL_POLYGON);
    glVertex2f(winX - 2, winY - 2); glVertex2f(winX + winW + 2, winY - 2);
    glVertex2f(winX + winW + 2, winY + winH + 2); glVertex2f(winX - 2, winY + winH + 2);
    glEnd();

    float winGlow = modeTransition * 0.5f;
    setColor(0.95f, 0.85f, 0.4f, 0.3f + winGlow);
    glBegin(GL_POLYGON);
    glVertex2f(winX, winY); glVertex2f(winX + winW, winY);
    glVertex2f(winX + winW, winY + winH); glVertex2f(winX, winY + winH);
    glEnd();

    if (modeTransition > 0.3f) {
        setColor(1, 0.9f, 0.4f, winGlow * 0.3f);
        drawCircle(winX + winW / 2, winY + winH / 2, winW * 1.5f, 16);
    }

    setColor(0.35f - d, 0.25f - d, 0.15f - d);
    bresenhamLine(winX + winW / 2, winY, winX + winW / 2, winY + winH, 2.0f);
    bresenhamLine(winX, winY + winH / 2, winX + winW, winY + winH / 2, 2.0f);

    // Foundation stilts
    setColor(0.40f - d, 0.30f - d, 0.18f - d);
    glBegin(GL_POLYGON);
    glVertex2f(8, -15); glVertex2f(16, -15); glVertex2f(16, 0); glVertex2f(8, 0);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(wallW - 16, -15); glVertex2f(wallW - 8, -15);
    glVertex2f(wallW - 8, 0); glVertex2f(wallW - 16, 0);
    glEnd();

    glPopMatrix();
}

void drawAllHouses() {
    drawHouseWithPolygon(80, 520, 100, 60, 50, 15, 1.0f);
    drawHouseWithPolygon(250, 530, 130, 80, 60, 20, 1.1f);
    drawHouseWithPolygon(430, 540, 80, 50, 45, 12, 0.9f);
    drawHouseWithPolygon(1550, 500, 110, 65, 55, 18, 0.95f);
    drawHouseWithPolygon(1700, 490, 80, 50, 45, 12, 0.85f);

    // Market stall
    float d = modeTransition * 0.15f;
    float mx = 1400, my = 500;
    setColor(0.45f - d, 0.32f - d, 0.18f - d);
    drawRect(mx, my, 5, 70); drawRect(mx + 80, my, 5, 70); drawRect(mx + 40, my, 5, 70);
    setColor(0.60f - d, 0.45f - d, 0.25f - d);
    drawQuad(mx - 10, my + 70, mx + 95, my + 70, mx + 90, my + 80, mx - 5, my + 80);
    setColor(0.9f, 0.2f, 0.15f, 0.7f); drawRect(mx - 8, my + 70, 100, 3);
    setColor(0.2f, 0.7f, 0.3f, 0.7f); drawRect(mx - 8, my + 73, 100, 3);
    setColor(0.9f, 0.8f, 0.1f, 0.7f); drawRect(mx - 8, my + 76, 100, 3);
}

// ============================================================
// DRAWING - BANYAN TREES 
// ============================================================

void drawBanyanTree(float x, float y, float scale) {
    float d = modeTransition * 0.1f;

    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1);

    // ---- MAIN THICK TRUNK with natural curves ----
    setColor(0.35f - d, 0.25f - d, 0.13f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-14, 0); glVertex2f(14, 0);
    glVertex2f(16, 30); glVertex2f(18, 60);
    glVertex2f(20, 90); glVertex2f(18, 120);
    glVertex2f(15, 150); glVertex2f(10, 165);
    glVertex2f(-10, 165); glVertex2f(-15, 150);
    glVertex2f(-18, 120); glVertex2f(-20, 90);
    glVertex2f(-18, 60); glVertex2f(-16, 30);
    glEnd();

    // Trunk right side thickening (buttress root effect)
    setColor(0.33f - d, 0.23f - d, 0.12f - d);
    glBegin(GL_POLYGON);
    glVertex2f(14, 0); glVertex2f(22, -5);
    glVertex2f(25, 10); glVertex2f(20, 40);
    glVertex2f(16, 30);
    glEnd();
    // Left buttress
    glBegin(GL_POLYGON);
    glVertex2f(-14, 0); glVertex2f(-22, -5);
    glVertex2f(-25, 10); glVertex2f(-20, 40);
    glVertex2f(-16, 30);
    glEnd();

    // Bark texture using Bresenham's - horizontal cracks
    setColor(0.28f - d, 0.18f - d, 0.08f - d, 0.45f);
    for (int i = 0; i < 16; i++) {
        float ly = i * 10 + 5 + sin(i * 0.7f) * 3;
        float leftEdge = -14 + sin(ly * 0.05f) * 4;
        float rightEdge = 14 + sin(ly * 0.07f) * 4;
        bresenhamLine(leftEdge, ly, rightEdge, ly, 1.0f);
    }
    // Vertical bark grooves
    setColor(0.30f - d, 0.20f - d, 0.10f - d, 0.35f);
    for (int i = 0; i < 5; i++) {
        float lx = -10 + i * 5 + sin(i * 1.2f) * 2;
        bresenhamLine(lx, 5, lx + sin(i * 0.8f) * 6, 155, 1.2f);
    }

    // Knots on trunk using midpoint circle
    setColor(0.30f - d, 0.20f - d, 0.10f - d, 0.5f);
    midpointCircle(5, 80, 4, 1.5f);
    midpointCircleFilled(-7, 50, 3);
    midpointCircle(-3, 120, 3, 1.2f);

    // ---- MAIN BRANCHES (thick, spreading) ----
    setColor(0.33f - d, 0.23f - d, 0.11f - d);
    // Left main branch
    glBegin(GL_POLYGON);
    glVertex2f(-12, 130); glVertex2f(-6, 135);
    glVertex2f(-35, 195); glVertex2f(-48, 185);
    glEnd();
    // Right main branch
    glBegin(GL_POLYGON);
    glVertex2f(6, 135); glVertex2f(12, 130);
    glVertex2f(48, 185); glVertex2f(35, 195);
    glEnd();
    // Upper center branch
    glBegin(GL_POLYGON);
    glVertex2f(-5, 155); glVertex2f(5, 155);
    glVertex2f(8, 210); glVertex2f(-8, 210);
    glEnd();
    // Far left sub-branch
    glBegin(GL_POLYGON);
    glVertex2f(-40, 185); glVertex2f(-35, 190);
    glVertex2f(-65, 200); glVertex2f(-68, 192);
    glEnd();
    // Far right sub-branch
    glBegin(GL_POLYGON);
    glVertex2f(35, 190); glVertex2f(40, 185);
    glVertex2f(68, 192); glVertex2f(65, 200);
    glEnd();
    // Additional upper branches
    glBegin(GL_POLYGON);
    glVertex2f(-8, 145); glVertex2f(-4, 150);
    glVertex2f(-28, 205); glVertex2f(-35, 200);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(4, 150); glVertex2f(8, 145);
    glVertex2f(32, 200); glVertex2f(25, 205);
    glEnd();

    // ---- AERIAL ROOTS (signature banyan feature) ----
    setColor(0.38f - d, 0.28f - d, 0.14f - d, 0.65f);
    float rootData[][3] = {
        {-45, 185, 25}, {-30, 195, 40}, {-18, 200, 55},
        {-5, 205, 60},  {8, 205, 58},   {20, 200, 45},
        {35, 195, 35},  {48, 185, 20},  {-55, 192, 15},
        {55, 190, 12}
    };
    int numRoots = 10;
    for (int i = 0; i < numRoots; i++) {
        float rx = rootData[i][0];
        float topY = rootData[i][1];
        float length = rootData[i][2];
        float sway = sin(globalTime * 1.0f + i * 0.6f) * windStrength * 2.0f;
        float bottomY = topY - length;

        // Draw each root with slight waviness
        setColor(0.38f - d, 0.28f - d, 0.14f - d, 0.55f + (length > 40 ? 0.15f : 0));
        float midSway = sway * 0.5f;
        // Use Bresenham for the root line
        bresenhamLine(rx + midSway * 0.3f, topY, rx + sway, bottomY, 1.5f);

        // Thicker roots that reach the ground
        if (length > 45) {
            setColor(0.36f - d, 0.26f - d, 0.13f - d, 0.7f);
            bresenhamLine(rx + midSway * 0.3f - 1, topY, rx + sway - 1, bottomY, 2.0f);
            // Root spreading at base
            if (bottomY < 10) {
                setColor(0.35f - d, 0.25f - d, 0.12f - d, 0.5f);
                bresenhamLine(rx + sway, bottomY, rx + sway - 5, bottomY - 5, 1.5f);
                bresenhamLine(rx + sway, bottomY, rx + sway + 5, bottomY - 5, 1.5f);
            }
        }
    }

    // ---- DENSE CANOPY - many overlapping circles for lush look ----
    struct CanopyPuff { float ox, oy, r; int colorIdx; };
    CanopyPuff canopyPuffs[] = {
        // Bottom layer (darker, larger)
        {-55, 180, 30, 2}, {55, 180, 30, 2}, {-35, 175, 35, 2}, {35, 175, 35, 2},
        // Middle layer
        {0, 190, 55, 0}, {-30, 185, 42, 1}, {30, 185, 42, 1},
        {-50, 185, 32, 0}, {50, 185, 32, 0},
        // Upper middle
        {-20, 200, 40, 1}, {20, 200, 40, 1}, {0, 205, 45, 0},
        {-40, 195, 30, 1}, {40, 195, 30, 1},
        // Upper layer
        {-10, 210, 35, 3}, {10, 210, 35, 3}, {0, 215, 30, 3},
        {-25, 208, 28, 3}, {25, 208, 28, 3},
        // Top highlights
        {-5, 220, 22, 4}, {8, 218, 20, 4}, {-18, 212, 18, 4},
        // Far edges
        {-65, 192, 22, 2}, {65, 192, 22, 2},
        {-58, 198, 18, 0}, {58, 198, 18, 0},
    };
    int numCanopyPuffs = 26;

    float canopyColors[][3] = {
        {0.12f, 0.42f, 0.08f},  // 0: medium green
        {0.15f, 0.48f, 0.10f},  // 1: medium-light green
        {0.09f, 0.35f, 0.06f},  // 2: dark green (shadow)
        {0.18f, 0.52f, 0.12f},  // 3: light green
        {0.22f, 0.58f, 0.15f},  // 4: highlight green
    };

    for (int i = 0; i < numCanopyPuffs; i++) {
        int ci = canopyPuffs[i].colorIdx;
        float sway = sin(globalTime * 0.7f + i * 0.4f) * windStrength * 1.2f;
        float growth = sin(globalTime * 0.3f + i * 0.2f) * 2; // subtle breathing
        setColor(canopyColors[ci][0] - d, canopyColors[ci][1] - d, canopyColors[ci][2] - d, 0.88f);
        drawCircle(canopyPuffs[i].ox + sway, canopyPuffs[i].oy, canopyPuffs[i].r + growth, 20);
    }

    // Canopy top highlights (light dapples)
    setColor(0.25f - d, 0.62f - d, 0.18f - d, 0.35f);
    drawCircle(5 + sin(globalTime * 0.5f) * 2, 215, 18, 14);
    drawCircle(-12 + sin(globalTime * 0.6f) * 2, 205, 15, 14);
    drawCircle(15, 210, 12, 12);

    // Sunset/light dapple on canopy
    if (modeTransition < 0.5f) {
        setColor(0.4f, 0.65f, 0.2f, 0.12f * (1.0f - modeTransition * 2));
        drawCircle(20, 210, 20, 12);
        drawCircle(-15, 208, 16, 12);
    }

    // ---- TRUNK BASE ROOTS spreading on ground ----
    setColor(0.36f - d, 0.26f - d, 0.14f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-14, 0); glVertex2f(-30, -8); glVertex2f(-25, -12);
    glVertex2f(-10, -3);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(14, 0); glVertex2f(30, -8); glVertex2f(25, -12);
    glVertex2f(10, -3);
    glEnd();
    // Center root
    glBegin(GL_POLYGON);
    glVertex2f(-5, -2); glVertex2f(5, -2);
    glVertex2f(8, -10); glVertex2f(-8, -10);
    glEnd();

    glPopMatrix();
}

void drawAllBanyanTrees() {
    // ---- Banyan trees positioned EXACTLY behind each house ----
    // Left house cluster: houses at x=80, x=250, x=430
    drawBanyanTree(130, 535, 1.0f);   // Behind house 1 (x=80)
    drawBanyanTree(320, 548, 1.15f);  // Behind house 2 (x=250)
    drawBanyanTree(470, 548, 0.85f);  // Behind house 3 (x=430)

    // Right house cluster: houses at x=1550, x=1700
    drawBanyanTree(1600, 510, 1.05f); // Behind house 4 (x=1550)
    drawBanyanTree(1750, 498, 0.9f);  // Behind house 5 (x=1700)
}

// ============================================================
// DRAWING - PALM TREES
// ============================================================

void drawPalmTrunk(float x, float y, float height, float baseWidth, float topWidth) {
    float d = modeTransition * 0.1f;
    int segments = 20;
    float prevX = x, prevY = y;
    for (int i = 1; i <= segments; i++) {
        float t = (float)i / segments;
        float curveX = x + sin(t * PI * 0.3f) * 15;
        float curveY = y + t * height;
        float width = lerp(baseWidth, topWidth, t);
        float brown = 0.40f - d + t * 0.1f;
        setColor(brown, brown * 0.7f, brown * 0.4f);
        drawQuad(prevX - width / 2, prevY, curveX - width / 2, curveY,
            curveX + width / 2, curveY, prevX + width / 2, prevY);
        if (i % 3 == 0) {
            setColor(brown - 0.05f, brown * 0.65f, brown * 0.35f, 0.5f);
            drawLine(curveX - width / 2, curveY, curveX + width / 2, curveY, 1);
        }
        prevX = curveX; prevY = curveY;
    }
}

void drawPalmFrond(float x, float y, float length, float angle, float swayOffset) {
    float d = modeTransition * 0.1f;
    float totalAngle = angle + swayOffset;
    int segments = 15;
    float prevX = x, prevY = y;
    for (int i = 1; i <= segments; i++) {
        float t = (float)i / segments;
        float droop = t * t * 40;
        float fx = x + cos(totalAngle * DEG_TO_RAD) * length * t;
        float fy = y + sin(totalAngle * DEG_TO_RAD) * length * t - droop;
        setColor(0.15f - d, 0.55f - d, 0.10f - d);
        drawLine(prevX, prevY, fx, fy, 2);
        float leafLen = (1 - t) * 20;
        float perpAngle = totalAngle + 90;
        float leafSway = sin(globalTime * 3 + i * 0.5f) * windStrength * 2;
        setColor(0.18f - d + t * 0.05f, 0.60f - d - t * 0.1f, 0.12f - d);
        float lx = fx + cos((perpAngle + leafSway) * DEG_TO_RAD) * leafLen;
        float ly = fy + sin((perpAngle + leafSway) * DEG_TO_RAD) * leafLen - leafLen * 0.3f;
        drawLine(fx, fy, lx, ly, 1.5f);
        float rx = fx + cos((-perpAngle + totalAngle * 2 + leafSway) * DEG_TO_RAD) * leafLen;
        float ry = fy + sin((-perpAngle + totalAngle * 2 + leafSway) * DEG_TO_RAD) * leafLen - leafLen * 0.3f;
        drawLine(fx, fy, rx, ry, 1.5f);
        prevX = fx; prevY = fy;
    }
}

void drawPalmTree(float x, float y, float height, float scale) {
    glPushMatrix();
    glTranslatef(x, y, 0); glScalef(scale, scale, 1);
    drawPalmTrunk(0, 0, height, 12, 6);
    float topX = sin(PI * 0.3f) * 15, topY = height;
    float d = modeTransition * 0.1f;
    setColor(0.45f - d, 0.32f - d, 0.12f - d);
    drawCircle(topX - 5, topY - 5, 5, 10); drawCircle(topX + 5, topY - 3, 4, 10); drawCircle(topX, topY - 8, 4, 10);
    float fl = height * 0.5f;
    drawPalmFrond(topX, topY, fl, 30, palmSwayAngle);
    drawPalmFrond(topX, topY, fl, 60, palmSwayAngle + 2);
    drawPalmFrond(topX, topY, fl * 0.9f, 90, palmSwayAngle + 1);
    drawPalmFrond(topX, topY, fl, 120, palmSwayAngle - 1);
    drawPalmFrond(topX, topY, fl, 150, palmSwayAngle - 2);
    drawPalmFrond(topX, topY, fl * 0.85f, 0, palmSwayAngle + 3);
    drawPalmFrond(topX, topY, fl * 0.8f, 170, palmSwayAngle - 1.5f);
    glPopMatrix();
}

void drawAllPalmTrees() {
    drawPalmTree(120, 540, 200, 1.0f);
    drawPalmTree(300, 560, 220, 1.1f);
    drawPalmTree(480, 555, 180, 0.9f);
    drawPalmTree(650, 550, 190, 0.85f);
    drawPalmTree(1500, 510, 210, 1.0f);
    drawPalmTree(1700, 500, 170, 0.8f);
    drawPalmTree(1850, 510, 200, 0.95f);
    drawPalmTree(800, 530, 150, 0.6f);
    drawPalmTree(1200, 520, 140, 0.55f);
}

void drawBushes(float x, float y, float scale) {
    float d = modeTransition * 0.1f;
    glPushMatrix(); glTranslatef(x, y, 0); glScalef(scale, scale, 1);
    setColor(0.12f - d, 0.45f - d, 0.08f - d);
    drawEllipse(0, 0, 30, 20, 16);
    setColor(0.15f - d, 0.50f - d, 0.10f - d);
    drawEllipse(-15, 5, 25, 18, 16); drawEllipse(15, 5, 25, 18, 16);
    setColor(0.18f - d, 0.55f - d, 0.12f - d);
    drawEllipse(0, 10, 20, 15, 16);
    glPopMatrix();
}

void drawAllVegetation() {
    drawBushes(50, 510, 1.0f); drawBushes(550, 500, 1.2f);
    drawBushes(1350, 495, 0.9f); drawBushes(1650, 500, 1.0f);
    drawBushes(100, 340, 0.6f); drawBushes(400, 330, 0.7f);
}

// ============================================================
// DRAWING - REALISTIC BOATS WITH DDA, BRESENHAM'S, MIDPOINT CIRCLE
// ============================================================

void drawCargoBoat(Boat& boat) {
    glPushMatrix();
    glTranslatef(boat.x, boat.y, 0);
    glRotatef(boat.rockAngle, 0, 0, 1);
    if (boat.direction == -1) glScalef(-1, 1, 1);
    float d = modeTransition * 0.1f;

    // ---- HULL (Main body using GL_POLYGON) ----
    setColor(0.42f - d, 0.28f - d, 0.14f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-65, -3);
    glVertex2f(-55, -18);
    glVertex2f(-30, -22);
    glVertex2f(30, -22);
    glVertex2f(55, -18);
    glVertex2f(65, -3);
    glVertex2f(58, 6);
    glVertex2f(-58, 6);
    glEnd();

    // Hull keel (bottom darker strip) using GL_POLYGON
    setColor(0.30f - d, 0.18f - d, 0.08f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-55, -18); glVertex2f(-30, -22);
    glVertex2f(30, -22); glVertex2f(55, -18);
    glVertex2f(50, -15); glVertex2f(-50, -15);
    glEnd();

    // Hull outline using Bresenham's line algorithm
    setColor(0.25f - d, 0.15f - d, 0.06f - d);
    bresenhamLine(-65, -3, -55, -18, 2.5f);
    bresenhamLine(-55, -18, -30, -22, 2.5f);
    bresenhamLine(-30, -22, 30, -22, 2.5f);
    bresenhamLine(30, -22, 55, -18, 2.5f);
    bresenhamLine(55, -18, 65, -3, 2.5f);
    bresenhamLine(65, -3, 58, 6, 2.5f);
    bresenhamLine(58, 6, -58, 6, 2.5f);
    bresenhamLine(-58, 6, -65, -3, 2.5f);

    // Hull decorative stripes using DDA algorithm
    setColor(0.60f - d, 0.12f, 0.08f);
    ddaLine(-52, -1, 52, -1, 3.5f);
    setColor(0.15f, 0.50f - d, 0.15f);
    ddaLine(-50, -5, 50, -5, 2.0f);
    setColor(0.90f, 0.80f, 0.20f, 0.6f);
    ddaLine(-48, -8, 48, -8, 1.5f);

    // Plank lines (wood grain) using DDA
    setColor(0.38f - d, 0.23f - d, 0.10f - d, 0.35f);
    for (int i = 0; i < 8; i++) {
        float lx = -48 + i * 13;
        ddaLine(lx, -20, lx + 2, 5, 1.0f);
    }
    // Horizontal planks
    ddaLine(-55, -10, 55, -10, 0.8f);
    ddaLine(-60, 0, 60, 0, 0.8f);

    // Rivets using Midpoint Circle algorithm
    setColor(0.55f, 0.50f, 0.35f);
    float rivetY[] = { -1, -8 };
    for (int r = 0; r < 2; r++) {
        for (int i = 0; i < 10; i++) {
            float rx = -45 + i * 10;
            midpointCircleFilled(rx, rivetY[r], 1.5f);
        }
    }
    // Rivet outlines
    setColor(0.40f, 0.35f, 0.25f);
    for (int r = 0; r < 2; r++) {
        for (int i = 0; i < 10; i++) {
            float rx = -45 + i * 10;
            midpointCircle(rx, rivetY[r], 1.5f, 1.0f);
        }
    }

    // ---- CABIN using GL_POLYGON ----
    setColor(0.58f - d, 0.48f - d, 0.32f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-22, 6); glVertex2f(22, 6);
    glVertex2f(22, 32); glVertex2f(-22, 32);
    glEnd();

    // Cabin side shading
    setColor(0.50f - d, 0.40f - d, 0.25f - d, 0.4f);
    glBegin(GL_POLYGON);
    glVertex2f(-22, 6); glVertex2f(-16, 6);
    glVertex2f(-16, 32); glVertex2f(-22, 32);
    glEnd();

    // Cabin outline using Bresenham's
    setColor(0.40f - d, 0.30f - d, 0.18f - d);
    bresenhamLine(-22, 6, 22, 6, 2.0f);
    bresenhamLine(22, 6, 22, 32, 2.0f);
    bresenhamLine(22, 32, -22, 32, 2.0f);
    bresenhamLine(-22, 32, -22, 6, 2.0f);

    // Cabin plank detail using DDA
    setColor(0.52f - d, 0.42f - d, 0.28f - d, 0.3f);
    for (int i = 0; i < 4; i++) {
        float ly = 10 + i * 6;
        ddaLine(-21, ly, 21, ly, 1.0f);
    }

    // Cabin roof (slightly overhanging)
    setColor(0.48f - d, 0.38f - d, 0.22f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-27, 32); glVertex2f(27, 32);
    glVertex2f(25, 40); glVertex2f(-25, 40);
    glEnd();
    // Roof ridge
    setColor(0.42f - d, 0.32f - d, 0.18f - d);
    bresenhamLine(-27, 32, 27, 32, 2.0f);
    bresenhamLine(-25, 40, 25, 40, 1.5f);

    // Cabin windows using Midpoint Circle algorithm (round portholes)
    // Window glass
    setColor(0.85f, 0.82f, 0.50f, 0.5f + modeTransition * 0.35f);
    drawCircle(-10, 20, 6, 14);
    drawCircle(10, 20, 6, 14);
    // Window frames using Midpoint Circle
    setColor(0.35f - d, 0.25f - d, 0.12f - d);
    midpointCircle(-10, 20, 6, 2.0f);
    midpointCircle(10, 20, 6, 2.0f);
    // Window cross-hair detail
    bresenhamLine(-10, 14, -10, 26, 1.5f);
    bresenhamLine(-16, 20, -4, 20, 1.5f);
    bresenhamLine(10, 14, 10, 26, 1.5f);
    bresenhamLine(4, 20, 16, 20, 1.5f);
    // Window glow at night
    if (modeTransition > 0.3f) {
        setColor(1.0f, 0.9f, 0.45f, modeTransition * 0.25f);
        drawCircle(-10, 20, 10, 12);
        drawCircle(10, 20, 10, 12);
    }

    // ---- CARGO AREA using GL_POLYGON ----
    setColor(0.48f - d, 0.42f - d, 0.23f - d);
    glBegin(GL_POLYGON);
    glVertex2f(25, 6); glVertex2f(55, 6);
    glVertex2f(55, 22); glVertex2f(25, 22);
    glEnd();

    // Individual cargo boxes/barrels
    setColor(0.68f - d, 0.55f - d, 0.30f - d);
    glBegin(GL_POLYGON);
    glVertex2f(27, 6); glVertex2f(37, 6); glVertex2f(37, 18); glVertex2f(27, 18);
    glEnd();
    setColor(0.62f - d, 0.50f - d, 0.28f - d);
    glBegin(GL_POLYGON);
    glVertex2f(39, 6); glVertex2f(53, 6); glVertex2f(53, 16); glVertex2f(39, 16);
    glEnd();

    // Barrel using Midpoint Circle
    setColor(0.55f - d, 0.40f - d, 0.22f - d);
    drawCircle(46, 22, 8, 16);
    setColor(0.45f - d, 0.32f - d, 0.16f - d);
    midpointCircle(46, 22, 8, 1.5f);
    // Barrel hoops using DDA
    setColor(0.50f, 0.50f, 0.45f);
    ddaLine(40, 20, 52, 20, 1.5f);
    ddaLine(40, 24, 52, 24, 1.5f);

    // Cargo box outlines using Bresenham's
    setColor(0.45f - d, 0.35f - d, 0.20f - d);
    bresenhamLine(27, 6, 37, 6, 1.5f);
    bresenhamLine(37, 6, 37, 18, 1.5f);
    bresenhamLine(37, 18, 27, 18, 1.5f);
    bresenhamLine(27, 18, 27, 6, 1.5f);

    // ---- BOW DECORATION ----
    setColor(0.60f - d, 0.45f - d, 0.18f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-58, 3); glVertex2f(-68, 6); glVertex2f(-65, -3);
    glEnd();

    // Waterline mark using DDA
    setColor(0.20f, 0.45f, 0.55f, 0.35f);
    ddaLine(-60, -18, 60, -18, 2.5f);

    // Anchor circle using Midpoint Circle
    setColor(0.40f, 0.40f, 0.38f);
    midpointCircle(-50, -5, 4, 1.5f);
    midpointCircleFilled(-50, -5, 2);

    glPopMatrix();
}

void drawSailBoat(Boat& boat) {
    glPushMatrix();
    glTranslatef(boat.x, boat.y, 0);
    glRotatef(boat.rockAngle, 0, 0, 1);
    if (boat.direction == -1) glScalef(-1, 1, 1);
    float d = modeTransition * 0.1f;

    // ---- HULL using GL_POLYGON (elegant curved shape) ----
    setColor(0.52f - d, 0.22f - d, 0.08f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-55, 0);
    glVertex2f(-45, -15);
    glVertex2f(-20, -18);
    glVertex2f(20, -18);
    glVertex2f(45, -15);
    glVertex2f(60, 0);
    glVertex2f(55, 6);
    glVertex2f(-50, 6);
    glEnd();

    // Hull bottom (keel) darker
    setColor(0.35f - d, 0.12f - d, 0.04f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-45, -15); glVertex2f(-20, -18);
    glVertex2f(20, -18); glVertex2f(45, -15);
    glVertex2f(40, -12); glVertex2f(-40, -12);
    glEnd();

    // Hull outline using Bresenham's line algorithm
    setColor(0.30f - d, 0.10f - d, 0.03f - d);
    bresenhamLine(-55, 0, -45, -15, 2.5f);
    bresenhamLine(-45, -15, -20, -18, 2.5f);
    bresenhamLine(-20, -18, 20, -18, 2.5f);
    bresenhamLine(20, -18, 45, -15, 2.5f);
    bresenhamLine(45, -15, 60, 0, 2.5f);
    bresenhamLine(60, 0, 55, 6, 2.5f);
    bresenhamLine(55, 6, -50, 6, 2.5f);
    bresenhamLine(-50, 6, -55, 0, 2.5f);

    // Hull decorative stripe using DDA
    setColor(0.70f - d, 0.12f, 0.08f);
    ddaLine(-42, -3, 52, -3, 3.0f);
    setColor(0.85f, 0.75f, 0.15f, 0.5f);
    ddaLine(-40, -6, 50, -6, 1.5f);

    // Plank details using DDA
    setColor(0.45f - d, 0.18f - d, 0.06f - d, 0.3f);
    for (int i = 0; i < 6; i++) {
        float lx = -38 + i * 16;
        ddaLine(lx, -16, lx + 1, 5, 1.0f);
    }
    ddaLine(-45, -8, 50, -8, 0.8f);
    ddaLine(-50, 0, 55, 0, 0.8f);

    // Rivets on hull stripe using Midpoint Circle
    setColor(0.55f, 0.48f, 0.30f);
    for (int i = 0; i < 8; i++) {
        float rx = -35 + i * 11;
        midpointCircleFilled(rx, -3, 1.3f);
    }
    setColor(0.42f, 0.35f, 0.22f);
    for (int i = 0; i < 8; i++) {
        float rx = -35 + i * 11;
        midpointCircle(rx, -3, 1.3f, 0.8f);
    }

    // ---- BOW (pointed prow with decoration) ----
    setColor(0.62f - d, 0.48f - d, 0.18f - d);
    glBegin(GL_POLYGON);
    glVertex2f(55, 0); glVertex2f(65, 3); glVertex2f(62, 10); glVertex2f(55, 6);
    glEnd();
    // Bow eye decoration using midpoint circle
    setColor(0.90f, 0.90f, 0.85f);
    midpointCircleFilled(58, 3, 2.5f);
    setColor(0.10f, 0.10f, 0.15f);
    midpointCircleFilled(58, 3, 1.2f);

    // ---- MAST using Bresenham's line algorithm ----
    setColor(0.38f - d, 0.28f - d, 0.15f - d);
    bresenhamLine(-2, 6, -2, 90, 3.0f);
    bresenhamLine(2, 6, 2, 90, 3.0f);
    // Mast cap using Midpoint Circle
    setColor(0.50f, 0.45f, 0.30f);
    midpointCircleFilled(0, 90, 3);
    midpointCircle(0, 90, 3, 1.5f);

    // Cross-beam (yard) using Bresenham's
    setColor(0.40f - d, 0.30f - d, 0.16f - d);
    bresenhamLine(-5, 75, 40, 70, 2.0f);

    // ---- SAIL using GL_POLYGON with wind effect ----
    glPushMatrix();
    glRotatef(boat.sailAngle, 0, 0, 1);

    // Main sail
    float sailBillow = 8 + windStrength * 5 + sin(globalTime * 0.5f) * 3;
    setColor(0.88f - d * 2, 0.83f - d * 2, 0.72f - d * 2, 0.92f);
    glBegin(GL_POLYGON);
    glVertex2f(3, 12);
    glVertex2f(3, 78);
    glVertex2f(10 + sailBillow * 0.3f, 65);
    glVertex2f(35 + sailBillow, 45);
    glVertex2f(42 + sailBillow * 0.8f, 30);
    glVertex2f(38 + sailBillow * 0.5f, 18);
    glEnd();

    // Sail shading (darker toward bottom)
    setColor(0.80f - d * 2, 0.75f - d * 2, 0.62f - d * 2, 0.3f);
    glBegin(GL_POLYGON);
    glVertex2f(3, 12); glVertex2f(38 + sailBillow * 0.5f, 18);
    glVertex2f(42 + sailBillow * 0.8f, 30); glVertex2f(3, 30);
    glEnd();

    // Sail stitching lines using DDA
    setColor(0.78f - d, 0.73f - d, 0.60f - d, 0.4f);
    ddaLine(3, 28, 38 + sailBillow * 0.6f, 23, 1.0f);
    ddaLine(3, 42, 40 + sailBillow * 0.8f, 35, 1.0f);
    ddaLine(3, 56, 30 + sailBillow * 0.5f, 50, 1.0f);
    ddaLine(3, 68, 15 + sailBillow * 0.2f, 63, 1.0f);

    // Sail patch (repair detail)
    setColor(0.82f - d, 0.78f - d, 0.65f - d, 0.5f);
    glBegin(GL_POLYGON);
    glVertex2f(15, 35); glVertex2f(25, 33);
    glVertex2f(27, 42); glVertex2f(17, 44);
    glEnd();
    setColor(0.70f - d, 0.65f - d, 0.52f - d, 0.4f);
    bresenhamLine(15, 35, 25, 33, 1.0f);
    bresenhamLine(25, 33, 27, 42, 1.0f);
    bresenhamLine(27, 42, 17, 44, 1.0f);
    bresenhamLine(17, 44, 15, 35, 1.0f);

    glPopMatrix();

    // ---- RIGGING (ropes) using Bresenham's ----
    setColor(0.28f, 0.23f, 0.18f, 0.6f);
    bresenhamLine(0, 88, 50, 6, 1.0f);
    bresenhamLine(0, 88, -45, 6, 1.0f);
    bresenhamLine(0, 75, 35, 6, 1.0f);
    // Halyard
    bresenhamLine(0, 90, -10, 6, 0.8f);

    // ---- COVERED CARGO using Midpoint Circle for dome shape ----
    setColor(0.68f - d, 0.52f - d, 0.28f - d);
    drawFilledArc(-22, 6, 22, 0, PI, 20);
    // Dome outline using Midpoint Ellipse
    setColor(0.52f - d, 0.38f - d, 0.18f - d);
    midpointEllipseOutline(-22, 6, 22, 22, 1.5f);

    // Dome rib details using Bresenham's
    setColor(0.55f - d, 0.42f - d, 0.22f - d, 0.5f);
    for (int i = 0; i < 5; i++) {
        float angle1 = (float)i / 4 * PI;
        float angle2 = angle1;
        float bx = -22 + cos(angle2) * 22;
        float by = 6 + sin(angle2) * 22;
        bresenhamLine(-22, 6, bx, by, 1.2f);
    }

    // Rope lashing on dome using DDA
    setColor(0.48f - d, 0.38f - d, 0.22f - d, 0.4f);
    for (int i = 1; i < 4; i++) {
        float t = (float)i / 4;
        float arcY = 6 + sin(t * PI) * 22;
        float arcLeft = -22 + (1 - sin(t * PI)) * 10;
        float arcRight = -22 + 44 - (1 - sin(t * PI)) * 10;
        ddaLine(arcLeft, arcY, arcRight, arcY, 1.0f);
    }

    // Waterline using DDA
    setColor(0.20f, 0.45f, 0.55f, 0.3f);
    ddaLine(-48, -17, 50, -17, 2.0f);

    // ---- RUDDER ----
    setColor(0.40f - d, 0.28f - d, 0.14f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-50, 4); glVertex2f(-55, -5);
    glVertex2f(-58, -12); glVertex2f(-52, -10);
    glVertex2f(-48, -2);
    glEnd();

    glPopMatrix();
}

void drawFishingBoat(Boat& boat) {
    glPushMatrix();
    glTranslatef(boat.x, boat.y, 0);
    glRotatef(boat.rockAngle, 0, 0, 1);
    if (boat.direction == -1) glScalef(-1, 1, 1);
    float d = modeTransition * 0.1f;

    // ---- HULL using GL_POLYGON ----
    setColor(0.48f - d, 0.32f - d, 0.16f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-40, 0);
    glVertex2f(-32, -12);
    glVertex2f(-15, -15);
    glVertex2f(15, -15);
    glVertex2f(32, -12);
    glVertex2f(40, 0);
    glVertex2f(35, 5);
    glVertex2f(-35, 5);
    glEnd();

    // Hull bottom strip
    setColor(0.35f - d, 0.20f - d, 0.08f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-32, -12); glVertex2f(-15, -15);
    glVertex2f(15, -15); glVertex2f(32, -12);
    glVertex2f(28, -10); glVertex2f(-28, -10);
    glEnd();

    // Hull outline using Bresenham's
    setColor(0.30f - d, 0.18f - d, 0.08f - d);
    bresenhamLine(-40, 0, -32, -12, 2.0f);
    bresenhamLine(-32, -12, -15, -15, 2.0f);
    bresenhamLine(-15, -15, 15, -15, 2.0f);
    bresenhamLine(15, -15, 32, -12, 2.0f);
    bresenhamLine(32, -12, 40, 0, 2.0f);
    bresenhamLine(40, 0, 35, 5, 2.0f);
    bresenhamLine(35, 5, -35, 5, 2.0f);
    bresenhamLine(-35, 5, -40, 0, 2.0f);

    // Hull color stripe using DDA
    setColor(0.55f - d, 0.15f, 0.10f);
    ddaLine(-30, -2, 30, -2, 2.5f);
    setColor(0.20f, 0.55f - d, 0.20f);
    ddaLine(-28, -5, 28, -5, 1.5f);

    // Plank detail using DDA
    setColor(0.42f - d, 0.28f - d, 0.13f - d, 0.35f);
    for (int i = 0; i < 5; i++) {
        float lx = -25 + i * 12;
        ddaLine(lx, -14, lx, 4, 1.0f);
    }

    // Rivets using Midpoint Circle
    setColor(0.52f, 0.48f, 0.35f);
    midpointCircleFilled(-28, -2, 1.5f);
    midpointCircleFilled(-14, -2, 1.5f);
    midpointCircleFilled(0, -2, 1.5f);
    midpointCircleFilled(14, -2, 1.5f);
    midpointCircleFilled(28, -2, 1.5f);
    // Rivet outlines
    setColor(0.38f, 0.35f, 0.25f);
    midpointCircle(-28, -2, 1.5f, 0.8f);
    midpointCircle(-14, -2, 1.5f, 0.8f);
    midpointCircle(0, -2, 1.5f, 0.8f);
    midpointCircle(14, -2, 1.5f, 0.8f);
    midpointCircle(28, -2, 1.5f, 0.8f);

    // ---- GUNWALE (top rail) using Bresenham's ----
    setColor(0.42f - d, 0.30f - d, 0.15f - d);
    bresenhamLine(-35, 5, 35, 5, 3.0f);

    // ---- FISHING NET hanging over side ----
    setColor(0.40f, 0.55f, 0.38f, 0.5f);
    // Net grid using Bresenham's
    for (int i = 0; i < 10; i++) {
        bresenhamLine(-22 + i * 5, 5, -18 + i * 5, 18, 1.0f);
    }
    for (int i = 0; i < 5; i++) {
        float ny = 7 + i * 3;
        bresenhamLine(-22, ny, 25, ny + 2, 0.8f);
    }
    // Net weights using Midpoint Circle
    setColor(0.35f, 0.35f, 0.40f);
    for (int i = 0; i < 5; i++) {
        float wx = -18 + i * 10;
        midpointCircleFilled(wx, 18, 1.5f);
    }

    // ---- PADDLE with animation ----
    float paddleAngle = sin(globalTime * 2) * 18;
    setColor(0.42f - d, 0.30f - d, 0.15f - d);
    glPushMatrix();
    glTranslatef(12, 5, 0);
    glRotatef(paddleAngle - 45, 0, 0, 1);
    // Paddle shaft using Bresenham's
    bresenhamLine(-1, 0, -1, 42, 2.0f);
    bresenhamLine(1, 0, 1, 42, 2.0f);
    // Paddle blade using GL_POLYGON
    setColor(0.45f - d, 0.33f - d, 0.17f - d);
    glBegin(GL_POLYGON);
    glVertex2f(-6, 42); glVertex2f(6, 42);
    glVertex2f(4, 56); glVertex2f(-4, 56);
    glEnd();
    // Blade outline using Midpoint Ellipse
    setColor(0.38f - d, 0.26f - d, 0.12f - d);
    midpointEllipseOutline(0, 49, 5, 7, 1.2f);
    glPopMatrix();

    // ---- SECOND PADDLE (resting) ----
    setColor(0.40f - d, 0.28f - d, 0.14f - d);
    glPushMatrix();
    glTranslatef(-15, 5, 0);
    glRotatef(-25, 0, 0, 1);
    bresenhamLine(0, 0, 0, 35, 2.0f);
    setColor(0.43f - d, 0.31f - d, 0.16f - d);
    drawEllipse(0, 38, 4, 8, 10);
    glPopMatrix();

    // ---- ROPE COIL using Midpoint Circle ----
    setColor(0.50f - d, 0.40f - d, 0.22f - d);
    midpointCircle(-5, 2, 5, 1.5f);
    midpointCircle(-5, 2, 3, 1.2f);

    // ---- FISH BASKET using GL_POLYGON ----
    setColor(0.55f - d, 0.45f - d, 0.28f - d);
    glBegin(GL_POLYGON);
    glVertex2f(20, 5); glVertex2f(32, 5);
    glVertex2f(30, 15); glVertex2f(22, 15);
    glEnd();
    // Basket weave using DDA
    setColor(0.48f - d, 0.38f - d, 0.22f - d, 0.5f);
    ddaLine(22, 7, 30, 7, 1.0f);
    ddaLine(22, 10, 30, 10, 1.0f);
    ddaLine(22, 13, 30, 13, 1.0f);
    ddaLine(24, 5, 24, 15, 0.8f);
    ddaLine(28, 5, 28, 15, 0.8f);

    // Waterline mark using DDA
    setColor(0.20f, 0.45f, 0.55f, 0.3f);
    ddaLine(-32, -14, 32, -14, 2.0f);

    glPopMatrix();
}

void drawAllBoats() {
    drawCargoBoat(boat1);
    drawSailBoat(boat2);
    drawFishingBoat(boat3);
}

// ============================================================
// DRAWING - ULTRA-REALISTIC PERSONS WITH DETAILED FACIAL FEATURES
// ============================================================

void drawDetailedFace(float headX, float headY, float headR, float d, int dir, Color skinColor, Color hairColor, int personType) {
    // ---- HEAD (base) ----
    setColor(skinColor.r - d, skinColor.g - d, skinColor.b - d);
    drawCircle(headX, headY, headR, 28);

    // Slight jaw definition (make face less perfectly round)
    setColor(skinColor.r - d - 0.01f, skinColor.g - d - 0.01f, skinColor.b - d - 0.01f);
    drawEllipse(headX, headY - headR * 0.2f, headR * 0.7f, headR * 0.5f, 16);

    // ---- EARS (anatomically detailed) ----
    float earX = headR * 0.88f;
    float earH = headR * 0.4f;
    float earW = headR * 0.22f;

    // Outer ear (pinna)
    setColor(skinColor.r - d - 0.02f, skinColor.g - d - 0.02f, skinColor.b - d - 0.02f);
    drawEllipse(headX - earX, headY, earW, earH, 12);
    drawEllipse(headX + earX, headY, earW, earH, 12);

    // Inner ear (concha)
    setColor(skinColor.r - d + 0.04f, skinColor.g - d - 0.04f, skinColor.b - d - 0.05f);
    drawEllipse(headX - earX, headY, earW * 0.5f, earH * 0.55f, 10);
    drawEllipse(headX + earX, headY, earW * 0.5f, earH * 0.55f, 10);

    // Tragus (small bump in front of ear canal)
    setColor(skinColor.r - d, skinColor.g - d, skinColor.b - d);
    drawCircle(headX - earX + earW * 0.3f, headY, earW * 0.2f, 6);
    drawCircle(headX + earX - earW * 0.3f, headY, earW * 0.2f, 6);

    // Ear lobe
    setColor(skinColor.r - d + 0.01f, skinColor.g - d - 0.01f, skinColor.b - d - 0.01f);
    drawCircle(headX - earX, headY - earH * 0.7f, earW * 0.4f, 6);
    drawCircle(headX + earX, headY - earH * 0.7f, earW * 0.4f, 6);

    // ---- HAIR (styled differently per person type) ----
    setColor(hairColor.r, hairColor.g, hairColor.b);
    if (personType == 0) {
        // MAN - neat short hair with parting
        drawFilledArc(headX, headY + headR * 0.15f, headR * 1.08f, 0.25f, PI - 0.25f, 18);
        // Side hair
        drawRect(headX - headR * 0.98f, headY - headR * 0.05f, headR * 0.22f, headR * 0.55f);
        drawRect(headX + headR * 0.76f, headY - headR * 0.05f, headR * 0.22f, headR * 0.55f);
        // Hair texture lines
        setColor(hairColor.r + 0.03f, hairColor.g + 0.02f, hairColor.b + 0.02f, 0.4f);
        for (int i = 0; i < 6; i++) {
            float hx = headX - headR * 0.5f + i * headR * 0.2f;
            float hy = headY + headR * 0.6f;
            drawLine(hx, hy, hx + headR * 0.05f, headY + headR * 1.05f, 1.0f);
        }
    }
    else if (personType == 1) {
        // WOMAN - longer hair with parting and bun
        drawFilledArc(headX, headY + headR * 0.1f, headR * 1.12f, 0.08f, PI - 0.08f, 18);
        // Hair flowing down sides
        setColor(hairColor.r, hairColor.g, hairColor.b);
        glBegin(GL_POLYGON);
        glVertex2f(headX - headR * 1.05f, headY + headR * 0.3f);
        glVertex2f(headX - headR * 0.85f, headY + headR * 0.3f);
        glVertex2f(headX - headR * 0.80f, headY - headR * 0.8f);
        glVertex2f(headX - headR * 1.10f, headY - headR * 0.7f);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex2f(headX + headR * 0.85f, headY + headR * 0.3f);
        glVertex2f(headX + headR * 1.05f, headY + headR * 0.3f);
        glVertex2f(headX + headR * 1.10f, headY - headR * 0.7f);
        glVertex2f(headX + headR * 0.80f, headY - headR * 0.8f);
        glEnd();
        // Bun at back
        drawCircle(headX, headY + headR * 0.85f, headR * 0.42f, 14);
        // Bun detail
        setColor(hairColor.r + 0.02f, hairColor.g + 0.01f, hairColor.b + 0.01f, 0.3f);
        drawCircleOutline(headX, headY + headR * 0.85f, headR * 0.3f, 10);
        // Hair parting
        setColor(skinColor.r - d - 0.05f, skinColor.g - d - 0.05f, skinColor.b - d - 0.05f, 0.3f);
        drawLine(headX, headY + headR * 1.1f, headX + 1, headY + headR * 0.5f, 1.0f);
    }
    else {
        // CHILD - messy playful hair
        drawFilledArc(headX, headY + headR * 0.2f, headR * 1.08f, 0.15f, PI - 0.15f, 16);
        // Messy strands sticking up
        setColor(hairColor.r, hairColor.g, hairColor.b);
        drawLine(headX - 3, headY + headR, headX - 5, headY + headR + 6, 1.5f);
        drawLine(headX, headY + headR * 1.05f, headX + 1, headY + headR + 7, 1.5f);
        drawLine(headX + 3, headY + headR * 0.95f, headX + 6, headY + headR + 5, 1.5f);
        drawLine(headX - 1, headY + headR * 0.98f, headX - 3, headY + headR + 8, 1.2f);
        // Side tufts
        drawLine(headX - headR * 0.7f, headY + headR * 0.5f, headX - headR * 0.9f, headY + headR * 0.6f, 1.5f);
        drawLine(headX + headR * 0.7f, headY + headR * 0.5f, headX + headR * 0.9f, headY + headR * 0.6f, 1.5f);
    }

    // ---- EYES (detailed anatomical rendering) ----
    float eyeSpacing = headR * 0.32f;
    float eyeY = headY + headR * 0.1f;
    float eyeW = headR * 0.22f;
    float eyeH = headR * 0.16f;

    for (int eye = -1; eye <= 1; eye += 2) {
        float ex = headX + eye * eyeSpacing;

        // Eye socket shadow
        setColor(skinColor.r - d - 0.06f, skinColor.g - d - 0.06f, skinColor.b - d - 0.06f, 0.25f);
        drawEllipse(ex, eyeY + 1, eyeW + 1, eyeH + 1.5f, 12);

        // Sclera (eye white)
        setColor(0.96f, 0.96f, 0.95f);
        drawEllipse(ex, eyeY, eyeW, eyeH, 14);

        // Sclera shading (slight red/pink at edges)
        setColor(0.92f, 0.88f, 0.88f, 0.3f);
        drawEllipse(ex - eyeW * 0.5f, eyeY, eyeW * 0.3f, eyeH * 0.7f, 8);
        drawEllipse(ex + eyeW * 0.5f, eyeY, eyeW * 0.3f, eyeH * 0.7f, 8);

        // Iris (colored part)
        float irisR = eyeH * 0.75f;
        float irisOffset = dir * 0.8f;
        setColor(0.28f, 0.16f, 0.06f); // Dark brown iris
        drawCircle(ex + irisOffset, eyeY, irisR, 14);

        // Iris pattern (lighter ring)
        setColor(0.38f, 0.22f, 0.10f, 0.5f);
        drawCircleOutline(ex + irisOffset, eyeY, irisR * 0.7f, 10);

        // Iris edge (limbal ring - darker outer ring)
        setColor(0.12f, 0.08f, 0.03f, 0.6f);
        drawCircleOutline(ex + irisOffset, eyeY, irisR, 12);

        // Pupil
        setColor(0.02f, 0.02f, 0.02f);
        drawCircle(ex + irisOffset, eyeY, irisR * 0.4f, 10);

        // Pupil highlight (specular reflection) - two spots for realism
        setColor(1.0f, 1.0f, 1.0f, 0.85f);
        drawCircle(ex + irisOffset + 1.2f, eyeY + 1.2f, irisR * 0.18f, 6);
        setColor(1.0f, 1.0f, 1.0f, 0.45f);
        drawCircle(ex + irisOffset - 0.8f, eyeY - 0.5f, irisR * 0.10f, 5);

        // Upper eyelid line
        setColor(skinColor.r - d - 0.08f, skinColor.g - d - 0.08f, skinColor.b - d - 0.08f, 0.7f);
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j <= 10; j++) {
            float t = (float)j / 10;
            float lx = ex - eyeW + t * eyeW * 2;
            float ly = eyeY + eyeH * cos(t * PI) * 0.3f + eyeH;
            glVertex2f(lx, ly);
        }
        glEnd();

        // Eyelashes (more prominent on women)
        int lashCount = (personType == 1) ? 5 : 3;
        setColor(0.05f, 0.03f, 0.02f, 0.7f);
        for (int j = 0; j < lashCount; j++) {
            float t = 0.15f + (float)j / (lashCount - 1) * 0.7f;
            float lx = ex - eyeW + t * eyeW * 2;
            float ly = eyeY + eyeH;
            float lashLen = (personType == 1) ? 2.5f : 1.5f;
            float lashAngle = PI * 0.5f + (t - 0.5f) * 0.8f;
            drawLine(lx, ly, lx + cos(lashAngle) * lashLen, ly + sin(lashAngle) * lashLen, 1.0f);
        }

        // Lower eyelid (subtle)
        setColor(skinColor.r - d - 0.03f, skinColor.g - d - 0.03f, skinColor.b - d - 0.03f, 0.3f);
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j <= 8; j++) {
            float t = (float)j / 8;
            float lx = ex - eyeW * 0.8f + t * eyeW * 1.6f;
            float ly = eyeY - eyeH * 0.85f - sin(t * PI) * eyeH * 0.15f;
            glVertex2f(lx, ly);
        }
        glEnd();
    }

    // ---- EYEBROWS (shaped, not just lines) ----
    float browY = eyeY + eyeH + headR * 0.12f;
    setColor(hairColor.r * 1.3f, hairColor.g * 1.3f, hairColor.b * 1.3f, 0.8f);
    for (int eye = -1; eye <= 1; eye += 2) {
        float bx = headX + eye * eyeSpacing;
        // Eyebrow shape as tapered polygon
        glBegin(GL_POLYGON);
        glVertex2f(bx - eyeW * 1.1f, browY - 0.5f);
        glVertex2f(bx - eyeW * 0.5f, browY + 1.5f);
        glVertex2f(bx + eyeW * 0.3f, browY + 2.0f);
        glVertex2f(bx + eyeW * 1.0f, browY + 0.5f);
        glVertex2f(bx + eyeW * 0.3f, browY + 0.5f);
        glVertex2f(bx - eyeW * 0.5f, browY);
        glEnd();
        // Eyebrow hair texture
        setColor(hairColor.r * 1.1f, hairColor.g * 1.1f, hairColor.b * 1.1f, 0.3f);
        for (int j = 0; j < 4; j++) {
            float hx = bx - eyeW * 0.8f + j * eyeW * 0.5f;
            drawLine(hx, browY, hx + 1, browY + 1.5f, 0.8f);
        }
    }

    // ---- NOSE (3D shaped with bridge, tip, nostrils) ----
    float noseY = headY - headR * 0.08f;
    float noseBridgeTop = eyeY - eyeH * 0.8f;

    // Nose bridge shadow (left side)
    setColor(skinColor.r - d - 0.05f, skinColor.g - d - 0.05f, skinColor.b - d - 0.05f, 0.25f);
    glBegin(GL_POLYGON);
    glVertex2f(headX - 1.5f, noseBridgeTop);
    glVertex2f(headX + 0.5f, noseBridgeTop);
    glVertex2f(headX + 0.5f, noseY);
    glVertex2f(headX - 2, noseY);
    glEnd();

    // Nose bridge highlight (right side)
    setColor(skinColor.r - d + 0.03f, skinColor.g - d + 0.02f, skinColor.b - d + 0.01f, 0.3f);
    glBegin(GL_POLYGON);
    glVertex2f(headX + 0.5f, noseBridgeTop);
    glVertex2f(headX + 2, noseBridgeTop);
    glVertex2f(headX + 2.5f, noseY);
    glVertex2f(headX + 0.5f, noseY);
    glEnd();

    // Nose tip (bulbous shape)
    setColor(skinColor.r - d + 0.02f, skinColor.g - d - 0.01f, skinColor.b - d - 0.02f);
    drawEllipse(headX, noseY - 1, 3.0f, 2.0f, 10);

    // Nostrils
    setColor(skinColor.r - d - 0.10f, skinColor.g - d - 0.10f, skinColor.b - d - 0.10f, 0.6f);
    drawEllipse(headX - 2.2f, noseY - 2, 1.5f, 1.0f, 8);
    drawEllipse(headX + 2.2f, noseY - 2, 1.5f, 1.0f, 8);

    // Nostril highlight
    setColor(skinColor.r - d, skinColor.g - d, skinColor.b - d, 0.4f);
    drawCircle(headX - 2, noseY - 1.5f, 0.8f, 5);
    drawCircle(headX + 2, noseY - 1.5f, 0.8f, 5);

    // ---- MOUTH (detailed lips with cupid's bow) ----
    float mouthY = headY - headR * 0.38f;
    float mouthW = headR * 0.35f;

    // Mouth shadow below lower lip
    setColor(skinColor.r - d - 0.06f, skinColor.g - d - 0.06f, skinColor.b - d - 0.06f, 0.2f);
    drawEllipse(headX, mouthY - headR * 0.08f, mouthW * 0.8f, headR * 0.04f, 10);

    // Upper lip (with cupid's bow shape)
    setColor(skinColor.r - d + 0.10f, skinColor.g - d - 0.05f, skinColor.b - d - 0.05f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(headX, mouthY + 0.5f); // center
    for (int i = 0; i <= 16; i++) {
        float t = (float)i / 16;
        float mx = headX - mouthW + t * mouthW * 2;
        float cupidBow = 0;
        if (t > 0.3f && t < 0.5f) cupidBow = sin((t - 0.3f) / 0.2f * PI) * 1.5f;
        else if (t > 0.5f && t < 0.7f) cupidBow = sin((t - 0.5f) / 0.2f * PI) * 1.5f;
        float upper = mouthY + sin(t * PI) * 1.8f + cupidBow;
        glVertex2f(mx, upper);
    }
    glEnd();

    // Lower lip (fuller)
    setColor(skinColor.r - d + 0.08f, skinColor.g - d - 0.06f, skinColor.b - d - 0.06f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(headX, mouthY - 0.5f);
    for (int i = 0; i <= 14; i++) {
        float t = (float)i / 14;
        float mx = headX - mouthW * 0.9f + t * mouthW * 1.8f;
        float lower = mouthY - sin(t * PI) * 1.5f;
        glVertex2f(mx, lower);
    }
    glEnd();

    // Lip highlight (specular on lower lip)
    setColor(skinColor.r - d + 0.15f, skinColor.g - d + 0.02f, skinColor.b - d, 0.3f);
    drawEllipse(headX + 1, mouthY - 0.8f, mouthW * 0.3f, 0.8f, 8);

    // Mouth line (the seam between lips)
    setColor(skinColor.r - d - 0.12f, skinColor.g - d - 0.12f, skinColor.b - d - 0.12f, 0.6f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 12; i++) {
        float t = (float)i / 12;
        float mx = headX - mouthW + t * mouthW * 2;
        float my = mouthY + sin(t * PI) * 0.6f;
        // Slight smile curve
        my += (t - 0.5f) * (t - 0.5f) * 2;
        glVertex2f(mx, my);
    }
    glEnd();

    // Mouth corner dimples
    setColor(skinColor.r - d - 0.04f, skinColor.g - d - 0.04f, skinColor.b - d - 0.04f, 0.2f);
    drawCircle(headX - mouthW - 1, mouthY + 0.5f, 1, 6);
    drawCircle(headX + mouthW + 1, mouthY + 0.5f, 1, 6);

    // ---- BINDI for women ----
    if (personType == 1) {
        setColor(0.88f, 0.08f, 0.08f);
        drawCircle(headX, eyeY + eyeH + headR * 0.28f, 1.8f, 10);
        // Bindi highlight
        setColor(1.0f, 0.3f, 0.3f, 0.4f);
        drawCircle(headX - 0.3f, eyeY + eyeH + headR * 0.28f + 0.3f, 0.8f, 6);
    }

    // ---- CHEEK BLUSH (natural warmth) ----
    setColor(skinColor.r - d + 0.08f, skinColor.g - d - 0.07f, skinColor.b - d - 0.06f, 0.18f);
    drawCircle(headX - eyeSpacing - 2, eyeY - headR * 0.18f, headR * 0.20f, 12);
    drawCircle(headX + eyeSpacing + 2, eyeY - headR * 0.18f, headR * 0.20f, 12);

    // ---- CHIN DEFINITION ----
    setColor(skinColor.r - d - 0.03f, skinColor.g - d - 0.03f, skinColor.b - d - 0.03f, 0.15f);
    drawEllipse(headX, headY - headR * 0.7f, headR * 0.25f, headR * 0.1f, 10);

    // ---- FOREHEAD HIGHLIGHT ----
    setColor(skinColor.r - d + 0.04f, skinColor.g - d + 0.03f, skinColor.b - d + 0.02f, 0.15f);
    drawEllipse(headX, headY + headR * 0.55f, headR * 0.4f, headR * 0.15f, 12);
}

void drawRealisticPerson(Person& p) {
    float d = modeTransition * 0.1f;
    float s = p.scale;

    glPushMatrix();
    glTranslatef(p.x, p.y, 0);
    glScalef(s * (p.direction == -1 ? -1 : 1), s, 1);

    float headR = (p.type == 2) ? 6 : 8;
    float bodyH = (p.type == 2) ? 20 : 30;
    float legH = (p.type == 2) ? 18 : 30;

    if (p.type == 0) {
        // MAN
        float legSwing = p.isWalking ? sin(p.walkPhase) * 15 : 0;
        // Legs
        setColor(p.pantsColor.r - d, p.pantsColor.g - d, p.pantsColor.b - d);
        glPushMatrix(); glTranslatef(-4, 0, 0); glRotatef(legSwing, 0, 0, 1);
        drawRect(-3, -legH, 6, legH);
        setColor(0.2f - d, 0.15f - d, 0.1f - d);
        drawRect(-4, -legH - 3, 8, 4);
        glPopMatrix();
        glPushMatrix(); glTranslatef(4, 0, 0); glRotatef(-legSwing, 0, 0, 1);
        setColor(p.pantsColor.r - d, p.pantsColor.g - d, p.pantsColor.b - d);
        drawRect(-3, -legH, 6, legH);
        setColor(0.2f - d, 0.15f - d, 0.1f - d);
        drawRect(-4, -legH - 3, 8, 4);
        glPopMatrix();

        // Body (torso)
        setColor(p.shirtColor.r - d, p.shirtColor.g - d, p.shirtColor.b - d);
        glBegin(GL_POLYGON);
        glVertex2f(-9, 0); glVertex2f(9, 0);
        glVertex2f(10, bodyH); glVertex2f(-10, bodyH);
        glEnd();
        setColor(p.shirtColor.r - d + 0.1f, p.shirtColor.g - d + 0.1f, p.shirtColor.b - d + 0.1f);
        drawTriangle(-5, bodyH, 5, bodyH, 0, bodyH - 5);

        // Arms
        float armSwing = p.isWalking ? sin(p.walkPhase + PI) * 20 : 0;
        setColor(p.skinColor.r - d, p.skinColor.g - d, p.skinColor.b - d);
        glPushMatrix(); glTranslatef(-11, 25, 0); glRotatef(armSwing, 0, 0, 1);
        drawRect(-2, -22, 5, 22);
        setColor(p.skinColor.r - d + 0.02f, p.skinColor.g - d, p.skinColor.b - d);
        drawCircle(0, -22, 3, 8);
        glPopMatrix();
        glPushMatrix(); glTranslatef(11, 25, 0); glRotatef(-armSwing, 0, 0, 1);
        setColor(p.skinColor.r - d, p.skinColor.g - d, p.skinColor.b - d);
        drawRect(-2, -22, 5, 22);
        setColor(p.skinColor.r - d + 0.02f, p.skinColor.g - d, p.skinColor.b - d);
        drawCircle(0, -22, 3, 8);
        glPopMatrix();

        // Neck
        setColor(p.skinColor.r - d - 0.02f, p.skinColor.g - d - 0.02f, p.skinColor.b - d - 0.02f);
        drawRect(-3, bodyH, 6, 5);

        drawDetailedFace(0, bodyH + 5 + headR, headR, d, p.direction, p.skinColor, p.hairColor, 0);
    }
    else if (p.type == 1) {
        // WOMAN
        setColor(p.shirtColor.r - d, p.shirtColor.g - d, p.shirtColor.b - d);
        glBegin(GL_POLYGON);
        glVertex2f(-12, -5); glVertex2f(12, -5);
        glVertex2f(14, bodyH); glVertex2f(-14, bodyH);
        glEnd();
        setColor(p.shirtColor.r - d + 0.1f, p.shirtColor.g - d - 0.05f, p.shirtColor.b - d);
        glBegin(GL_POLYGON);
        glVertex2f(-10, bodyH); glVertex2f(5, bodyH);
        glVertex2f(-2, bodyH + 8);
        glEnd();
        setColor(0.9f, 0.8f, 0.2f, 0.6f);
        drawRect(-12, -5, 26, 2);
        drawRect(-12, bodyH - 2, 26, 2);

        // Arms
        setColor(p.skinColor.r - d, p.skinColor.g - d, p.skinColor.b - d);
        drawRect(-15, 18, 4, 16);
        drawRect(11, 18, 4, 16);
        drawCircle(-13, 17, 2.5f, 8);
        drawCircle(13, 17, 2.5f, 8);
        setColor(0.9f, 0.7f, 0.2f, 0.8f);
        drawCircleOutline(-13, 20, 3, 10);
        drawCircleOutline(13, 20, 3, 10);

        setColor(p.skinColor.r - d - 0.02f, p.skinColor.g - d - 0.02f, p.skinColor.b - d - 0.02f);
        drawRect(-3, bodyH, 6, 4);
        setColor(0.9f, 0.75f, 0.15f);
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= 10; i++) {
            float t = (float)i / 10;
            float nx = -4 + t * 8;
            float ny = bodyH + 2 - sin(t * PI) * 2;
            glVertex2f(nx, ny);
        }
        glEnd();

        drawDetailedFace(0, bodyH + 4 + headR * 0.85f, headR * 0.85f, d, p.direction, p.skinColor, p.hairColor, 1);

        if (&p == &persons[4]) {
            setColor(p.shirtColor.r - d, p.shirtColor.g - d + 0.1f, p.shirtColor.b - d);
            float hY = bodyH + 4 + headR * 0.85f;
            drawFilledArc(0, hY + headR * 0.2f, headR * 1.15f, 0.15f, PI - 0.15f, 16);
            glBegin(GL_POLYGON);
            glVertex2f(-headR * 0.9f, hY - headR * 0.3f);
            glVertex2f(-headR * 1.15f, hY - headR * 1.3f);
            glVertex2f(-headR * 0.5f, hY - headR * 0.3f);
            glEnd();
        }
    }
    else {
        // CHILD
        float legSwing = p.isWalking ? sin(p.walkPhase) * 12 : 0;
        setColor(p.pantsColor.r - d, p.pantsColor.g - d, p.pantsColor.b - d);
        glPushMatrix(); glTranslatef(-3, 0, 0); glRotatef(legSwing, 0, 0, 1);
        drawRect(-2, -legH, 4, legH);
        setColor(0.25f - d, 0.18f - d, 0.12f - d);
        drawRect(-3, -legH - 2, 6, 3);
        glPopMatrix();
        glPushMatrix(); glTranslatef(3, 0, 0); glRotatef(-legSwing, 0, 0, 1);
        setColor(p.pantsColor.r - d, p.pantsColor.g - d, p.pantsColor.b - d);
        drawRect(-2, -legH, 4, legH);
        setColor(0.25f - d, 0.18f - d, 0.12f - d);
        drawRect(-3, -legH - 2, 6, 3);
        glPopMatrix();

        setColor(p.shirtColor.r - d, p.shirtColor.g - d, p.shirtColor.b - d);
        glBegin(GL_POLYGON);
        glVertex2f(-7, 0); glVertex2f(7, 0);
        glVertex2f(8, bodyH); glVertex2f(-8, bodyH);
        glEnd();

        setColor(p.skinColor.r - d, p.skinColor.g - d, p.skinColor.b - d);
        drawRect(-10, 12, 3, 14);
        drawRect(7, 12, 3, 14);
        drawCircle(-8.5f, 11, 2, 6);
        drawCircle(8.5f, 11, 2, 6);

        setColor(p.skinColor.r - d - 0.02f, p.skinColor.g - d - 0.02f, p.skinColor.b - d - 0.02f);
        drawRect(-2, bodyH, 4, 3);

        drawDetailedFace(0, bodyH + 3 + headR, headR, d, p.direction, p.skinColor, p.hairColor, 2);
    }

    // Umbrella in rain
    if (isRainyMode && modeTransition > 0.5f && p.type != 2) {
        float umbY = (p.type == 2) ? bodyH + 3 + headR * 2 + 10 : bodyH + 5 + headR * 2 + 10;
        setColor(0.1f, 0.1f, 0.55f, 0.85f);
        drawFilledArc(0, umbY, 22, 0, PI, 16);
        setColor(0.08f, 0.08f, 0.4f, 0.5f);
        for (int i = 0; i < 6; i++) {
            float a = (float)i / 5 * PI;
            drawLine(0, umbY, cos(a) * 22, umbY + sin(a) * 22, 1.0f);
        }
        setColor(0.3f, 0.25f, 0.2f);
        drawLine(0, bodyH + 5, 0, umbY, 2.0f);
    }

    glPopMatrix();
}

void drawAllPersons() {
    for (int i = 0; i < 10; i++) {
        if (persons[i].x <= -500) continue;
        // Depth-perspective scale around each person's position
        float ds = getRoadDepthScale(persons[i].x);
        glPushMatrix();
        glTranslatef(persons[i].x, persons[i].y, 0.0f);
        glScalef(ds, ds, 1.0f);
        glTranslatef(-persons[i].x, -persons[i].y, 0.0f);
        drawRealisticPerson(persons[i]);
        glPopMatrix();
    }
}

// ============================================================
// DRAWING - KITE
// ============================================================

void drawKite() {
    float d = modeTransition * 0.1f;
    setColor(0.3f, 0.3f, 0.3f, 0.7f);
    glBegin(GL_LINE_STRIP);
    int segs = 20;
    for (int i = 0; i <= segs; i++) {
        float t = (float)i / segs;
        float sx = lerp(kite.ownerX, kite.x, t);
        float sy = lerp(kite.ownerY, kite.y, t);
        sx += sin(t * PI * 4 + globalTime * 3) * 5 * (1 - t);
        glVertex2f(sx, sy);
    }
    glEnd();

    glPushMatrix(); glTranslatef(kite.x, kite.y, 0);
    glRotatef(sin(kite.swayPhase) * 15, 0, 0, 1);
    float kw = 20, kh = 25;
    setColor(kite.color1.r - d, kite.color1.g - d, kite.color1.b - d);
    drawTriangle(-kw, 0, 0, kh, 0, -kh * 0.5f);
    setColor(kite.color2.r - d, kite.color2.g - d, kite.color2.b - d);
    drawTriangle(kw, 0, 0, kh, 0, -kh * 0.5f);
    setColor(0.3f, 0.25f, 0.15f);
    drawLine(-kw, 0, kw, 0, 1.5f); drawLine(0, -kh * 0.5f, 0, kh, 1.5f);

    setColor(0.9f, 0.2f, 0.2f, 0.8f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 15; i++) {
        float t = (float)i / 15;
        float tx = sin(t * PI * 3 + globalTime * 5) * 10 * t;
        float ty = -kh * 0.5f - t * 40;
        glVertex2f(tx, ty);
    }
    glEnd();
    for (int i = 0; i < 4; i++) {
        float t = (float)(i + 1) / 5;
        float tx = sin(t * PI * 3 + globalTime * 5) * 10 * t;
        float ty = -kh * 0.5f - t * 40;
        Color bowColor = (i % 2 == 0) ? Color(0.9f, 0.8f, 0.1f) : Color(0.2f, 0.7f, 0.3f);
        setColor(bowColor.r, bowColor.g, bowColor.b, 0.8f);
        drawCircle(tx, ty, 3, 6);
    }
    glPopMatrix();
}

// ============================================================
// DRAWING - CHICKENS
// ============================================================

void drawChicken(Chicken& ch) {
    if (!ch.active) return;
    float d = modeTransition * 0.1f;
    glPushMatrix();
    glTranslatef(ch.x, ch.y, 0);
    glScalef(ch.direction == -1 ? -1 : 1, 1, 1);
    float peckAngle = ch.isPecking ? sin(ch.peckPhase) * 30 : 0;
    float walkBob = !ch.isPecking ? sin(ch.walkPhase) * 2 : 0;
    float legSwing = !ch.isPecking ? sin(ch.walkPhase) * 15 : 0;

    setColor(0.85f - d, 0.65f - d, 0.15f - d);
    glPushMatrix(); glTranslatef(-3, 0, 0); glRotatef(legSwing, 0, 0, 1);
    drawLine(0, 0, 0, -8, 1.5f); drawLine(-2, -8, 2, -8, 1);
    glPopMatrix();
    glPushMatrix(); glTranslatef(3, 0, 0); glRotatef(-legSwing, 0, 0, 1);
    drawLine(0, 0, 0, -8, 1.5f); drawLine(-2, -8, 2, -8, 1);
    glPopMatrix();

    glTranslatef(0, walkBob, 0);
    setColor(0.85f - d, 0.78f - d, 0.65f - d);
    drawEllipse(0, 5, 10, 7, 16);
    setColor(0.80f - d, 0.72f - d, 0.58f - d);
    drawEllipse(3, 7, 7, 5, 12);
    setColor(0.75f - d, 0.68f - d, 0.50f - d);
    drawTriangle(-10, 5, -15, 12, -8, 10);

    glPushMatrix(); glTranslatef(7, 10, 0); glRotatef(-peckAngle, 0, 0, 1);
    setColor(0.85f - d, 0.78f - d, 0.65f - d);
    drawRect(-2, 0, 4, 8);
    drawCircle(0, 10, 4, 12);
    setColor(0.90f, 0.15f, 0.10f);
    drawCircle(0, 14, 2.5f, 8); drawCircle(-2, 13, 2, 8);
    setColor(0.90f, 0.75f, 0.15f);
    drawTriangle(3, 10, 8, 9, 3, 8);
    setColor(0, 0, 0); drawCircle(2, 11, 1, 6);
    setColor(0.90f, 0.15f, 0.10f); drawEllipse(2, 7, 2, 3, 6);
    glPopMatrix();
    glPopMatrix();
}

void drawAllChickens() { for (int i = 0; i < 5; i++) drawChicken(chickens[i]); }

// ============================================================
// DRAWING - BIRDS
// ============================================================

void drawBird(Bird& b) {
    if (!b.active) return;
    float d = modeTransition * 0.15f;
    glPushMatrix();
    glTranslatef(b.x, b.y, 0);
    glScalef(b.scale * (b.direction == -1 ? -1 : 1), b.scale, 1);
    setColor(0.15f - d * 0.5f, 0.12f - d * 0.5f, 0.10f - d * 0.5f, 0.8f);
    drawEllipse(0, 0, 8, 3, 12);
    float wingA = b.wingAngle;
    glPushMatrix(); glRotatef(wingA, 1, 0, 0);
    setColor(0.12f - d * 0.5f, 0.10f - d * 0.5f, 0.08f - d * 0.5f, 0.8f);
    glBegin(GL_TRIANGLES); glVertex2f(-3, 0); glVertex2f(-18, 8 + wingA * 0.2f); glVertex2f(-8, 0); glEnd();
    glPopMatrix();
    glPushMatrix(); glRotatef(-wingA, 1, 0, 0);
    glBegin(GL_TRIANGLES); glVertex2f(3, 0); glVertex2f(18, 8 + wingA * 0.2f); glVertex2f(8, 0); glEnd();
    glPopMatrix();
    setColor(0.12f, 0.10f, 0.08f, 0.8f);
    drawCircle(8, 1, 2.5f, 8);
    glPopMatrix();
}

void drawAllBirds() { for (int i = 0; i < MAX_BIRDS; i++) drawBird(birds[i]); }

// ============================================================
// DRAWING - FISH
// ============================================================

void drawAllFish() {
    for (int i = 0; i < MAX_FISH; i++) {
        if (!fishes[i].active) continue;
        glPushMatrix();
        glTranslatef(fishes[i].x, fishes[i].y, 0);
        glScalef(fishes[i].scale * (fishes[i].direction == -1 ? -1 : 1), fishes[i].scale, 1);
        setColor(fishes[i].color.r * 0.8f, fishes[i].color.g * 0.8f, fishes[i].color.b * 0.8f, 0.6f);
        drawEllipse(0, 0, 12, 5, 16);
        glPushMatrix(); glTranslatef(-12, 0, 0); glRotatef(fishes[i].tailAngle, 0, 0, 1);
        setColor(fishes[i].color.r * 0.7f, fishes[i].color.g * 0.7f, fishes[i].color.b * 0.7f, 0.6f);
        drawTriangle(0, 0, -10, 6, -10, -6);
        glPopMatrix();
        setColor(fishes[i].color.r * 0.6f, fishes[i].color.g * 0.6f, fishes[i].color.b * 0.6f, 0.5f);
        drawTriangle(-2, 5, 5, 5, 0, 10);
        setColor(0, 0, 0, 0.5f); drawCircle(7, 1, 1.5f, 6);
        glPopMatrix();
    }
}

// ============================================================
// DRAWING - BUTTERFLIES, DRAGONFLIES, FIREFLIES
// ============================================================

void drawAllButterflies() {
    for (int i = 0; i < MAX_BUTTERFLIES; i++) {
        if (!butterflies[i].active) continue;
        glPushMatrix(); glTranslatef(butterflies[i].x, butterflies[i].y, 0);
        float wingScale = cos(butterflies[i].wingAngle * DEG_TO_RAD);
        setColor(0.15f, 0.10f, 0.08f); drawEllipse(0, 0, 2, 6, 8);
        setColor(butterflies[i].color1.r, butterflies[i].color1.g, butterflies[i].color1.b, 0.8f);
        drawEllipse(-6 * wingScale, 2, 6 * fabs(wingScale), 8, 12);
        drawEllipse(6 * wingScale, 2, 6 * fabs(wingScale), 8, 12);
        setColor(butterflies[i].color2.r, butterflies[i].color2.g, butterflies[i].color2.b, 0.6f);
        drawEllipse(-5 * wingScale, -2, 4 * fabs(wingScale), 5, 10);
        drawEllipse(5 * wingScale, -2, 4 * fabs(wingScale), 5, 10);
        setColor(0.15f, 0.10f, 0.08f);
        drawLine(0, 6, -4, 12, 1); drawLine(0, 6, 4, 12, 1);
        glPopMatrix();
    }
}

void drawAllDragonflies() {
    for (int i = 0; i < MAX_DRAGONFLIES; i++) {
        if (!dragonflies[i].active) continue;
        glPushMatrix(); glTranslatef(dragonflies[i].x, dragonflies[i].y, 0);
        glScalef(dragonflies[i].scale, dragonflies[i].scale, 1);
        setColor(0.1f, 0.3f, 0.5f, 0.7f); drawEllipse(0, 0, 2, 15, 10);
        float wingA = dragonflies[i].wingAngle;
        setColor(0.7f, 0.8f, 0.9f, 0.4f);
        drawEllipse(-8 * cos(wingA * DEG_TO_RAD), 5, 12, 3, 10);
        drawEllipse(8 * cos(wingA * DEG_TO_RAD), 5, 12, 3, 10);
        drawEllipse(-7 * cos((wingA + 30) * DEG_TO_RAD), 0, 10, 2.5f, 10);
        drawEllipse(7 * cos((wingA + 30) * DEG_TO_RAD), 0, 10, 2.5f, 10);
        drawCircle(0, 15, 3, 8);
        setColor(0.8f, 0.2f, 0.1f, 0.6f);
        drawCircle(-2, 16, 1.5f, 6); drawCircle(2, 16, 1.5f, 6);
        glPopMatrix();
    }
}

void drawFireflies() {
    for (int i = 0; i < MAX_FIREFLIES; i++) {
        if (!fireflies[i].active) continue;
        float b = fireflies[i].brightness;
        setColor(0.9f, 0.95f, 0.3f, b * 0.2f);
        drawCircle(fireflies[i].x, fireflies[i].y, 8, 10);
        setColor(0.95f, 1.0f, 0.5f, b);
        drawCircle(fireflies[i].x, fireflies[i].y, 2, 8);
    }
}

// ============================================================
// DRAWING - WEATHER EFFECTS
// ============================================================

void drawRain() {
    if (rainIntensity < 0.05f) return;
    glLineWidth(1.5f);
    for (int i = 0; i < MAX_RAIN_DROPS; i++) {
        if (!rainDrops[i].active) continue;
        setColor(0.7f, 0.75f, 0.85f, rainDrops[i].alpha);
        float nx = rainDrops[i].speedX * 0.3f, ny = rainDrops[i].speedY * 0.3f;
        float len = rainDrops[i].length;
        float factor = len / sqrt(nx * nx + ny * ny);
        glBegin(GL_LINES);
        glVertex2f(rainDrops[i].x, rainDrops[i].y);
        glVertex2f(rainDrops[i].x + nx * factor, rainDrops[i].y + ny * factor);
        glEnd();
    }
    glLineWidth(1.0f);
}

void drawLightningEffect() {
    for (int i = 0; i < MAX_LIGHTNING; i++) {
        if (!lightnings[i].active) continue;
        float b = lightnings[i].brightness;
        setColor(0.8f, 0.85f, 1.0f, b * 0.3f);
        for (size_t j = 0; j < lightnings[i].segments.size() - 1; j++) {
            Vec2 a = lightnings[i].segments[j], bb = lightnings[i].segments[j + 1];
            drawThickLine(a.x, a.y, bb.x, bb.y, 8);
        }
        setColor(0.95f, 0.95f, 1.0f, b);
        glLineWidth(3); glBegin(GL_LINE_STRIP);
        for (size_t j = 0; j < lightnings[i].segments.size(); j++)
            glVertex2f(lightnings[i].segments[j].x, lightnings[i].segments[j].y);
        glEnd(); glLineWidth(1);
    }
}

void drawPuddlesEffect() {
    for (int i = 0; i < MAX_PUDDLES; i++) {
        if (!puddles[i].active) continue;
        setColor(0.3f, 0.4f, 0.5f, 0.4f);
        drawEllipse(puddles[i].x, puddles[i].y, puddles[i].width, puddles[i].height, 16);
        setColor(0.5f, 0.6f, 0.7f, 0.2f);
        drawEllipse(puddles[i].x - puddles[i].width * 0.2f, puddles[i].y + puddles[i].height * 0.2f,
            puddles[i].width * 0.5f, puddles[i].height * 0.3f, 12);
        if (rainIntensity > 0.3f) {
            float rippleSize = 3 + sin(puddles[i].ripplePhase) * 2;
            setColor(0.6f, 0.7f, 0.8f, 0.3f);
            drawCircleOutline(puddles[i].x + sin(puddles[i].ripplePhase * 2) * 5, puddles[i].y, rippleSize, 10);
        }
    }
}

void drawSmoke() {
    for (int i = 0; i < MAX_SMOKE_PARTICLES; i++) {
        if (!smokeParticles[i].active) continue;
        setColor(0.6f, 0.6f, 0.65f, smokeParticles[i].alpha);
        drawCircle(smokeParticles[i].x, smokeParticles[i].y, smokeParticles[i].size, 12);
        setColor(0.65f, 0.65f, 0.70f, smokeParticles[i].alpha * 0.5f);
        drawCircle(smokeParticles[i].x + smokeParticles[i].size * 0.3f,
            smokeParticles[i].y + smokeParticles[i].size * 0.2f, smokeParticles[i].size * 0.7f, 10);
    }
}

void drawLeaves() {
    for (int i = 0; i < MAX_LEAVES; i++) {
        if (!leaves[i].active) continue;
        glPushMatrix(); glTranslatef(leaves[i].x, leaves[i].y, 0);
        glRotatef(leaves[i].rotation, 0, 0, 1); glScalef(leaves[i].scale, leaves[i].scale, 1);
        setColor(leaves[i].color.r, leaves[i].color.g, leaves[i].color.b, 0.8f);
        drawEllipse(0, 0, 5, 3, 8);
        setColor(leaves[i].color.r * 0.8f, leaves[i].color.g * 0.8f, leaves[i].color.b * 0.8f, 0.5f);
        drawLine(-4, 0, 4, 0, 1); glPopMatrix();
    }
}

void drawFog() {
    if (modeTransition > 0.3f) {
        float fogAlpha = modeTransition * 0.12f;
        for (int i = 0; i < 10; i++) {
            float fx = i * (worldRight / 10.0f);
            float fy = waterLevel + 20 + sin(globalTime * 0.5f + i) * 10;
            setColor(0.4f, 0.42f, 0.48f, fogAlpha * (0.5f + 0.5f * sin(globalTime * 0.3f + i)));
            drawEllipse(fx, fy, 250 + sin(i * 0.7f) * 50, 40 + sin(i * 0.5f) * 10, 16);
        }
    }
}

// ============================================================
// DRAWING - REFLECTIONS
// ============================================================

void drawWaterReflections() {
    float alpha = 0.15f * (1.0f - modeTransition * 0.5f);
    if (alpha < 0.01f) return;
    Color skyRefl = lerpColor(Color(0.98f, 0.65f, 0.20f, alpha), Color(0.15f, 0.15f, 0.22f, alpha * 0.5f), modeTransition);
    // Apply wave shear to make sky reflection shimmer
    glPushMatrix();
    float wShear = sin(waterWavePhase * 1.1f) * 0.014f + sin(waterWavePhase * 0.7f) * 0.007f;
    applyShearMatrix(wShear, 0.0f);
    glBegin(GL_QUADS);
    glColor4f(skyRefl.r, skyRefl.g, skyRefl.b, skyRefl.a);
    glVertex2f(0, waterLevel); glVertex2f(worldRight, waterLevel);
    glColor4f(skyRefl.r * 0.5f, skyRefl.g * 0.5f, skyRefl.b * 0.5f, skyRefl.a * 0.3f);
    glVertex2f(worldRight, waterLevel - 100); glVertex2f(0, waterLevel - 100);
    glEnd();
    glPopMatrix();
}

void drawBoatWake(float x, float y, int dir, float speed) {
    if (speed < 0.1f) return;
    float wakeAlpha = 0.15f * clamp(speed, 0, 1);
    for (int i = 0; i < 12; i++) {
        float t = (float)i / 12;
        float dist = t * 70, spread = t * 25;
        float wave = sin(waterWavePhase + i * 0.5f) * 2;
        float alpha = wakeAlpha * (1 - t);
        setColor(0.7f, 0.8f, 0.95f, alpha);
        drawCircle(x - dir * dist + wave, y + spread + wave, 2, 6);
        drawCircle(x - dir * dist + wave, y - spread + wave, 2, 6);
    }
}

// ============================================================
// DRAWING - SCENERY DETAILS
// ============================================================

void drawFence(float x, float y, float length, float height) {
    float d = modeTransition * 0.1f;
    int posts = (int)(length / 30);
    for (int i = 0; i <= posts; i++) {
        float px = x + i * 30;
        setColor(0.45f - d, 0.35f - d, 0.20f - d);
        drawRect(px - 2, y, 4, height);
        setColor(0.50f - d, 0.40f - d, 0.25f - d);
        drawTriangle(px - 3, y + height, px + 3, y + height, px, y + height + 8);
    }
    setColor(0.42f - d, 0.32f - d, 0.18f - d);
    drawRect(x, y + height * 0.3f, length, 3);
    drawRect(x, y + height * 0.7f, length, 3);
}

void drawLilyPads() {
    float d = modeTransition * 0.05f;
    float lp[][2] = { {200, waterLevel - 15}, {350, waterLevel - 25}, {550, waterLevel - 20}, {1100, waterLevel - 30}, {1400, waterLevel - 18} };
    for (int i = 0; i < 5; i++) {
        float lx = lp[i][0], ly = lp[i][1] + sin(waterWavePhase + i * 2) * 2;
        setColor(0.15f - d, 0.55f - d, 0.12f - d, 0.7f);
        drawEllipse(lx, ly, 12, 8, 16);
        if (i % 2 == 0 && !isRainyMode) {
            setColor(0.95f, 0.6f, 0.7f, 0.8f);
            for (int p = 0; p < 5; p++) {
                float pa = p * 72 * DEG_TO_RAD;
                drawEllipse(lx + cos(pa) * 4, ly + sin(pa) * 3 + 5, 3, 5, 8);
            }
            setColor(0.95f, 0.90f, 0.3f); drawCircle(lx, ly + 5, 2, 8);
        }
    }
}

void drawReeds() {
    float d = modeTransition * 0.08f;
    float reedPos[][2] = { {50, waterLevel + 5}, {350, waterLevel + 8}, {600, waterLevel + 3}, {1200, waterLevel + 6}, {1600, waterLevel + 4} };
    int counts[] = { 6, 4, 5, 4, 5 };
    for (int r = 0; r < 5; r++) {
        float rx = reedPos[r][0], ry = reedPos[r][1];
        for (int i = 0; i < counts[r]; i++) {
            float cx = rx + i * 15 + sin(i * 2.3f) * 5;
            float height = 40 + sin(i * 1.7f) * 10;
            float sway = sin(globalTime * 1.5f + i * 0.8f) * windStrength * 3;
            setColor(0.25f - d, 0.50f - d, 0.15f - d);
            drawLine(cx, ry, cx + sway, ry + height, 1.5f);
            setColor(0.45f - d, 0.30f - d, 0.15f - d);
            drawEllipse(cx + sway, ry + height, 3, 8, 10);
        }
    }
}

// ============================================================
// DRAWING - WHEEL (for rickshaw)
// ============================================================

void drawWheel(float x, float y, float radius, float angle) {
    float d = modeTransition * 0.1f;
    setColor(0.15f - d, 0.15f - d, 0.15f - d); drawCircle(x, y, radius, 20);
    setColor(0.50f, 0.50f, 0.50f); drawCircle(x, y, radius * 0.7f, 16);
    setColor(0.60f, 0.55f, 0.50f); drawCircle(x, y, radius * 0.2f, 10);
    setColor(0.55f, 0.55f, 0.50f);
    for (int i = 0; i < 8; i++) {
        float a = angle + i * PI / 4;
        drawLine(x + cos(a) * radius * 0.2f, y + sin(a) * radius * 0.2f,
            x + cos(a) * radius * 0.65f, y + sin(a) * radius * 0.65f, 1);
    }
}

void drawSingleRickshaw(Rickshaw& rick) {
    float x = rick.x, y = rick.y, d = modeTransition * 0.1f;

    glPushMatrix(); glTranslatef(x, y, 0);
    if (rick.direction == -1) glScalef(-1, 1, 1);
    setColor(0.30f - d, 0.30f - d, 0.30f - d);
    drawLine(-20, 0, 55, 10, 3); drawLine(55, 10, 65, 5, 3);
    setColor(0.40f, 0.40f, 0.40f);
    drawLine(60, 5, 65, 25, 2.5f); drawLine(60, 25, 70, 25, 2);
    glPopMatrix();

    float backWheelX = x + (rick.direction == -1 ? 20 : -20);
    drawWheel(backWheelX, y - 5, 15, rick.wheelAngle);
    float frontWheelX = x + (rick.direction == -1 ? -55 : 55);
    drawWheel(frontWheelX, y - 3, 12, rick.wheelAngle);

    float bounce = rick.bodyBounce;
    glPushMatrix(); glTranslatef(x, y + bounce, 0);
    if (rick.direction == -1) glScalef(-1, 1, 1);
    setColor(rick.bodyColor.r - d, rick.bodyColor.g - d, rick.bodyColor.b - d);
    drawRect(-30, 0, 60, 35);
    setColor(0.9f, 0.15f, 0.1f, 0.7f); drawCircle(-10, 18, 6, 10);
    setColor(0.2f, 0.7f, 0.3f, 0.7f); drawCircle(10, 18, 6, 10);
    setColor(0.9f, 0.85f, 0.2f, 0.7f); drawCircle(0, 8, 5, 10);

    Color canCol = rick.canopyColor;
    setColor(0.35f - d, 0.30f - d, 0.25f - d);
    drawLine(-28, 35, -20, 70, 2); drawLine(28, 35, 35, 70, 2);
    setColor(canCol.r - d, canCol.g - d, canCol.b - d);
    glBegin(GL_POLYGON);
    glVertex2f(-25, 65); glVertex2f(40, 65); glVertex2f(42, 72);
    glVertex2f(38, 78); glVertex2f(-20, 78); glVertex2f(-27, 72);
    glEnd();
    for (int i = 0; i < 12; i++) {
        float fringeX = -24 + i * 5.3f;
        setColor(1.0f, 0.9f, 0.2f, 0.8f);
        drawTriangle(fringeX, 65, fringeX + 4, 65, fringeX + 2, 60);
    }
    setColor(0.40f - d, 0.30f - d, 0.20f - d);
    drawRect(-30, 35, 5, 35);

    float pedalA = rick.pedalAngle;
    float riderX = 50, riderY = 20;
    setColor(0.55f - d, 0.40f - d, 0.28f - d);
    glPushMatrix(); glTranslatef(riderX, riderY, 0); glRotatef(sin(pedalA) * 30, 0, 0, 1);
    drawRect(-2, -25, 5, 25); glPopMatrix();
    glPushMatrix(); glTranslatef(riderX, riderY, 0); glRotatef(sin(pedalA + PI) * 30, 0, 0, 1);
    drawRect(-2, -25, 5, 25); glPopMatrix();
    setColor(0.30f - d, 0.55f - d, 0.70f - d);
    drawRect(riderX - 8, riderY, 16, 30);
    setColor(0.55f - d, 0.40f - d, 0.28f - d);
    drawLine(riderX - 5, riderY + 25, riderX + 15, riderY + 15, 3);
    drawLine(riderX + 5, riderY + 25, riderX + 15, riderY + 15, 3);
    drawDetailedFace(riderX, riderY + 38, 8, d, 1,
        Color(0.60f, 0.45f, 0.30f), Color(0.08f, 0.06f, 0.04f), 0);

    setColor(0.75f - d, 0.25f - d, 0.30f - d);
    drawRect(-10, 35, 20, 25);
    drawDetailedFace(0, 68, 7, d, rick.direction,
        Color(0.62f, 0.47f, 0.32f), Color(0.05f, 0.03f, 0.02f), 1);

    glPopMatrix();
}

void drawAllRickshaws() {
    // Apply depth-perspective scale: entities further left on road look smaller
    float ds1 = getRoadDepthScale(rickshaw1.x);
    glPushMatrix();
    glTranslatef(rickshaw1.x, rickshaw1.y, 0.0f);
    glScalef(ds1, ds1, 1.0f);
    glTranslatef(-rickshaw1.x, -rickshaw1.y, 0.0f);
    drawSingleRickshaw(rickshaw1);
    glPopMatrix();

    float ds2 = getRoadDepthScale(rickshaw2.x);
    glPushMatrix();
    glTranslatef(rickshaw2.x, rickshaw2.y, 0.0f);
    glScalef(ds2, ds2, 1.0f);
    glTranslatef(-rickshaw2.x, -rickshaw2.y, 0.0f);
    drawSingleRickshaw(rickshaw2);
    glPopMatrix();
}

// ============================================================
// DRAWING - UI
// ============================================================

void drawModeIndicator() {
    float x = 20, y = worldTop - 40;
    setColor(0, 0, 0, 0.5f); drawRoundedRect(x, y - 10, 260, 35, 5);
    if (isRainyMode) {
        setColor(0.6f, 0.7f, 0.9f);
        drawText(x + 10, y, "NIGHT/RAIN MODE [R to toggle]", GLUT_BITMAP_HELVETICA_18);
    }
    else {
        setColor(1.0f, 0.9f, 0.5f);
        drawText(x + 10, y, "SUNSET MODE [R to toggle]", GLUT_BITMAP_HELVETICA_18);
    }
}

void drawInstructions() {
    float x = 20, y = 20;
    setColor(0, 0, 0, 0.4f); drawRoundedRect(x, y, 320, 60, 5);
    setColor(1, 1, 1, 0.8f);
    drawText(x + 10, y + 35, "R - Toggle Rain/Night | F - Fullscreen", GLUT_BITMAP_HELVETICA_12);
    drawText(x + 10, y + 15, "ESC - Exit | +/- Wind | Space - Pause", GLUT_BITMAP_HELVETICA_12);
}

// ============================================================
// FINAL DISPLAY FUNCTION
// ============================================================

void displayFinal() {
    Color bgColor = lerpColor(Color(0.96f, 0.45f, 0.15f), Color(0.05f, 0.05f, 0.08f), modeTransition);
    glClearColor(bgColor.r, bgColor.g, bgColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(worldLeft, worldRight, worldBottom, worldTop);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH); glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    // Screen shake: translation transform applied to entire scene (lightning)
    glPushMatrix();
    glTranslatef(screenShakeX, screenShakeY, 0.0f);

    // 1. Sky
    drawSky();
    drawStars();
    drawCrescentMoon();
    drawSun();
    drawAllClouds();

    // 2. Landscape background
    drawGrassland();
    drawDistantTrees();

    // 4. Ground
    drawGrass();
    drawRoad();
    drawAllShadows();    // sun-angle shear shadow projection

    // 3. Banyan trees (drawn BEHIND houses)
    drawAllBanyanTrees();

    // 5. Houses
    drawAllPalmTrees();
    drawAllHouses();
    drawFence(50, 490, 150, 25);
    drawFence(1380, 485, 120, 25);

    // 6. Trees and vegetation

    drawAllVegetation();
    drawReeds();

    // 7. River bank + water
    drawRiverBank();
    drawWater();
    drawWaterSceneReflection(); // Y-flip + shear scene reflection
    drawWaterReflections();     // sky-colour overlay with wave shear
    drawLilyPads();

    // 8. Underwater
    drawAllFish();

    // 9. On water
    drawAllBoats();
    drawBoatWake(boat1.x, boat1.y, boat1.direction, fabs(boat1.speed));
    drawBoatWake(boat2.x, boat2.y, boat2.direction, fabs(boat2.speed));
    drawBoatWake(boat3.x, boat3.y, boat3.direction, fabs(boat3.speed));

    // 10. Water splash particles
    for (int i = 0; i < MAX_WATER_PARTICLES; i++) {
        if (!waterParticles[i].active) continue;
        setColor(0.5f, 0.7f, 0.9f, waterParticles[i].alpha);
        drawCircle(waterParticles[i].x, waterParticles[i].y, 2, 6);
    }

    // 11. Ground entities
    drawAllChickens();
    drawAllRickshaws();
    drawAllPersons();
    drawKite();

    // 12. Sky entities
    drawAllBirds();
    drawAllButterflies();
    drawAllDragonflies();
    drawFireflies();

    // 13. Effects
    drawSmoke();
    drawLeaves();
    drawFog();
    drawRain();
    drawPuddlesEffect();
    drawLightningEffect();

    // 14. Overlays
    if (modeTransition > 0.01f) {
        setColor(0, 0, 0.05f, ambientDarkness);
        drawRect(0, 0, worldRight, worldTop);
    }
    if (screenFlash) {
        setColor(0.9f, 0.92f, 1.0f, screenFlashAlpha);
        drawRect(0, 0, worldRight, worldTop);
    }

    glPopMatrix(); // end screen-shake scope

    // 15. UI (drawn outside shake scope for stability)
    drawModeIndicator();
    drawInstructions();

    glutSwapBuffers();
}

// ============================================================
// GLUT CALLBACKS
// ============================================================

void reshape(int w, int h) {
    windowWidth = w; windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    float aspect = (float)w / (float)h;
    float targetAspect = worldRight / worldTop;
    if (aspect > targetAspect) {
        float newWidth = worldTop * aspect;
        float excess = (newWidth - worldRight) / 2;
        gluOrtho2D(-excess, worldRight + excess, worldBottom, worldTop);
    }
    else {
        float newHeight = worldRight / aspect;
        float excess = (newHeight - worldTop) / 2;
        gluOrtho2D(worldLeft, worldRight, -excess, worldTop + excess);
    }
    glMatrixMode(GL_MODELVIEW);
}

void timer(int value) {
    updateAll();
    glutPostRedisplay();
    glutTimerFunc(TIMER_INTERVAL, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: exit(0); break;
    case 'r': case 'R': isRainyMode = !isRainyMode; break;
    case 'f': case 'F':
        if (isFullscreen) { glutReshapeWindow(1280, 720); isFullscreen = false; }
        else { glutFullScreen(); isFullscreen = true; }
        break;
    case '+': case '=': targetWindStrength = clamp(targetWindStrength + 0.5f, 0, 5); break;
    case '-': case '_': targetWindStrength = clamp(targetWindStrength - 0.5f, 0, 5); break;
    case ' ': rickshaw1.isMoving = !rickshaw1.isMoving; rickshaw2.isMoving = !rickshaw2.isMoving; break;
    }
}

void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_F11:
        if (isFullscreen) { glutReshapeWindow(1280, 720); isFullscreen = false; }
        else { glutFullScreen(); isFullscreen = true; }
        break;
    case GLUT_KEY_LEFT: windDirection = -1; break;
    case GLUT_KEY_RIGHT: windDirection = 1; break;
    case GLUT_KEY_UP: sunY = clamp(sunY + 10, 300, 800); break;
    case GLUT_KEY_DOWN: sunY = clamp(sunY - 10, 300, 800); break;
    }
}

void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float worldX = (float)x / windowWidth * worldRight;
        float worldY = worldTop - (float)y / windowHeight * worldTop;
        if (worldY < waterLevel) {
            for (int i = 0; i < MAX_RIPPLES; i++) {
                if (!ripples[i].active) {
                    ripples[i] = { worldX, worldY, 2, 30, 0.8f, true };
                    break;
                }
            }
            for (int j = 0; j < 8; j++) {
                for (int i = 0; i < MAX_WATER_PARTICLES; i++) {
                    if (!waterParticles[i].active) {
                        waterParticles[i] = { worldX, worldY, randomFloat(-3,3), randomFloat(1,5), 0.8f, 1.0f, true };
                        break;
                    }
                }
            }
        }
    }
}



int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Bengali Village Scene - Ultra Realistic Sunset & Night");
    glutFullScreen();

    glClearColor(0, 0, 0, 1);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH); glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    initAll();

    glutDisplayFunc(displayFinal);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseClick);
    glutTimerFunc(TIMER_INTERVAL, timer, 0);


    printf("  Bengali Village Scene - OpenGL Enhanced\n");

    printf("Controls:\n");
    printf("  R        - Toggle Night/Rain Mode\n");
    printf("  F / F11  - Toggle Fullscreen\n");
    printf("  ESC      - Exit\n");
    printf("  +/-      - Wind Speed\n");
    printf("  Space    - Pause Rickshaws\n");
    printf("  Arrows   - Wind Dir / Sun Pos\n");
    printf("  Mouse    - Click Water for Splash\n");
    printf("Features:\n");
    printf("  * Volumetric realistic clouds\n");
    printf("  * Detailed faces (eyes, nose, ears, mouth)\n");
    printf("  * Boats with DDA/Bresenham/Midpoint algorithms\n");
    printf("  * Banyan trees behind houses\n");
    printf("  * Sun with corona / Crescent moon with craters\n");

    glutMainLoop();
    return 0;
}
