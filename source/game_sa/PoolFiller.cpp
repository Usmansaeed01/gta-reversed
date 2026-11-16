#include "StdInc.h"

#include <algorithm>

#include "PoolFiller.h"
#include "WaterCannons.h"
#include "WaterCannon.h"
#include "WaterLevel.h"
#include "Timer.h"

// Called once per frame from CWaterCannons::Update() (see PoolFiller.h).
void CPoolFiller::Update() {
    if (!m_bInitialised) {
        Init();
    }
    if (m_nQuadIdx < 0) {
        return; // no pool quad (e.g. water arrays full) - nothing to do
    }

    // Count how many active spray sections are landing inside the pool.
    int32 hits = 0;
    for (const auto& cannon : CWaterCannons::aCannons) {
        if (cannon.m_nId == 0) {
            continue; // cannon not in use
        }
        for (auto i = 0; i < CWaterCannon::SECTIONS_COUNT; i++) {
            if (!cannon.m_abUsed[i]) {
                continue;
            }
            if (IsPointInPool(cannon.m_sectionPoint[i])) {
                hits++;
            }
        }
    }

    const float dt = CTimer::GetTimeStep(); // ~1.0 at 60 fps, scales with frame time
    float newLevel = m_fLevel;
    if (hits > 0) {
        // Fill: more spray sections in the pool = faster fill.
        newLevel += hits * FILL_PER_SECTION * dt;
    } else {
        // Slow leak so the pool can be filled again later.
        newLevel -= DRAIN_PER_SECOND * dt / 60.0f;
    }

    // Clamp to [start, full] and apply.
    newLevel = std::clamp(newLevel, POOL_START_Z, POOL_FULL_Z);
    if (newLevel != m_fLevel) {
        m_fLevel = newLevel;
        SetWaterHeight(m_fLevel);
    }
}

void CPoolFiller::Init() {
    m_bInitialised = true;
    m_fLevel = POOL_START_Z;

    // Add ONE water quad over the pool at the start (near-empty) height.
    // We keep its quad + vertex indices so we can raise its Z later instead
    // of adding a new quad every frame (the water arrays hold max 301 quads
    // and 1021 vertices - adding one per frame would overflow them).
    const int32 quadIdx = (int32)CWaterLevel::NumWaterQuads;
    if (quadIdx >= 301) {
        return; // water quad array full
    }
    // The quad needs 4 NEW vertices (our pool corners are unique, so none
    // are reused). If the 1021-vertex array is already nearly full, adding
    // them would write past the end of m_aVertices and corrupt memory
    // (freeze/crash). Skip the pool in that case - the game stays playable.
    if ((int32)CWaterLevel::NumWaterVertices + 4 > 1021) {
        return; // water vertex array nearly full - no room for the pool quad
    }
    const CRenPar rp{ POOL_START_Z, 0.0f, 0.0f, 0, 0 };
    CWaterLevel::AddWaterLevelQuad(
        POOL_MIN_X, POOL_MIN_Y, rp,
        POOL_MAX_X, POOL_MIN_Y, rp,
        POOL_MAX_X, POOL_MAX_Y, rp,
        POOL_MIN_X, POOL_MAX_Y, rp,
        0 // flags: visible, normal depth
    );
    if ((int32)CWaterLevel::NumWaterQuads <= quadIdx) {
        return; // quad was not added (e.g. degenerate rectangle)
    }
    m_nQuadIdx = quadIdx;

    // Remember the quad's 4 vertex indices so we can raise their Z later.
    // Note: AddWaterLevelVertex reuses a vertex only when (x, y, z) all match,
    // and this pool is inland, so these 4 vertices belong to our quad alone.
    const auto& quad = CWaterLevel::WaterQuads[(size_t)m_nQuadIdx];
    for (auto i = 0; i < 4; i++) {
        m_aVertIdx[i] = quad.verts[i];
    }
}

bool CPoolFiller::IsPointInPool(const CVector& pt) {
    // Spray section must be inside the pool rectangle and at pool height
    // (not flying high over it, and not already fallen through the floor).
    return pt.x >= (float)POOL_MIN_X && pt.x <= (float)POOL_MAX_X
        && pt.y >= (float)POOL_MIN_Y && pt.y <= (float)POOL_MAX_Y
        && pt.z >= POOL_FLOOR_Z - 0.5f && pt.z <= POOL_FULL_Z + 2.0f;
}

void CPoolFiller::SetWaterHeight(float z) {
    // Raise/lower the pool quad by editing its 4 vertices' Z in place.
    // (Do NOT call AddWaterLevelQuad again - that would append a new quad
    // every time and overflow the 301-quad array.)
    for (auto i = 0; i < 4; i++) {
        CWaterLevel::m_aVertices[m_aVertIdx[i]].rp.z = z;
    }
}
