#include <controller/rio_Controller.h>
#include <rio.h>
#include <gfx/rio_Camera.h>
#include <helpers/CameraController.h>

// Set projection matrix
rio::Vector3f CAM_POS = {0, 1, 3};
rio::Vector3f LOOK_POS = {0, 0, 3};

void useFlyCam(rio::LookAtCamera *camera, rio::Controller *controller)
{
    static float yaw = 0.0f;   // Rotation around the y-axis
    static float pitch = 0.0f; // Rotation around the x-axis
    rio::Vector3f forward;     // Calculate forward vector

    rio::Vector2f leftStickVector = controller->getLeftStick();
    rio::Vector2f rightStickVector = controller->getRightStick();

    // Update camera rotation
    const float rotationSpeed = 0.05f; // Adjust this value as needed
    yaw += rightStickVector.x * rotationSpeed * -1;
    pitch += rightStickVector.y * rotationSpeed;

    // Clamp pitch to avoid flipping the camera
    if (pitch > 1.5f)
        pitch = 1.5f;
    if (pitch < -1.5f)
        pitch = -1.5f;

    forward.x = cos(pitch) * sin(yaw);
    forward.y = sin(pitch);
    forward.z = cos(pitch) * cos(yaw);

    // Calculate right vector
    rio::Vector3f right;
    right.x = sin(yaw - 3.14f / 2.0f);
    right.y = 0;
    right.z = cos(yaw - 3.14f / 2.0f);

    // Move camera
    const float moveSpeed = 0.1f; // Adjust this value as needed
    CAM_POS += forward * leftStickVector.y * moveSpeed;
    CAM_POS += right * leftStickVector.x * moveSpeed;

    // Update camera position and orientation
    camera->at() = CAM_POS + forward;
    camera->pos().set(
        CAM_POS.x,
        CAM_POS.y,
        CAM_POS.z);
}