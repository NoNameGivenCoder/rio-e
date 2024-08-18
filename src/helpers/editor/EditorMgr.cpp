#include <helpers/editor/EditorMgr.h>
#include <helpers/properties/Property.h>
#include <helpers/properties/audio/AudioProperty.h>
#include <string>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <gfx/rio_PrimitiveRenderer.h>
#include <helpers/common/NodeMgr.h>
#include <gfx/rio_Window.h>
#include <iostream>
#include <gpu/rio_RenderBuffer.h>
#include <misc/rio_MemUtil.h>
#include <helpers/editor/ConversionMgr.h>
#include <filesystem>
#include <helpers/editor/Texture2DUtil.h>
#include <misc/cpp/imgui_stdlib.h>

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

    mInstance->mFileDevice = rio::FileDeviceMgr::instance()->getMainFileDevice();
    mInstance->mTextureFolderPath = mInstance->mFileDevice->getNativePath("textures");
    mInstance->mStringFolderPath = mInstance->mFileDevice->getNativePath("lang");

    return true;
}

bool EditorMgr::destorySingleton()
{
    if (!mInstance)
        return false;

    delete mInstance->mpColorTexture;
    delete mInstance->mpDepthTexture;

    delete mInstance;
    mInstance = nullptr;

    return true;
}

void EditorMgr::Update()
{
    rio::Window *window = rio::Window::instance();
    s32 width, height;

    width = window->getWidth();
    height = window->getHeight();

    mRenderBuffer.setSize(width, height);
    mRenderBuffer.clear(rio::RenderBuffer::CLEAR_FLAG_COLOR_DEPTH, {0.2f, 0.3f, 0.3f, 0.0f});
}

void EditorMgr::SetupFrameBuffer()
{
    mpColorTexture = new rio::Texture2D(rio::TEXTURE_FORMAT_R8_G8_B8_A8_UNORM, 1280, 720, 1);
    mpDepthTexture = new rio::Texture2D(rio::DEPTH_TEXTURE_FORMAT_R32_FLOAT, 1280, 720, 1);

    mRenderBuffer.setSize(1280, 720);
    mColorTarget.linkTexture2D(*mpColorTexture);
    mDepthTarget.linkTexture2D(*mpDepthTexture);

    mRenderBuffer.setRenderTargetColor(&mColorTarget);
    mRenderBuffer.setRenderTargetDepth(&mDepthTarget);

    mRenderBuffer.clear(rio::RenderBuffer::CLEAR_FLAG_DEPTH);

    RIO_LOG("[EDITORMGR] Created render buffer! \n");
}

void EditorMgr::BindRenderBuffer()
{
    rio::Window *window = rio::Window::instance();

    int width, height;

    width = window->getWidth();
    height = window->getHeight();

    mRenderBuffer.setSize(1280, 720);

    mRenderBuffer.setRenderTargetColorNull(2);
    mRenderBuffer.bind();
}

void EditorMgr::UnbindRenderBuffer()
{
    mRenderBuffer.getRenderTargetColor()->invalidateGPUCache();
    mpColorTexture->setCompMap(0x00010205);

    rio::Window::instance()->makeContextCurrent();

    rio::Graphics::setViewport(0, 0, 1280, 720);
    rio::Graphics::setScissor(0, 0, 1280, 720);
}

