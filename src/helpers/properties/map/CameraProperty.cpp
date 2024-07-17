#include <gfx/rio_Camera.h>
#include <gfx/rio_Projection.h>
#include <gfx/rio_PrimitiveRenderer.h>
#include <gfx/rio_Window.h>
#include <controller/rio_Controller.h>
#include <controller/rio_ControllerMgr.h>
#include <misc/rio_MemUtil.h>

#include <helpers/properties/map/CameraProperty.h>
#include <helpers/properties/Property.h>
#include <helpers/common/Node.h>

void CameraProperty::UseFlyCam()
{
    rio::Controller *controller = rio::ControllerMgr::instance()->getGamepad(0);
    rio::Vector3f cameraPosition = CameraProperty::GetParentNode().lock()->GetPosition();

    if (!controller || !controller->isConnected())
    {
        controller = rio::ControllerMgr::instance()->getMainGamepad();
        RIO_ASSERT(controller);
    }

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
    cameraPosition += forward * leftStickVector.y * moveSpeed;
    cameraPosition += right * leftStickVector.x * moveSpeed;

    // Update camera position and orientation
    mCamera.at() = cameraPosition + forward;
    CameraProperty::GetParentNode().lock()->SetPosition(cameraPosition);
    mCamera.pos().set(cameraPosition.x, cameraPosition.y, cameraPosition.z);
}

void CameraProperty::Load(YAML::Node node)
{
    if (!node["cameraType"])
    {
        RIO_LOG("[CAMERA] Error: Missing 'cameraType' in YAML node.\n");
        return;
    }

    if (!node["cameraFOV"])
    {
        RIO_LOG("[CAMERA] Error: Missing 'cameraFOV' in YAML node.\n");
        return;
    }

    f32 fov = node["cameraFOV"].as<f32>();
    mCameraType = static_cast<CameraType>(node["cameraType"].as<int>());

    // Get window instance
    const rio::Window *const window = rio::Window::instance();

    // Create perspective projection instance
    rio::PerspectiveProjection proj(
        0.1f,
        100.0f,
        rio::Mathf::deg2rad(fov),
        f32(window->getWidth()) / f32(window->getHeight()));

    // Calculate matrix
    rio::MemUtil::copy(&mProjMtx, &proj.getMatrix(), sizeof(rio::Matrix44f));

    // Set primitive renderer projection
    rio::PrimitiveRenderer::instance()->setProjection(proj);
}

void CameraProperty::Update()
{
    rio::Window::instance()->clearColor(0.2f, 0.3f, 0.3f, 0.0f);
    rio::Window::instance()->clearDepthStencil();

    switch (mCameraType)
    {
    case CAMERA_NODE_FLYCAM:
        CameraProperty::UseFlyCam();
        break;
    }

    // Get window instance
    const rio::Window *const window = rio::Window::instance();

    // Create perspective projection instance
    rio::PerspectiveProjection proj(
        0.1f,
        100.0f,
        rio::Mathf::deg2rad(90.f),
        f32(window->getWidth()) / f32(window->getHeight()));

    // Calculate matrix
    rio::MemUtil::copy(&mProjMtx, &proj.getMatrix(), sizeof(rio::Matrix44f));

    rio::PrimitiveRenderer::instance()->setCamera(mCamera);
}
