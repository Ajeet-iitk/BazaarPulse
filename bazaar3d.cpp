/* =========================================================================
   BazaarPulse 3D — C++ scene builder
   -------------------------------------------------------------------------
   Builds the complete 3D bazaar environment as a scene graph:
   street geometry, 12 shop stalls, awnings, counters, lamps, torans,
   rangoli points, skyline, props and the end-of-street gate.

   The browser renderer (Three.js) consumes the generated
   bazaar3d_scene.js file. This keeps the 3D world layout, shops and
   decorations authored in C++ rather than hand-placed in HTML/CSS.

   Build:
     g++ -O2 -std=c++17 bazaar3d.cpp -o bazaar3d.exe
     .\bazaar3d.exe > bazaar3d_scene.js
   ========================================================================= */

#include <cstdio>
#include <cstdint>
#include <cmath>
#include <vector>
#include <string>

struct ShopDef {
  const char* id;
  const char* side;
  const char* colorA;
  const char* colorB;
};

struct ShopOut {
  int index;
  std::string id;
  std::string side;
  std::string colorA;
  std::string colorB;
  float x;
  float z;
  float rotY;
};

struct Lamp { float x; float z; const char* side; float height; };
struct Bunting { float z; float y; int count; };
struct Rangoli { float z; float radius; float spin; };
struct Building { float x; float z; float w; float h; float d; const char* side; int tone; };
struct Prop { int type; float x; float z; float rot; float scale; int tone; };
struct Decor { float x; float z; float rotY; const char* side; const char* a; const char* b; int kind; };
struct Dash { float z; };
struct Slot { float x; float y; float z; };

static const float SHOP_SPACING = 20.0f;
static const float FIRST_SHOP_Z = -18.0f;
static const float ROAD_WIDTH = 12.0f;
static const float WALK_WIDTH = 3.4f;
static const float SHOP_X = 9.9f;
static const float SHOP_WIDTH = 10.0f;
static const float SHOP_DEPTH = 4.6f;
static const float SHOP_HEIGHT = 5.2f;
static const int SHOP_COUNT = 12;

static const ShopDef SHOP_DEFS[SHOP_COUNT] = {
  {"fruit",    "left",  "#f43f5e", "#fb923c"},
  {"dairy",    "right", "#10b981", "#22d3ee"},
  {"veg",      "left",  "#22c55e", "#84cc16"},
  {"egg",      "right", "#f59e0b", "#fbbf24"},
  {"dry",      "left",  "#b45309", "#eab308"},
  {"travel",   "right", "#6366f1", "#8b5cf6"},
  {"grooming", "left",  "#06b6d4", "#3b82f6"},
  {"food",     "right", "#b45309", "#f59e0b"},
  {"study",    "left",  "#0ea5e9", "#38bdf8"},
  {"health",   "right", "#e11d48", "#fb7185"},
  {"grains",   "left",  "#ca8a04", "#84cc16"},
  {"sweets",   "right", "#db2777", "#f472b6"}
};

static uint32_t rngState = 20261112u;

static float rnd() {
  rngState = rngState * 1664525u + 1013904223u;
  return (float)((rngState >> 8) & 0xFFFFFF) / (float)0xFFFFFF;
}

static float range(float a, float b) { return a + (b - a) * rnd(); }
static int irange(int a, int b) { return a + (int)(rnd() * (float)(b - a + 1)); }

static void comma(int i, int n) {
  if (i < n - 1) std::printf(",");
}

