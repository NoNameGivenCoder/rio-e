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
#include <filesystem>

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
        ConvertDDSToGtx();

        mTexturesLastWriteTime = currentFileWriteTime;
        mTextureCachedContents.clear();
        mTextures.clear();

        for (const auto &fileEntry : std::filesystem::directory_iterator(mTextureFolderPath))
        {
            // We're only loading .rtx, since that is the file format for pc.
            if (fileEntry.path().string().find(".rtx") == std::string::npos)
                continue;

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
            std::unique_ptr<rio::Texture2D> texture = std::make_unique<rio::Texture2D>(fileBuffer, fileEntry.file_size());

            mTextures[fileEntry.path().string()] = std::move(texture);
            mTextureCachedContents.push_back(fileEntry.path());

            delete[] fileBuffer;
        }
    }
}

void EditorMgr::ConvertDDSToGtx()
{
    DDSHeader ddsHeader;
    std::array<int, 6> compSelArray;

    rio::FileDevice::LoadArg arg;
    arg.path = mTextureFolderPath + "/emerald.dds";
    u8 *fileBuffer = rio::FileDeviceMgr::instance()->getNativeFileDevice()->load(arg);

    bool ddsReadResult = DDSReadFile(fileBuffer, &ddsHeader);

    std::vector<uint8_t> buffer(fileBuffer, fileBuffer + arg.read_size);

    RIO_LOG("[EDITORMGR] DDS Result: %d\n", (int)(ddsReadResult));

    if (ddsHeader.width <= 0 || ddsHeader.height <= 0 || ddsHeader.size <= 0)
    {
        RIO_LOG("[EDITORMGR] Width, height, and/or size is less than or equal to 0. %d x %d, %d\n", ddsHeader.width, ddsHeader.height, ddsHeader.size);
        return;
    }

    if (ddsHeader.depth > 1 || ddsHeader.caps2 & DDS_CAPS2_VOLUME)
    {
        RIO_LOG("[EDITORMGR] 3D Textures are not supported for conversion! %d | %d\n", ddsHeader.depth, (int)(ddsHeader.caps2));
        return;
    }

    if (ddsHeader.caps2 & (DDS_CAPS2_CUBE_MAP | DDS_CAPS2_CUBE_MAP_POSITIVE_X | DDS_CAPS2_CUBE_MAP_NEGATIVE_X | DDS_CAPS2_CUBE_MAP_POSITIVE_Y | DDS_CAPS2_CUBE_MAP_NEGATIVE_Y | DDS_CAPS2_CUBE_MAP_POSITIVE_Z | DDS_CAPS2_CUBE_MAP_NEGATIVE_Z))
    {
        RIO_LOG("[EDITORMGR] Cube maps are not supported for conversion! %d\n", (int)(ddsHeader.caps2));
        return;
    }

    if (ddsHeader.pixelFormat.flags & DDS_PIXEL_FORMAT_FLAGS_YUV)
    {
        RIO_LOG("[EDITORMGR] YUV color space is not supported for conversion! %d\n", (int)(ddsHeader.pixelFormat.flags));
        return;
    }

    int width = ddsHeader.width;
    int height = ddsHeader.height;
    int numMips = ddsHeader.mipMapCount;
    int imageSize, format_;
    GX2SurfaceFormat gx2SurfaceFormat;

    // Uncompressed DDS Format
    if (!ddsHeader.pixelFormat.flags & DDS_PIXEL_FORMAT_FLAGS_FOUR_CC)
    {
        return;
    }
    else
    {
        RIO_LOG("[EDITORMGR] Compressed DDS\n");

        if (ddsHeader.pixelFormat.fourCC == "DX10")
        {
            RIO_LOG("[EDITORMGR] DX10 DDS files are not supported for conversion!\n");
            return;
        }
        else if (!(fourCCs.find(std::string(ddsHeader.pixelFormat.fourCC)) != fourCCs.end()))
        {
            RIO_LOG("[EDITORMGR] Unsupported pixel format! %s\n", ddsHeader.pixelFormat.fourCC);
            return;
        }

        auto arr = fourCCs.at(std::string(ddsHeader.pixelFormat.fourCC));
        format_ = arr[0];
        auto blockSize = arr[1];

        if (!(format_ & 4))
        {
            compSelArray = {0, 1, 2, 3, 4, 5};

            format_ |= 0x400;
        }
        else if ((format_ & 0x3F) == 0x34)
        {
            compSelArray = {0, 4, 4, 5, 4, 5};
        }
        else if ((format_ & 0x3F) == 0x35)
        {
            compSelArray = {0, 1, 4, 5, 4, 5};
        }

        gx2SurfaceFormat = GX2SurfaceFormat(format_);

        imageSize = ((width + 3) >> 2) * ((height + 3) >> 2) * blockSize;
    }

    std::vector<uint8_t> imageData(buffer.begin() + ddsHeader.size, buffer.begin() + ddsHeader.size + imageSize);
    std::vector<uint8_t> mipData(buffer.begin() + ddsHeader.size + imageSize, buffer.end());

    int compSel = (compSelArray[0] << 24 |
                   compSelArray[1] << 16 |
                   compSelArray[2] << 8 |
                   compSelArray[3]);

    GX2Texture gx2Texture = GX2Texture();

    gx2Texture.surface.aa = GX2_AA_MODE_1X;
    gx2Texture.surface.dim = GX2_SURFACE_DIM_2D;
    gx2Texture.surface.width = width;
    gx2Texture.surface.height = height;
    gx2Texture.surface.depth = 1;
    gx2Texture.surface.numMips = numMips;
    gx2Texture.surface.format = gx2SurfaceFormat;
    gx2Texture.surface.use = GX2_SURFACE_USE_TEXTURE;
    gx2Texture.surface.tileMode = GX2_TILE_MODE_LINEAR_SPECIAL;
    gx2Texture.surface.swizzle = 0;
    gx2Texture.compSel = compSel;

    GX2CalcSurfaceSizeAndAlignment(&gx2Texture.surface);

    u8 *imageBufferData = new u8[imageData.size()];
    u8 *mipBufferData = new u8[mipData.size()];

    std::copy(imageData.begin(), imageData.end(), imageBufferData);
    std::copy(mipData.begin(), mipData.end(), mipBufferData);

    gx2Texture.surface.imagePtr = imageBufferData;
    gx2Texture.surface.mipPtr = mipBufferData;

    // After here we can make a new function for this

    GFDFile *gfdFile = new GFDFile();
    gfdFile->mTextures.emplace_back(gx2Texture);

    int blockMajorVersion = 0;
    int blockMinorVersion = 1;
    bool align = (gfdFile->mHeader.alignMode == GFD_ALIGN_MODE_ENABLE);
    int pos = 0;
    bool endian = true;
    bool serialized = true;

    std::vector<u8> outBuffer;

    void *blockHeaderData = rio::MemUtil::alloc(sizeof(GFDBlockHeader), 0);
    void *gfdHeaderBuffer = rio::MemUtil::alloc(sizeof(GFDHeader), 0);
    void *gx2TextureData = rio::MemUtil::alloc(sizeof(GX2Texture), 0);

    SaveGFDHeader(gfdHeaderBuffer, &gfdFile->mHeader, serialized, endian);
    addToBuffer(gfdHeaderBuffer, gfdFile->mHeader.size, &outBuffer);
    pos += gfdFile->mHeader.size;

    size_t blockHeaderSize = sizeof(GFDBlockHeader);
    size_t gx2TextureSize = sizeof(GX2Texture);

    GFDBlockHeader blockHeader = GFDBlockHeader();
    if (gfdFile->mHeader.majorVersion == 6 && gfdFile->mHeader.minorVersion == 0)
    {
        blockMajorVersion = 0;
        blockMinorVersion = 1;
    }
    else
    {
        blockMajorVersion = 1;
        blockMinorVersion = 0;
    }

    blockHeader.majorVersion = blockMajorVersion;
    blockHeader.minorVersion = blockMinorVersion;

    for (const auto &texture : gfdFile->mTextures)
    {
        blockHeader.type = GFD_BLOCK_TYPE_V1_GX2_TEX_HEADER;
        blockHeader.dataSize = gx2TextureSize;

        // Converted
        SaveGFDBlockHeader(blockHeaderData, &blockHeader, serialized, endian);
        addToBuffer(blockHeaderData, blockHeaderSize, &outBuffer);
        pos += blockHeaderSize;

        // Converted
        SaveGX2Texture(gx2TextureData, &texture, serialized, endian);
        addToBuffer(gx2TextureData, gx2TextureSize, &outBuffer);
        pos += gx2TextureSize;

        if (align)
        {
            int dataPos = pos + blockHeaderSize * 2;
            int padSize = RoundUp(dataPos, texture.surface.alignment) - dataPos;

            blockHeader.type = GFD_BLOCK_TYPE_V1_PAD;
            blockHeader.dataSize = padSize;

            // Converted
            SaveGFDBlockHeader(blockHeaderData, &blockHeader, serialized, endian);
            addToBuffer(blockHeaderData, blockHeaderSize, &outBuffer);
            pos += blockHeaderSize;

            outBuffer.resize(outBuffer.size() + padSize, 0);
            pos += padSize;
        }

        blockHeader.type = GFD_BLOCK_TYPE_V1_GX2_TEX_IMAGE_DATA;
        blockHeader.dataSize = texture.surface.imageSize;

        // Converted
        SaveGFDBlockHeader(blockHeaderData, &blockHeader, serialized, endian);
        addToBuffer(blockHeaderData, blockHeaderSize, &outBuffer);
        pos += blockHeaderSize;

        size_t oldSize = outBuffer.size();
        outBuffer.resize(oldSize + texture.surface.imageSize);
        std::memcpy(outBuffer.data() + oldSize, texture.surface.imagePtr, texture.surface.imageSize);
        pos += texture.surface.imageSize;

        if (texture.surface.mipPtr)
        {
            if (align)
            {
                int dataPos = pos + blockHeaderSize * 2;
                int padSize = RoundUp(dataPos, texture.surface.alignment) - dataPos;

                blockHeader.type = GFD_BLOCK_TYPE_V1_PAD;
                blockHeader.dataSize = padSize;

                SaveGFDBlockHeader(blockHeaderData, &blockHeader, serialized, endian);
                addToBuffer(blockHeaderData, sizeof(GFDBlockHeader), &outBuffer);
                pos += blockHeaderSize;

                outBuffer.resize(outBuffer.size() + padSize, 0);
                pos += padSize;
            }

            blockHeader.type = GFD_BLOCK_TYPE_V1_GX2_TEX_MIP_DATA;
            blockHeader.dataSize = texture.surface.mipSize;

            SaveGFDBlockHeader(blockHeaderData, &blockHeader, serialized, endian);
            addToBuffer(blockHeaderData, sizeof(GFDBlockHeader), &outBuffer);
            pos += blockHeaderSize;

            size_t oldSize = outBuffer.size();
            outBuffer.resize(oldSize + texture.surface.mipSize);
            std::memcpy(outBuffer.data() + oldSize, texture.surface.mipPtr, texture.surface.mipSize);
            pos += texture.surface.mipSize;
        }
    }

    blockHeader.type = GFD_BLOCK_TYPE_END;
    blockHeader.dataSize = 0;

    SaveGFDBlockHeader(blockHeaderData, &blockHeader, serialized, endian);
    addToBuffer(blockHeaderData, sizeof(GFDBlockHeader), &outBuffer);
    pos += blockHeaderSize;

    rio::FileDeviceMgr::instance()->getNativeFileDevice()->open(&fileHandle, "fullTest.gtx", rio::FileDevice::FILE_OPEN_FLAG_WRITE);
    rio::FileDeviceMgr::instance()->getNativeFileDevice()->write(&fileHandle, outBuffer.data(), outBuffer.size());
    rio::FileDeviceMgr::instance()->getNativeFileDevice()->close(&fileHandle);

    rio::MemUtil::free(imageBufferData);
    rio::MemUtil::free(mipBufferData);
    rio::MemUtil::free(blockHeaderData);
    rio::MemUtil::free(gfdHeaderBuffer);
    rio::MemUtil::free(gx2TextureData);
}

