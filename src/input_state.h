#pragma once

struct InputState {
    float moveX = 0.0f;

    bool jump = false;
    bool fastFall = false;

    bool shoot = false;

    bool toggleFullscreen = false;
};