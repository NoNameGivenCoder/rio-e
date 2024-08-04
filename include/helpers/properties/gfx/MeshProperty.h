#ifndef MESHPROPERTY_H
#define MESHPROPERTY_H

#include <helpers/properties/Property.h>
#include <helpers/properties/map/CameraProperty.h>
#include <gfx/mdl/rio_Material.h>
#include <gfx/mdl/rio_Mesh.h>
#include <gfx/mdl/res/rio_ModelCacher.h>
#include <gfx/mdl/res/rio_ModelData.h>
#include <gfx/mdl/rio_Model.h>
#include <gpu/rio_Drawer.h>
#include <misc/rio_MemUtil.h>
#include <vector>

class MeshProperty : public Property
{
public:
    struct ViewBlock
    {
        rio::Vector3f view_pos;
        u32 _padding;
        rio::Matrix44f view_proj_mtx;
    };

    struct LightBlock
    {
        rio::Vector3f light_color;
        u32 _padding_0;
        rio::Vector3f light_pos;
        u32 _padding_1;
    };

    struct ModelBlock
    {
        rio::Matrix34f model_mtx;
        rio::Matrix34f normal_mtx;
        rio::BaseVec4u _padding[10];
    };

public:
    // All class members here will be accessible from any other properties within the task.
    MeshProperty(std::shared_ptr<Node> pParentNode) : Property(pParentNode), mMdlModel(nullptr) {};

    ~MeshProperty();

    void LoadMesh();

    // Called when the task is loading from YAML. Used for loading values into members of your property class.
    void Load(YAML::Node node) override;

    // Called when task starts. Used for initializing values, and preparing for rendering or controlling.
    void Start() override;

    // Called every frame.
    void Update() override;

    // Editor function. Do not use within normal gameplay.
    // Called when a task is saving. Used for saving values into a YAML node.
    YAML::Node Save() override;

    // Editor function. Do not use within normal gameplay.
    // Called when a property is selected within the editor. Used for creating ImGui UI to change default property values.
    void CreatePropertiesMenu() override;

    static ViewBlock sViewBlock;
    static LightBlock sLightBlock;
    std::unique_ptr<rio::mdl::Model> mMdlModel;

private:
    struct ShaderLocation
    {
        ShaderLocation(u32 in_vs = u32(-1), u32 in_fs = u32(-1), rio::UniformBlock::ShaderStage in_stage = rio::UniformBlock::STAGE_NONE)
            : vs(in_vs), fs(in_fs), stage(in_stage)
        {
        }

        void findStage()
        {
            if (vs != u32(-1))
            {
                if (fs != u32(-1))
                    stage = rio::UniformBlock::STAGE_ALL;

                else
                    stage = rio::UniformBlock::STAGE_VERTEX_SHADER;
            }
            else if (fs != u32(-1))
            {
                stage = rio::UniformBlock::STAGE_FRAGMENT_SHADER;
            }
            else
            {
                stage = rio::UniformBlock::STAGE_NONE;
            }
        }

        u32 vs;
        u32 fs;
        rio::UniformBlock::ShaderStage stage;
    };

    struct UniformBlocks
    {
        UniformBlocks()
            : view_block_idx(), light_block_idx()
        {
        }

        UniformBlocks(const ShaderLocation &in_view_block_idx, const ShaderLocation &in_light_block_idx)
            : view_block_idx{in_view_block_idx}, light_block_idx{in_light_block_idx}
        {
        }

        ShaderLocation view_block_idx;
        ShaderLocation light_block_idx;
    };

    // Private class members for use within your property.
    std::string mMeshFileName;
    std::string mMeshKey;

    CameraProperty *mCameraProperty;

    rio::UniformBlock *mModelUniformBlock;
    ModelBlock *mModelBlock;
    UniformBlocks *mUniformBlocks;
    rio::UniformBlock *mpViewUniformBlock;
    rio::UniformBlock *mpLightUniformBlock;
};

#endif // MESHPROPERTY_H