#include <helpers/editor/EditorMgr.h>
#include <helpers/properties/Property.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <gfx/rio_PrimitiveRenderer.h>
#include <helpers/common/NodeMgr.h>
#include <gfx/rio_Window.h>
#include <iostream>
#include <gpu/rio_RenderBuffer.h>
#include <misc/rio_MemUtil.h>
#include <filesystem>
#include <helpers/editor/Texture2DUtil.h>
#include <misc/cpp/imgui_stdlib.h>
#include <span>
#include <memory>
#include <imfilebrowser.h>

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
    mInstance->mModelsFolderPath = mInstance->mFileDevice->getNativePath("models");
    mInstance->fileBrowser = new ImGui::FileBrowser();

    return true;
}

bool EditorMgr::destorySingleton()
{
    if (!mInstance)
        return false;

    delete mInstance->mpColorTexture;
    delete mInstance->mpDepthTexture;
    delete mInstance->fileBrowser;

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

void EditorMgr::UpdateModelsDirCache()
{
    std::filesystem::file_time_type currentFileWriteTime = std::filesystem::last_write_time(mModelsFolderPath);

    if (currentFileWriteTime != mModelsLastWriteTime)
    {
        mModelsLastWriteTime = currentFileWriteTime;
        mModelsCachedContent.clear();

        for (const auto &fileEntry : std::filesystem::directory_iterator(mModelsFolderPath))
        {
            if (fileEntry.path().extension().string() == ".rmdl")
                mModelsCachedContent.emplace(fileEntry.path().filename().string(), MODEL_TYPE_RMDL);
            else if (fileEntry.path().extension().string() == ".obj")
                mModelsCachedContent.emplace(fileEntry.path().filename().string(), MODEL_TYPE_OBJ);
            else
                mModelsCachedContent.emplace(fileEntry.path().filename().string(), MODEL_TYPE_UNK);
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
                ImGui::MenuItem(mModelsWindowName.c_str(), NULL, &mModelsWindowEnabled);
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
            ImGui::Image((void *)(uint32_t)mpColorTexture->getNativeTextureHandle(), ImVec2(newWidth, newHeight), ImVec2(0, 0), ImVec2(1, 1));

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

        if (mModelsWindowEnabled)
            DrawModelsWindow();

        ImGui::End();
    }

    if (mStringsNewModal)
        ImGui::OpenPopup("New Strings File..");

    ImGui::SetNextWindowSize(ImVec2(400, 130));
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetWindowSize().x - 400) / 2, (ImGui::GetWindowSize().y - 130) / 2));
    if (ImGui::BeginPopupModal("New Strings File..", __null, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("Strings Filename");
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("String Input", &mStringsNewModalFileName);

        float buttonWidth = ImGui::GetContentRegionAvail().x / 2;

        ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 40);

        // Close button
        if (ImGui::Button("Close", ImVec2(buttonWidth, 30)))
        {
            mStringsNewModal = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        // Create Strings File button
        if (ImGui::Button("Create Strings File", ImVec2(buttonWidth, 30)))
        {
            mFileDevice->open(&fileHandle, "lang/" + mStringsNewModalFileName + ".yaml", rio::FileDevice::FILE_OPEN_FLAG_CREATE);
            mFileDevice->write(&fileHandle, reinterpret_cast<u8 *>((char *)("strings:\n New String (0): Hello World!")), strlen("strings:\n New String (0): Hello World!"));
            mFileDevice->close(&fileHandle);

            mStringsNewModal = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::ShowDemoWindow();

    fileBrowser->Display();

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
                if (ImGui::MenuItem("New Strings File.."))
                {
                    mStringsNewModalFileName.clear();
                    mStringsNewModal = true;
                }

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
    if (ImGui::Begin(mTextureWindowName.c_str(), __null, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open"))
                {
                    fileBrowser->SetTypeFilters({".gtx", ".dds", ".rtx"});
                    fileBrowser->Open();
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        if (fileBrowser->HasSelected())
        {
            auto fileEntry = fileBrowser->GetSelected();

            rio::FileDevice::LoadArg arg;
            arg.path = fileEntry.string();
            u8 *fileBuffer = rio::FileDeviceMgr::instance()->getNativeFileDevice()->load(arg);

            if (!fileBuffer)
            {
                RIO_LOG("[EDITORMGR] Failed to read texture file: %s\n", fileEntry.string().c_str());
                rio::MemUtil::free(fileBuffer);
                return;
            }

            if (fileEntry.extension().string() == ".rtx")
            {
                std::unique_ptr<rio::Texture2D> texture = std::make_unique<rio::Texture2D>(fileBuffer, arg.read_size);

                mTextures[fileEntry.string()] = std::move(texture);
                mTextureCachedContents.push_back(fileEntry.string());

                rio::MemUtil::free(fileBuffer);
            }

            // Thank you https://github.com/aboood40091/Miyamoto-Next
            if (fileEntry.extension().string() == ".gtx")
            {
                std::unique_ptr<rio::Texture2D> texture;

                Texture2DUtil::createFromGTX(fileBuffer, &texture);

                mTextures[fileEntry.string()] = std::move(texture);
                mTextureCachedContents.push_back(fileEntry.string());

                rio::MemUtil::free(fileBuffer);
            }

            if (fileEntry.extension().string() == ".dds")
            {
                // Saving GTX Texture
                GX2Texture texture;
                GFDFile *gfd = new GFDFile();

                GX2TextureFromDDS(&texture, fileBuffer, arg.read_size);

                gfd->mTextures.push_back(texture);
                const std::vector<u8> &gfd_data = gfd->saveGTX();

                // Saving RTX Texture
                std::unique_ptr<rio::Texture2D> rioTexture;
                Texture2DUtil::createFromGTX(gfd_data.data(), &rioTexture);

                RIO_LOG("%s\n", fileEntry.filename().c_str());
                RIO_LOG("%s\n", fileEntry.c_str());
                // mFileDevice->open(&fileHandle, )

                rioTexture.reset();
                rio::MemUtil::free(gfd);
                rio::MemUtil::free(fileBuffer);
            }

            fileBrowser->ClearSelected();
        }

        if (ImGui::BeginChild("textures", ImVec2(ImGui::GetContentRegionAvail().x / 2, 0)), true)
        {
            if (ImGui::TreeNode("textures"))
            {
                for (const auto &textureFilePath : mTextureCachedContents)
                {
                    bool isTextureSelected = mTextureSelected == textureFilePath;

                    if (isTextureSelected)
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);

                    if (!mTextures[textureFilePath.string()].get())
                        ImGui::BeginDisabled();

                    if (ImGui::Button(textureFilePath.filename().string().c_str(), {ImGui::GetContentRegionAvail().x, 22}))
                        mTextureSelected = textureFilePath;

                    if (!mTextures[textureFilePath.string()].get())
                        ImGui::EndDisabled();

                    if (isTextureSelected)
                        ImGui::PopStyleColor(1);
                }
            }

            ImGui::EndChild();
        }

        ImGui::SameLine();

        if (ImGui::BeginChild("texture_preview", ImVec2(0, 0), true))
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
        }
    }

    ImGui::End();
}

void EditorMgr::DrawModelsWindow()
{
    UpdateModelsDirCache();
    if (ImGui::Begin(mModelsWindowName.c_str(), NULL, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            ImGui::MenuItem("Material");
            ImGui::EndMenuBar();
        }

        // Leftmost child (directory)
        if (ImGui::BeginChild("models_dir", ImVec2(175, 0), true))
        {
            if (ImGui::TreeNode("models"))
            {
                for (const auto &model : mModelsCachedContent)
                {
                    if (model.second != MODEL_TYPE_OBJ)
                        ImGui::BeginDisabled();

                    if (ImGui::Selectable(model.first.c_str()))
                    {
                        mPreviewModel = OBJToRioModel(mModelsFolderPath + "/" + model.first, mModelsFolderPath + "/BlueEmerald.mtl");
                    }

                    if (model.second != MODEL_TYPE_OBJ)
                        ImGui::EndDisabled();
                }

                ImGui::TreePop();
            }
            ImGui::EndChild();
        }

        ImGui::SameLine();

        // Middle child (options)
        if (ImGui::BeginChild("models_options", ImVec2(275, 0), true))
        {
            if (mPreviewModel)
            {
                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

                ImGui::SeparatorText("Mesh Settings");
                for (int i = 0; i < mPreviewModel->meshes.size(); i++)
                {
                    if (ImGui::TreeNode(std::string("Mesh (" + std::to_string(i) + ")").c_str()))
                    {
                        ImGui::Text("Vertex Count: %d", mPreviewModel->meshes[i].vertices.size());
                        ImGui::Text("Index Count: %d", mPreviewModel->meshes[i].indices.size());

                        ImGui::TreePop();
                    }
                }

                ImGui::SeparatorText("Material Settings");
                for (size_t i = 0; i < mPreviewModel->materials.size(); ++i)
                {
                    auto &material = mPreviewModel->materials[i];
                    if (ImGui::TreeNode(std::string("Material (" + material.name + ")").c_str()))
                    {
                        ImGui::Text("Name");
                        ImGui::InputText(("##material_name_" + std::to_string(i)).c_str(), &material.name);

                        ImGui::Text("Shader Name");
                        ImGui::InputText(("##material_shaderName_" + std::to_string(i)).c_str(), &material.shaderName);

                        ImGui::Text("Is Translucent");
                        ImGui::SameLine();
                        ImGui::Checkbox(("##material_translucent_" + std::to_string(i)).c_str(), &material.isTranslucent);

                        ImGui::Text("Is Visible");
                        ImGui::SameLine();
                        ImGui::Checkbox(("##material_visible_" + std::to_string(i)).c_str(), &material.isVisible);

                        ImGui::Text("Alpha Test Enable");
                        ImGui::SameLine();
                        ImGui::Checkbox(("##material_alpTestEnable_" + std::to_string(i)).c_str(), &material.alphaTestEnable);

                        for (size_t j = 0; j < material.textures.size(); ++j)
                        {
                            auto &texture = material.textures[j];

                            ImGui::PushID(std::string("texture_header_" + std::to_string(i) + "_" + std::to_string(j)).c_str());
                            if (ImGui::CollapsingHeader(std::string("Texture (" + std::to_string(j) + ")").c_str()))
                            {
                                ImGui::PopID();

                                ImGui::Text("Texture Name (*.gtx/*.rtx)");
                                ImGui::InputText(("##texture_name_" + std::to_string(i) + "_" + std::to_string(j)).c_str(), &texture.name);

                                ImGui::Text("Sampler Name");
                                ImGui::InputText(("##texture_samplerName_" + std::to_string(i) + "_" + std::to_string(j)).c_str(), &texture.samplerName);

                                ImGui::Text("Mag Filter");
                                if (ImGui::BeginCombo(("##texture_magFilter_" + std::to_string(i) + "_" + std::to_string(j)).c_str(), mModelsTexXYMap[texture.magFilter].c_str()))
                                {
                                    for (const auto &val : mModelsTexXYMap)
                                    {
                                        if (ImGui::Selectable(val.second.c_str()))
                                            texture.magFilter = val.first;

                                        if (val.first == texture.magFilter)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    ImGui::EndCombo();
                                }

                                ImGui::Text("Min Filter");
                                if (ImGui::BeginCombo(("##texture_minFilter_" + std::to_string(i) + "_" + std::to_string(j)).c_str(), mModelsTexXYMap[texture.minFilter].c_str()))
                                {
                                    for (const auto &val : mModelsTexXYMap)
                                    {
                                        if (ImGui::Selectable(val.second.c_str()))
                                            texture.minFilter = val.first;

                                        if (val.first == texture.minFilter)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    ImGui::EndCombo();
                                }

                                ImGui::Text("Mip Filter");
                                if (ImGui::BeginCombo(("##texture_mipFilter_" + std::to_string(i) + "_" + std::to_string(j)).c_str(), mModelsMipFilterMap[texture.mipFilter].c_str()))
                                {
                                    for (const auto &val : mModelsMipFilterMap)
                                    {
                                        if (ImGui::Selectable(val.second.c_str()))
                                            texture.mipFilter = val.first;

                                        if (val.first == texture.mipFilter)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    ImGui::EndCombo();
                                }

                                ImGui::Text("Max Aniso");
                                if (ImGui::BeginCombo(("##texture_maxAniso_" + std::to_string(i) + "_" + std::to_string(j)).c_str(), mModelsAnisoMap[texture.maxAniso].c_str()))
                                {
                                    for (const auto &val : mModelsAnisoMap)
                                    {
                                        if (ImGui::Selectable(val.second.c_str()))
                                            texture.maxAniso = val.first;

                                        if (val.first == texture.maxAniso)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    ImGui::EndCombo();
                                }

                                ImGui::Text("Wrap Mode (X)");
                                if (ImGui::BeginCombo(("##texture_wrapX_" + std::to_string(i) + "_" + std::to_string(j)).c_str(), mModelsWrapModeMap[texture.wrapX].c_str()))
                                {
                                    for (const auto &val : mModelsWrapModeMap)
                                    {
                                        if (ImGui::Selectable(val.second.c_str()))
                                            texture.wrapX = val.first;

                                        if (val.first == texture.wrapX)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    ImGui::EndCombo();
                                }

                                ImGui::Text("Wrap Mode (Y)");
                                if (ImGui::BeginCombo(("##texture_wrapY_" + std::to_string(i) + "_" + std::to_string(j)).c_str(), mModelsWrapModeMap[texture.wrapY].c_str()))
                                {
                                    for (const auto &val : mModelsWrapModeMap)
                                    {
                                        if (ImGui::Selectable(val.second.c_str()))
                                            texture.wrapY = val.first;

                                        if (val.first == texture.wrapY)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    ImGui::EndCombo();
                                }

                                ImGui::Text("Wrap Mode (Z)");
                                if (ImGui::BeginCombo(("##texture_wrapZ_" + std::to_string(i) + "_" + std::to_string(j)).c_str(), mModelsWrapModeMap[texture.wrapZ].c_str()))
                                {
                                    for (const auto &val : mModelsWrapModeMap)
                                    {
                                        if (ImGui::Selectable(val.second.c_str()))
                                            texture.wrapZ = val.first;

                                        if (val.first == texture.wrapZ)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    ImGui::EndCombo();
                                }
                            }
                        }

                        ImGui::TreePop();
                    }
                }

                ImGui::PopItemWidth();
            }

            ImGui::EndChild();
        }

        ImGui::SameLine();

        // Right side container
        ImGui::PushStyleColor(ImGuiCol_ChildBg, {255, 0, 0, 0});
        if (ImGui::BeginChild("right_container", ImVec2(0, 0), false))
        {
            ImGui::PopStyleColor();
            // Top child in the right section (preview)
            if (ImGui::BeginChild("model_preview", ImVec2(0, ImGui::GetWindowHeight() * 0.5f), true))
            {
                ImGui::EndChild();
            }

            // New child underneath the preview
            if (ImGui::BeginChild("model_additional", ImVec2(0, 0), true))
            {
                if (ImGui::Button("Export!"))
                {
                    SaveRioModel(*mPreviewModel.get(), false, mModelsFolderPath + "/test");
                }
                ImGui::EndChild();
            }

            ImGui::EndChild();
        }

        ImGui::PopStyleColor();

        ImGui::End();
    }
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