void EditorMgr::UpdateTexturesDirCache()
{
    std::filesystem::file_time_type currentFileWriteTime = std::filesystem::last_write_time(mTextureFolderPath);

    if (currentFileWriteTime != mTexturesLastWriteTime)
    {
        mTextureSelected.clear();
        mTexturesLastWriteTime = currentFileWriteTime;
        mTextureCachedContents.clear();
        mTextures.clear();

        for (const auto &fileEntry : std::filesystem::directory_iterator(mTextureFolderPath))
        {
            RIO_LOG("[EDITORMGR] Loading texture %s..\n", fileEntry.path().filename().c_str());

            std::ifstream file(fileEntry.path(), std::ios::binary);

            if (!file)
            {
                RIO_LOG("[EDITORMGR] Failed to open texture file: %s\n", fileEntry.path().filename().c_str());
                continue;
            }

            u8 *fileBuffer = new u8[fileEntry.file_size()];

            file.read(reinterpret_cast<char *>(fileBuffer), fileEntry.file_size());
            file.close();

            if (!file)
            {
                RIO_LOG("[EDITORMGR] Failed to read texture file: %s\n", fileEntry.path().filename().c_str());
                delete[] fileBuffer;
                continue;
            }

            // Create rio::Texture2D object
            if (fileEntry.path().extension().string() == ".rtx")
            {
                std::unique_ptr<rio::Texture2D> texture = std::make_unique<rio::Texture2D>(fileBuffer, fileEntry.file_size());

                mTextures[fileEntry.path().string()] = std::move(texture);
                mTextureCachedContents.push_back(fileEntry.path());

                delete[] fileBuffer;

                continue;
            }

            // Thank you https://github.com/aboood40091/Miyamoto-Next
            if (fileEntry.path().extension().string() == ".gtx")
            {
                std::vector<u8> fillBuffer;
                ConvertGtxToRtx(fileBuffer, fillBuffer);

                mFileDevice->open(&fileHandle, "textures/" + fileEntry.path().filename().string() + ".rtx", rio::FileDevice::FILE_OPEN_FLAG_WRITE);
                mFileDevice->write(&fileHandle, fillBuffer.data(), fillBuffer.size());
                mFileDevice->close(&fileHandle);

                std::unique_ptr<rio::Texture2D> texture;

                Texture2DUtil::createFromGTX(fileBuffer, &texture);

                mTextures[fileEntry.path().string()] = std::move(texture);
                mTextureCachedContents.push_back(fileEntry.path());

                delete[] fileBuffer;

                continue;
            }

            mTextures[fileEntry.path().string()] = nullptr;
            mTextureCachedContents.push_back(fileEntry.path());

            delete[] fileBuffer;
        }
    }
}

void EditorMgr::UpdateStringsDirCache()
{
    std::filesystem::file_time_type currentFileWriteTime = std::filesystem::last_write_time(mStringFolderPath);

    if (currentFileWriteTime != mStringLastWriteTime)
    {
        mStringLastWriteTime = currentFileWriteTime;
        mStringCachedContents.clear();

        for (const auto &fileEntry : std::filesystem::directory_iterator(mStringFolderPath))
        {
            StringMgr::instance()->LoadStrings(fileEntry.path().filename().string(), fileEntry.path().filename().string());
            mStringCachedContents.emplace_back(fileEntry.path().filename().string());
        }
    }
}