void EditorMgr::ConvertGtxToRtx()
{
    rio::FileDevice::LoadArg arg;
    arg.path = mTextureFolderPath + "/emerald.gtx";
    u8 *fileBuffer = rio::FileDeviceMgr::instance()->getNativeFileDevice()->load(arg);

    RIO_LOG("%d\n", arg.read_size);

    if (!fileBuffer)
        return;

    GFDFile *gfd = new GFDFile();
    size_t fileSize = gfd->load(fileBuffer);

    RIO_LOG("%d\n", fileSize);

    // Not equal to one
    if (gfd->mTextures.size() != 1)
        return;

    GX2Texture texture = gfd->mTextures[0];

    if (!&texture)
        return;

    GX2Texture linear_texture;
    LoadGX2Texture(&texture, &linear_texture, true, false);
    LoadGX2Surface(&texture.surface, &linear_texture.surface, true, false);

    GX2TexturePrintInfo(&linear_texture);
    GX2SurfacePrintInfo(&linear_texture.surface);

    rio::Texture2D *rioTexture = new rio::Texture2D((rio::TextureFormat)(linear_texture.surface.format), linear_texture.surface.width, linear_texture.surface.height, linear_texture.surface.numMips);

    rioTexture->setCompMap(linear_texture.compSel);

    if (rioTexture->getNumMips() > 1)
    {
        if (linear_texture.surface.mipOffset[0] == rioTexture->getNativeTexture().surface.imageSize)
            return;
    }

    rio::MemUtil::free(rioTexture);
    rio::MemUtil::free(gfd);
    rio::MemUtil::free(fileBuffer);
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
                for (const auto &node : NodeMgr::instance()->mNodes)
                {
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

                        if (ImGui::Button(textureFilePath.filename().string().c_str(), {ImGui::GetContentRegionAvail().x, 25}))
                        {
                            mTextureSelected = textureFilePath;
                        }

                        if (isTextureSelected)
                            ImGui::PopStyleColor(1);
                    }
                    ImGui::EndChild();
                }
                ImGui::End();
            }

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

            ImGui::End();
        }

        ImGui::End();
    }

    ImGui::ShowDemoWindow();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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