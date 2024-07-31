#include <helpers/common/Node.h>
#include <math/rio_Math.h>
#include <helpers/common/NodeMgr.h>
#include <helpers/properties/Property.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

Node::Node(std::string pNodeKey, rio::Vector3f pPos, rio::Vector3f pRot, rio::Vector3f pScale)
{
    nodeKey = pNodeKey;

    mPosition = pPos;
    mRotation = pRot;
    mScale = pScale;

    transformMatrix.makeSRT(mPosition, mRotation, mScale);
    ID = NodeMgr::instance()->GetNodeCount() + 1;

    RIO_LOG("[NODE] New node created with key: %s.\n", nodeKey.c_str());
};

bool Node::AddProperty(std::unique_ptr<Property> pProperty)
{
    properties.push_back(std::move(pProperty));
    return true;
}

void Node::CreateNodeProperties()
{
    ImGui::PushID("nodeKey");
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("", &nodeKey);
    ImGui::PopID();

    rio::Vector3f positionVector = GetPosition();
    rio::Vector3f rotationVector = GetRotation();
    rio::Vector3f scaleVector = GetScale();

    float positionArray[3] = {positionVector.x, positionVector.y, positionVector.z};
    float rotationArray[3] = {rotationVector.x, rotationVector.y, rotationVector.z};
    float scaleArray[3] = {scaleVector.x, scaleVector.y, scaleVector.z};

    ImGui::Text("Position");
    ImGui::PushID("position");
    if (ImGui::DragFloat3("", positionArray, 0.01f))
    {
        positionVector.x = positionArray[0];
        positionVector.y = positionArray[1];
        positionVector.z = positionArray[2];
        SetPosition(positionVector);
    }
    ImGui::PopID();

    ImGui::Text("Rotation");
    ImGui::PushID("rotation");
    if (ImGui::DragFloat3("", rotationArray, 0.01f))
    {
        rotationVector.x = rotationArray[0];
        rotationVector.y = rotationArray[1];
        rotationVector.z = rotationArray[2];
        SetRotation(rotationVector);
    }
    ImGui::PopID();

    ImGui::Text("Scale");
    ImGui::PushID("scale");
    if (ImGui::DragFloat3("", scaleArray, 0.01f))
    {
        scaleVector.x = scaleArray[0];
        scaleVector.y = scaleArray[1];
        scaleVector.z = scaleArray[2];
        SetScale(scaleVector);
    }
    ImGui::PopID();
}