int main() {
  std::vector<ShopOut> shops;
  shops.reserve(SHOP_COUNT);

  for (int i = 0; i < SHOP_COUNT; ++i) {
    const ShopDef& d = SHOP_DEFS[i];
    ShopOut s;
    s.index = i;
    s.id = d.id;
    s.side = d.side;
    s.colorA = d.colorA;
    s.colorB = d.colorB;
    bool left = std::string(d.side) == "left";
    s.x = left ? -SHOP_X : SHOP_X;
    s.z = FIRST_SHOP_Z - (float)i * SHOP_SPACING;
    /* Left stalls face +X, right stalls face -X. */
    s.rotY = left ? 1.5707963f : -1.5707963f;
    shops.push_back(s);
  }

  float lastZ = shops.back().z;
  float maxZ = lastZ - 14.0f;

  /* Goods display slots on each counter (local shop space). */
  std::vector<Slot> slots;
  for (int i = 0; i < 4; ++i) {
    slots.push_back({ -3.45f + (float)i * 2.3f, 1.72f, 1.55f });
  }

  /* Lamps, alternating sides every two stalls. */
  std::vector<Lamp> lamps;
  for (float z = -8.0f; z > maxZ; z -= 32.0f) {
    lamps.push_back({ -7.25f, z, "left", 5.1f });
    lamps.push_back({  7.25f, z, "right", 5.1f });
  }

  /* Festoon / toran strings across the street. */
  std::vector<Bunting> buntings;
  for (float z = -30.0f; z > maxZ; z -= 60.0f) {
    buntings.push_back({ z, 5.7f, 11 });
  }

  /* Rangoli medallions painted on the road. */
  std::vector<Rangoli> rangolis;
  for (float z = -30.0f; z > maxZ; z -= 82.0f) {
    rangolis.push_back({ z, range(2.3f, 3.1f), range(0.0f, 1.0f) });
  }

  /* Old-city skyline behind the stalls, both sides. */
  std::vector<Building> buildings;
  for (int i = 0; i < 44; ++i) {
    bool left = (i % 2) == 0;
    float x = (left ? -1.0f : 1.0f) * range(26.0f, 48.0f);
    float z = range(maxZ - 10.0f, FIRST_SHOP_Z + 6.0f);
    float w = range(7.0f, 13.0f);
    float h = range(9.0f, 24.0f);
    float d = range(6.0f, 14.0f);
    buildings.push_back({ x, z, w, h, d, left ? "left" : "right", irange(0, 2) });
  }

  /* Filler stalls on the opposite side of every gap, so both sides of the
     street stay crowded like a real bazaar. These are decorative market
     carts; the interactive shops are the twelve entries above. */
  static const char* DECOR_A[6] = { "#f4a521", "#f2547d", "#19c2b0", "#8b7bf7", "#e0623e", "#2fa36b" };
  static const char* DECOR_B[6] = { "#f7d060", "#ff9ab5", "#7ff0e0", "#c3b7ff", "#ffb073", "#8fd8a6" };
  std::vector<Decor> decor;
  for (int i = 0; i < SHOP_COUNT - 1; ++i) {
    bool shopLeft = std::string(shops[(size_t)i].side) == "left";
    bool sideLeft = !shopLeft;
    float x = sideLeft ? -SHOP_X : SHOP_X;
    float z = shops[(size_t)i].z - SHOP_SPACING * 0.5f;
    float rotY = sideLeft ? 1.5707963f : -1.5707963f;
    int pal = i % 6;
    decor.push_back({ x, z, rotY, sideLeft ? "left" : "right", DECOR_A[pal], DECOR_B[pal], i % 3 });
  }

  /* Street-side props: pots, crates, barrels, sacks. */
  std::vector<Prop> props;
  for (int i = 0; i < SHOP_COUNT; ++i) {
    float z = shops[(size_t)i].z;
    bool left = shops[(size_t)i].side == "left";
    float x = (left ? -1.0f : 1.0f) * range(7.0f, 7.9f);
    props.push_back({ irange(0, 3), x, z + range(-4.0f, 4.0f), range(0.0f, 6.28f), range(0.7f, 1.25f), irange(0, 2) });
    props.push_back({ irange(0, 3), x + (left ? -0.55f : 0.55f), z + range(-5.5f, 5.5f), range(0.0f, 6.28f), range(0.55f, 0.95f), irange(0, 2) });
  }

  /* Centre-line dashes. */
  std::vector<Dash> dashes;
  for (float z = -12.0f; z > maxZ; z -= 4.5f) dashes.push_back({ z });

  std::printf("/* AUTO-GENERATED by bazaar3d.cpp — Indian Bazaar 3D scene graph. */\n");
  std::printf("window.BAZAAR_SCENE = {\n");
  std::printf("  meta: {\n");
  std::printf("    generator: \"C++ bazaar3d.cpp\",\n");
  std::printf("    seed: %u,\n", rngState);
  std::printf("    unit: \"meters\",\n");
  std::printf("    shopCount: %d,\n", SHOP_COUNT);
  std::printf("    shopSpacing: %.2f,\n", SHOP_SPACING);
  std::printf("    firstShopZ: %.2f,\n", FIRST_SHOP_Z);
  std::printf("    lastShopZ: %.2f,\n", lastZ);
  std::printf("    streetEndZ: %.2f,\n", maxZ);
  std::printf("    roadWidth: %.2f,\n", ROAD_WIDTH);
  std::printf("    walkWidth: %.2f,\n", WALK_WIDTH);
  std::printf("    shopX: %.2f,\n", SHOP_X);
  std::printf("    shopWidth: %.2f,\n", SHOP_WIDTH);
  std::printf("    shopDepth: %.2f,\n", SHOP_DEPTH);
  std::printf("    shopHeight: %.2f,\n", SHOP_HEIGHT);
  std::printf("    playerZScale: %.8f\n", 20.0 / 380.0);
  std::printf("  },\n");

  std::printf("  shops: [\n");
  for (size_t i = 0; i < shops.size(); ++i) {
    const ShopOut& s = shops[i];
    std::printf("    { id: \"%s\", index: %d, side: \"%s\", x: %.2f, z: %.2f, rotY: %.5f, a: \"%s\", b: \"%s\" }",
      s.id.c_str(), s.index, s.side.c_str(), s.x, s.z, s.rotY, s.colorA.c_str(), s.colorB.c_str());
    comma((int)i, (int)shops.size());
    std::printf("\n");
  }
  std::printf("  ],\n");

  std::printf("  decorStalls: [\n");
  for (size_t i = 0; i < decor.size(); ++i) {
    const Decor& d = decor[i];
    std::printf("    { x: %.2f, z: %.2f, rotY: %.5f, side: \"%s\", a: \"%s\", b: \"%s\", kind: %d }",
      d.x, d.z, d.rotY, d.side, d.a, d.b, d.kind);
    comma((int)i, (int)decor.size());
    std::printf("\n");
  }
  std::printf("  ],\n");

  std::printf("  goodsSlots: [\n");
  for (size_t i = 0; i < slots.size(); ++i) {
    std::printf("    { x: %.2f, y: %.2f, z: %.2f }", slots[i].x, slots[i].y, slots[i].z);
    comma((int)i, (int)slots.size());
    std::printf("\n");
  }
  std::printf("  ],\n");

  std::printf("  lamps: [\n");
  for (size_t i = 0; i < lamps.size(); ++i) {
    std::printf("    { x: %.2f, z: %.2f, side: \"%s\", height: %.2f }", lamps[i].x, lamps[i].z, lamps[i].side, lamps[i].height);
    comma((int)i, (int)lamps.size());
    std::printf("\n");
  }
  std::printf("  ],\n");

  std::printf("  bunting: [\n");
  for (size_t i = 0; i < buntings.size(); ++i) {
    std::printf("    { z: %.2f, y: %.2f, count: %d }", buntings[i].z, buntings[i].y, buntings[i].count);
    comma((int)i, (int)buntings.size());
    std::printf("\n");
  }
  std::printf("  ],\n");

  std::printf("  rangoli: [\n");
  for (size_t i = 0; i < rangolis.size(); ++i) {
    std::printf("    { z: %.2f, radius: %.2f, spin: %.3f }", rangolis[i].z, rangolis[i].radius, rangolis[i].spin);
    comma((int)i, (int)rangolis.size());
    std::printf("\n");
  }
  std::printf("  ],\n");

  std::printf("  skyline: [\n");
  for (size_t i = 0; i < buildings.size(); ++i) {
    const Building& b = buildings[i];
    std::printf("    { x: %.2f, z: %.2f, w: %.2f, h: %.2f, d: %.2f, side: \"%s\", tone: %d }",
      b.x, b.z, b.w, b.h, b.d, b.side, b.tone);
    comma((int)i, (int)buildings.size());
    std::printf("\n");
  }
  std::printf("  ],\n");

  std::printf("  props: [\n");
  for (size_t i = 0; i < props.size(); ++i) {
    const Prop& p = props[i];
    std::printf("    { type: %d, x: %.2f, z: %.2f, rot: %.2f, scale: %.2f, tone: %d }",
      p.type, p.x, p.z, p.rot, p.scale, p.tone);
    comma((int)i, (int)props.size());
    std::printf("\n");
  }
  std::printf("  ],\n");

  std::printf("  dashes: [");
  for (size_t i = 0; i < dashes.size(); ++i) {
    std::printf("%.2f", dashes[i].z);
    comma((int)i, (int)dashes.size());
  }
  std::printf("],\n");

  std::printf("  gate: { z: %.2f, width: 15.0, height: 8.2 }\n", maxZ - 8.0f);
  std::printf("};\n");
  return 0;
}
