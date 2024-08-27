#include <imgui.h>

#include <helpers/properties/map/CameraProperty.h>

YAML::Node CameraProperty::Save()
{
    YAML::Node node;

    node["Camera"]["cameraType"] = (int)(mCameraType);
    node["Camera"]["cameraFOV"] = (float)(fov);
    node["Camera"]["propertyId"] = Property::GetPropertyID();

    return node;
}

void CameraProperty::UseFlyCam()
{
    rio::Controller *controller = rio::ControllerMgr::instance()->getMainController();
    rio::Vector3f cameraPosition = CameraProperty::GetParentNode().lock()->GetPosition();

    RIO_LOG("%f, %f\n", controller->getPointer().x, controller->getPointer().y);

    return;

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

    fov = node["cameraFOV"].as<f32>();
    mCameraType = static_cast<CameraType>(node["cameraType"].as<int>());
    Property::SetPropertyID(node["propertyId"].as<int>());
}

void CameraProperty::Start()
{
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

    mInitialized = true;
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

    rio::Vector3f camPos = CameraProperty::GetParentNode().lock()->GetPosition();

    mCamera.pos().set(camPos.x, camPos.y, camPos.z);

    // Get window instance
    const rio::Window *const window = rio::Window::instance();

    // Create perspective projection instance
    rio::PerspectiveProjection proj(
        0.1f,
        1000.0f,
        rio::Mathf::deg2rad(fov),
        f32(window->getWidth()) / f32(window->getHeight()));

    // Calculate matrix
    rio::MemUtil::copy(&mProjMtx, &proj.getMatrix(), sizeof(rio::Matrix44f));

    rio::PrimitiveRenderer::instance()->setCamera(mCamera);
    rio::AudioMgr::instance()->setListener(GetParentNode().lock()->GetPosition(), mCamera.at(), mCamera.getUp());
}

void CameraProperty::CreatePropertiesMenu()
{
    ImGui::PushID(GetPropertyID());

    if (ImGui::CollapsingHeader("Camera"))
    {
        ImGui::PopID();

        std::string comboId = "CameraType_" + std::to_string(GetPropertyID());
        std::string fovId = "FOV_" + std::to_string(GetPropertyID());

        ImGui::Text("FOV");
        ImGui::PushID(fovId.c_str());

        ImGui::DragFloat("", &fov, 0.1f, 0.0f, 200.f);
        ImGui::PopID();

        ImGui::Text("Camera Type");
        ImGui::PushID(comboId.c_str());
        if (ImGui::BeginCombo("", CameraTypeInfo[(int)(mCameraType)].name))
        {
            for (int i = 0; i < 2; ++i)
            {
                // Calculate if our selectable should appear as selected.
                bool isSelected = (mCameraType == (CameraType)(i));

                if (ImGui::Selectable(CameraTypeInfo[i].name, isSelected))
                {
                    // Finally, if our selectable is clicked, we change our class member enum to the selected value.
                    mCameraType = (CameraType)(CameraTypeInfo[i].value);
                }

                // If our selectable is selected, we set the default focus to it.
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }
    }
}
