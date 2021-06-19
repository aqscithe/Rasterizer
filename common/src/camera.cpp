
#include <imgui.h>

#include <common/maths.hpp>

#include <common/camera.hpp>

Camera::Camera(int width, int height)
{
    // width and height are useful to compute projection matrix with the right aspect ratio
    aspect = (float)width / (float)height;

}

void Camera::update(float deltaTime, const CameraInputs& inputs)
{
    const float MOUSE_SENSITIVITY = 0.002f;
    const float SPEED = 1.f;

    yaw += inputs.deltaX * MOUSE_SENSITIVITY;
    pitch += inputs.deltaY * MOUSE_SENSITIVITY;

    float forwardMovement = 0.f;
    if (inputs.moveForward)
        forwardMovement += SPEED * deltaTime;
    if (inputs.moveBackward)
        forwardMovement -= SPEED * deltaTime;

    float strafeMovement = 0.f;
    if (inputs.strafeLeft)
        strafeMovement -= SPEED * deltaTime;
    if (inputs.strafeRight)
        strafeMovement += SPEED * deltaTime;

    float verticalMovement = 0.f;
    if (inputs.moveUp)
        verticalMovement += SPEED * deltaTime;
    if (inputs.moveDown)
        verticalMovement -= SPEED * deltaTime;

    // TODO: Add Pitch

    position.x += maths::sin(yaw) * forwardMovement;
    position.z -= maths::cos(yaw) * forwardMovement;

    position.x += maths::cos(yaw) * strafeMovement;
    position.z += maths::sin(yaw) * strafeMovement;

    position.y += verticalMovement;
}

mat4x4 Camera::getViewMatrix()
{
    return mat4::rotateX(pitch) * mat4::rotateY(yaw) * mat4::translate(-position);
}

mat4x4 Camera::getProjection()
{
    return mat4::perspective(fovY, aspect, near, far);
}

void Camera::showImGuiControls()
{
    ImGui::DragFloat3("pos", position.e);
    //ImGui::DragFloat("aspect", &aspect);
    ImGui::DragFloat("fovY", &fovY);
    ImGui::DragFloat("near", &near);
    ImGui::DragFloat("far", &far);
}