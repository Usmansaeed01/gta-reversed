#pragma once

// PoolFiller - Fill a swimming pool with a fire truck's water cannon.
// GTA Reversed (gta-reversed) feature - only this part, nothing else.
//
// How it works:
//   * Every frame it reads the active water cannons (CWaterCannons::aCannons).
//   * Each cannon spray is 32 sections; every section has a 3D position.
//   * If a spray section lands inside the pool rectangle, the pool's water
//     level rises a little. The level is drawn as a real CWaterLevel quad
//     (added once, then its 4 vertices' Z is raised as the pool fills).
//   * The level drains very slowly when nobody is spraying, so the pool can
//     be filled again.
//
// Target pool (default): the Johnson house pool, Grove Street, Ganton.
//   The house front door is at X 2495.142, Y -1687.714 (GTA SA coords).
//   The pool is in the backyard, north of the house.
//   The bounds below are a best estimate - if the spray does not register,
//   stand at the pool corners in-game, note the coords, and adjust them.
//
// Integration (one line):
//   Call CPoolFiller::Update() at the end of CWaterCannons::Update()
//   in source/game_sa/WaterCannons.cpp. New files are picked up by the
//   project's GLOB_RECURSE CMake, so no CMake change is needed.
//
// Build: Windows only (Visual Studio 2026 + Conan + CMake, see the
//   gta-reversed README). This file is NOT built on Linux.

class CVector;

class CPoolFiller {
public:
    // ---- Pool rectangle (Johnson house pool, Grove Street) ----
    // X/Y are world coords (integers, as used by CWaterLevel).
    static constexpr int32 POOL_MIN_X = 2491;
    static constexpr int32 POOL_MAX_X = 2501;
    static constexpr int32 POOL_MIN_Y = -1679;
    static constexpr int32 POOL_MAX_Y = -1667;

    // Z heights. Ground around the house is ~13.5. The pool floor is below
    // ground; the water surface when "full" sits just under the rim.
    static constexpr float POOL_FLOOR_Z = 11.8f; // pool bottom
    static constexpr float POOL_START_Z = 12.0f; // water level when empty
    static constexpr float POOL_FULL_Z  = 13.2f; // water level when full

    // How fast the pool fills / drains.
    static constexpr float FILL_PER_SECTION = 0.00020f; // per spray section, per frame
    static constexpr float DRAIN_PER_SECOND = 0.004f;    // slow leak when idle

public:
    static void Update();

private:
    static void Init();
    static bool IsPointInPool(const CVector& pt);
    static void SetWaterHeight(float z);

    static inline bool   m_bInitialised = false;
    static inline float  m_fLevel       = POOL_START_Z;
    static inline int32  m_nQuadIdx     = -1;
    static inline uint16 m_aVertIdx[4]  = { 0, 0, 0, 0 };
};

// Built via GitHub Actions (windows-2025-vs2026) on the Usmansaeed01 fork.
