#ifndef _DELTAFRAME_H
#define _DELTAFRAME_H

#include <chrono>
#include <cstdint>

const double TARGET_DT_60 = 60.0;
const double TARGET_DT_144 = 144.0;

class DeltaFrameClass
{
    float deltaTime;        // 이번 프레임의 dt
    uint64_t tick;             // 누적 프레임 번호
    double lastUpdateTime;   // 이전 프레임 timestamp (초 단위)
    const float targetDelta;       // 목표 프레임 (예: 1/60)
    const float maxDeltaClamp;     // 최대 delta (예: 0.1초)

public:
    DeltaFrameClass(float targetFps = TARGET_DT_60)
        : targetDelta(1.0f / targetFps),
        maxDeltaClamp(0.1f)      // 100ms 이상 튀는 건 clamping
    {
        lastUpdateTime = now();
    }

    inline double now() const
    {
        using namespace std::chrono;
        return duration_cast<duration<double>>(steady_clock::now().time_since_epoch()).count();
    }

    // 프레임 갱신
    bool update()
    {
        double curr = now();
        double dt = curr - lastUpdateTime;

        // 너무 튀는 dt 방지
        if (dt > maxDeltaClamp) dt = targetDelta;

        deltaTime = (float)dt;
        lastUpdateTime = curr;

        tick++;
        return true;
    }

    float GetDelta() const { return deltaTime; }
    uint64_t GetTick() const { return tick; }
};

#endif