void EditorMgr::CreateEditorUI()
{
    if (!&io)
        io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Create New Scene"))
                    NodeMgr::instance()->ClearAllNodes();

                if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
                    NodeMgr::instance()->SaveToFile();

                ImGui::MenuItem("Open Scene", "Ctrl+O");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Create Node"))
                {
                    std::string nodeKey = "Node (" + std::to_string(NodeMgr::instance()->GetNodeCount() + 1) + ")";
                    rio::Vector3f defaultRotAndPos = {0, 0, 0};
                    rio::Vector3f defaultScale = {1, 1, 1};
                    auto createdNode = std::make_shared<Node>(nodeKey, defaultRotAndPos, defaultRotAndPos, defaultScale);
                    NodeMgr::instance()->AddNode(createdNode);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window"))
            {
                ImGui::MenuItem(mTextureWindowName.c_str(), NULL, &mTextureWindowEnabled);
                ImGui::MenuItem(mStringWindowName.c_str(), NULL, &mStringsWindowEnabled);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGuiID dockspace_id = ImGui::GetID("Dockspace");
        ImGuiViewport *viewport = ImGui::GetMainViewport();

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {0, 0});
        ImGui::Begin("Dockspace", nullptr, windowFlags);
        ImGui::PopStyleVar(5);

        ImGui::DockSpace(dockspace_id, {0, 0});

        if (ImGui::Begin("Layout", nullptr, ImGuiWindowFlags_NoCollapse))
        {
            if (ImGui::BeginChild("nodes", ImGui::GetContentRegionAvail(), ImGuiChildFlags_AutoResizeY))
            {
                for (const auto &it : NodeMgr::instance()->mNodes)
                {
                    std::shared_ptr<Node> node = it.second;

                    if (!node || !node->nodeKey.c_str())
                        continue;

                    bool isNodeSelected = (EditorMgr::instance()->mSelectedNode == node);

                    if (isNodeSelected)
                    {
                        BindRenderBuffer();
                        rio::PrimitiveRenderer::instance()->begin();

                        rio::PrimitiveRenderer::CubeArg cubeArg;
                        cubeArg.setCenter(mSelectedNode->GetPosition());
                        cubeArg.setSize(mSelectedNode->GetScale());
                        cubeArg.setColor({1, 1, 1, 1});

                        rio::PrimitiveRenderer::instance()->drawWireCube(cubeArg);

                        rio::PrimitiveRenderer::instance()->end();
                        UnbindRenderBuffer();
                    }

                    if (isNodeSelected)
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);

                    if (ImGui::Button(node->nodeKey.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 22)))
                        EditorMgr::instance()->mSelectedNode = node;

                    if (isNodeSelected)
                        ImGui::PopStyleColor();
                }

                ImGui::EndChild();
            }

            ImGui::End();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {0, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        if (ImGui::Begin("Task View"))
        {
            // Desired aspect ratio
            const float desiredAspectRatio = 1280.0f / 720.0f;

            // Get available space
            ImVec2 availSize = ImGui::GetContentRegionAvail();

            // Calculate the new size keeping the aspect ratio
            float newWidth = availSize.x;
            float newHeight = newWidth / desiredAspectRatio;

            if (newHeight > availSize.y)
            {
                newHeight = availSize.y;
                newWidth = newHeight * desiredAspectRatio;
            }

            // Center the image
            ImVec2 centerPos = {(availSize.x - newWidth) * 0.5f, (availSize.y - newHeight) * 0.5f};

            ImGui::SetCursorPos(centerPos);

            // Display the texture
            ImGui::Image((void *)mpColorTexture->getNativeTextureHandle(), ImVec2(newWidth, newHeight), ImVec2(0, 0), ImVec2(1, 1));

            ImGui::End();
        }
        ImGui::PopStyleVar(4);

        if (ImGui::Begin("Properties"))
        {
            if (mSelectedNode)
            {
                CreateNodePropertiesMenu();

                ImGui::End();
            }
        }

        if (mTextureWindowEnabled)
            DrawTexturesWindow();

        if (mStringsWindowEnabled)
            DrawStringsWindow();

        ImGui::End();
    }

    ImGui::ShowDemoWindow();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorMgr::DrawStringsWindow()
{
    UpdateStringsDirCache();

    ImGuiWindowFlags flags = 0;
    if (mStringsUnsaved)
        flags |= ImGuiWindowFlags_UnsavedDocument;

    if (ImGui::Begin(mStringWindowName.c_str(), __null, flags | ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save"))
                {
                    for (const auto &string : mStringCachedContents)
                        StringMgr::instance()->SaveStrings(string, string);

                    mStringsUnsaved = false;
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("String"))
            {
                if (mSelectedStringsID.empty())
                    ImGui::BeginDisabled();

                if (ImGui::MenuItem("Add String"))
                {
                    StringMgr::instance()->mAllStrings[mSelectedStringsID]["New String (" + std::to_string(StringMgr::instance()->mAllStrings[mSelectedStringsID].size()) + ")"] = "Hello World!";
                    mStringsUnsaved = true;
                }

                if (mSelectedStringsID.empty())
                    ImGui::EndDisabled();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        if (ImGui::BeginChild("stringFiles", ImVec2(160, 0), true))
        {
            for (const auto &stringFile : mStringCachedContents)
            {
                if (ImGui::Selectable(stringFile.c_str(), mSelectedStringsID == stringFile))
                    mSelectedStringsID = stringFile;
            }

            ImGui::EndChild();
        }

        ImGui::SameLine();

        if (ImGui::BeginChild("strings", ImVec2(160, 0), true))
        {
            auto &stringsMap = StringMgr::instance()->mAllStrings[mSelectedStringsID];
            for (auto it = stringsMap.begin(); it != stringsMap.end();)
            {
                const auto &string = *it;

                if (ImGui::Selectable(string.first.c_str()))
                    mSelectedStringKey = string.first;

                if (ImGui::BeginPopupContextItem(string.first.c_str()))
                {
                    if (ImGui::MenuItem("Delete"))
                    {
                        if (mSelectedStringKey == string.first)
                            mSelectedStringKey.clear();

                        it = stringsMap.erase(it);
                        mStringsUnsaved = true;
                        ImGui::EndPopup();
                        continue;
                    }

                    ImGui::EndPopup();
                }

                ++it;
            }

            ImGui::EndChild();
        }

        ImGui::SameLine();

        if (ImGui::BeginChild("string", ImVec2(0, 0), true))
        {
            if (StringMgr::instance()->mAllStrings[mSelectedStringsID].find(mSelectedStringKey) != StringMgr::instance()->mAllStrings[mSelectedStringsID].end())
            {
                auto it = StringMgr::instance()->mAllStrings[mSelectedStringsID].find(mSelectedStringKey);

                if (it != StringMgr::instance()->mAllStrings[mSelectedStringsID].end())
                {
                    std::string newKey = it->first;

                    ImGui::PushID("string_info");
                    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::InputText("", &newKey, ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        if (newKey != it->first)
                        {
                            auto value = it->second;
                            StringMgr::instance()->mAllStrings[mSelectedStringsID].erase(it);
                            StringMgr::instance()->mAllStrings[mSelectedStringsID][newKey] = value;
                            mSelectedStringKey = newKey;
                        }

                        mStringsUnsaved = true;
                    }
                    ImGui::PopID();
                    ImGui::PopItemWidth();
                }

                ImGui::PushID("string");
                ImGui::InputTextMultiline("", &StringMgr::instance()->mAllStrings[mSelectedStringsID][mSelectedStringKey], ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_CallbackEdit, mInstance->UnsavedStringCallback);
                ImGui::PopID();
            }

            ImGui::EndChild();
        }

        ImGui::End();
    }
}

void EditorMgr::DrawTexturesWindow()
{
    UpdateTexturesDirCache();

    if (ImGui::Begin(mTextureWindowName.c_str()))
    {
        if (ImGui::BeginChild("textures", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y}))
        {
            for (const auto &textureFilePath : mTextureCachedContents)
            {
                bool isTextureSelected = mTextureSelected == textureFilePath;

                if (isTextureSelected)
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);

                if (!mTextures[textureFilePath.string()].get())
                    ImGui::BeginDisabled();

                if (ImGui::Button(textureFilePath.filename().string().c_str(), {ImGui::GetContentRegionAvail().x, 25}))
                {
                    mTextureSelected = textureFilePath;
                }

                if (!mTextures[textureFilePath.string()].get())
                    ImGui::EndDisabled();

                if (isTextureSelected)
                    ImGui::PopStyleColor(1);
            }
            ImGui::EndChild();
        }
        ImGui::End();

        if (ImGui::Begin("Texture"))
        {
            if (!mTextureSelected.empty())
            {
                auto textureIter = mTextures.find(mTextureSelected);

                if (textureIter == mTextures.end())
                {
                    RIO_LOG("[EDITORMGR] Texture not found: %s\n", mTextureSelected.c_str());
                    return;
                }

                auto &texture = textureIter->second;

                if (!texture || !texture->getNativeTextureHandle())
                {
                    RIO_LOG("[EDITORMGR] Error loading %s.", mTextureSelected.c_str());
                    return;
                }

                if (ImGui::BeginChild("texture_info", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 3}))
                {
                    ImGui::Text("%s", mTextureSelected.filename().c_str());
                    ImGui::Text("Size: %d x %d", texture->getWidth(), texture->getHeight());
                    ImGui::Text("Mipmap Count: %d", texture->getNumMips());
                    ImGui::Text("Comp Map: %d", texture->getCompMap());
                    rio::TextureFormat texFormat = texture->getTextureFormat();
                    std::string stringFormat = mTextureFormatMap.find(texFormat)->second;
                    ImGui::Text("Texture Format: %s", stringFormat.c_str());

                    ImGui::EndChild();
                }

                if (ImGui::BeginChild("texture_display", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y}))
                {
                    // Desired aspect ratio
                    const float desiredAspectRatio = 200.0f / 200.0f;

                    // Get available space
                    ImVec2 availSize = ImGui::GetContentRegionAvail();

                    // Calculate the new size keeping the aspect ratio
                    float newWidth = availSize.x;
                    float newHeight = newWidth / desiredAspectRatio;

                    if (newHeight > availSize.y)
                    {
                        newHeight = availSize.y;
                        newWidth = newHeight * desiredAspectRatio;
                    }

                    // Center the image
                    ImVec2 centerPos = {(availSize.x - newWidth) * 0.5f, (availSize.y - newHeight) * 0.5f};

                    ImGui::SetCursorPos(centerPos);
                    ImGui::Image(reinterpret_cast<void *>(texture->getNativeTextureHandle()), {newHeight, newHeight});

                    ImGui::EndChild();
                }
            }

            ImGui::End();
        }
    }

    ImGui::End();
}

void EditorMgr::CreateNodePropertiesMenu()
{
    // ImGui::InputText(EditorMgr::instance()->selectedNode->nodeKey, EditorMgr::instance()->selectedNode->nodeKey, sizeof(EditorMgr::instance()->selectedNode->nodeKey));

    std::shared_ptr<Node> selectedNode = EditorMgr::instance()->mSelectedNode;

    selectedNode->CreateNodeProperties();

    if (selectedNode->properties.size() > 0)
    {
        if (ImGui::CollapsingHeader("Properties"))
        {
            for (const auto &property : selectedNode->properties)
                property->CreatePropertiesMenu();
        }
    }
}