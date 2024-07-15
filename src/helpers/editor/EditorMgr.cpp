#include <helpers/editor/EditorMgr.h>
#include <string>
#include <imgui.h>

EditorMgr *EditorMgr::mInstance = nullptr;

bool EditorMgr::createSingleton()
{
    if (mInstance)
        return false;

    mInstance = new EditorMgr();
    mInstance->mInitialized = true;

    if (!mInstance->mInitialized)
    {
        delete mInstance;
        mInstance = nullptr;
        return false;
    }

    return true;
}

bool EditorMgr::destorySingleton()
{
    if (!mInstance)
        return false;

    delete mInstance;
    mInstance = nullptr;

    return true;
}

void EditorMgr::CreateNodePropertiesMenu()
{
    // ImGui::InputText(EditorMgr::instance()->selectedNode->nodeKey, EditorMgr::instance()->selectedNode->nodeKey, sizeof(EditorMgr::instance()->selectedNode->nodeKey));

    Node *selectedNode = EditorMgr::instance()->selectedNode;

    if (ImGui::CollapsingHeader("Position"))
    {
        rio::Vector3f nodePos = selectedNode->GetPosition();

        ImGui::Text("X");
        ImGui::SameLine();
        if (ImGui::DragFloat("posX", &nodePos.x, 0.01f))
            selectedNode->SetPosition(nodePos);

        ImGui::Text("Y");
        ImGui::SameLine();
        if (ImGui::DragFloat("posY", &nodePos.y, 0.01f))
            selectedNode->SetPosition(nodePos);

        ImGui::Text("Z");
        ImGui::SameLine();
        if (ImGui::DragFloat("posZ", &nodePos.z, 0.01f))
            selectedNode->SetPosition(nodePos);
    }

    if (ImGui::CollapsingHeader("Scale"))
    {
        rio::Vector3f nodeScale = selectedNode->GetScale();

        ImGui::Text("X");
        ImGui::SameLine();
        if (ImGui::DragFloat("scaleX", &nodeScale.x, 0.01f))
            selectedNode->SetScale(nodeScale);

        ImGui::Text("Y");
        ImGui::SameLine();
        if (ImGui::DragFloat("scaleY", &nodeScale.y, 0.01f))
            selectedNode->SetScale(nodeScale);

        ImGui::Text("Z");
        ImGui::SameLine();
        if (ImGui::DragFloat("scaleZ", &nodeScale.z, 0.01f))
            selectedNode->SetScale(nodeScale);
    }
}