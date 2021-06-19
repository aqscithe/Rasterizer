#pragma once

#include <common/types.hpp>

struct CameraInputs
{
    float deltaX;
    float deltaY;
    bool moveForward;
    bool moveBackward;
    bool strafeLeft;
    bool strafeRight;
    bool moveUp;
    bool moveDown;
};

struct Camera
{
    Camera(int width, int height);

    void update(float deltaTime, const CameraInputs& inputs);
    mat4x4 getViewMatrix();
    mat4x4 getProjection();

    void showImGuiControls();

private:
    float pitch = 0.f;
    float yaw = 0.f;

    float3 position = { 0.175f, 0.474f, 1.773f };
    float aspect = 1.f;  // 1 when width == height
    float fovY = maths::toRadians(60.f);
    float near = 0.01f;
    float far = 10.f;

};