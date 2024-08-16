#include <rio.h>
#include <gfx/mdl/res/rio_ModelData.h>
#include <gfx/mdl/res/rio_MeshData.h>

#include <filedevice/rio_FileDeviceMgr.h>
#include <filedevice/rio_FileDevice.h>

#include <ninTexUtils/gfd/gfdStruct.h>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <algorithm>

class Vertex
{
public:
    Vertex(const std::tuple<float, float, float> &pos, const std::tuple<float, float> &tex, const std::tuple<float, float, float> &norm)
        : pos(pos), texCoord(tex), normal(norm) {}

    bool operator==(const Vertex &other) const
    {
        return pos == other.pos && texCoord == other.texCoord && normal == other.normal;
    }

private:
    std::tuple<float, float, float> pos;
    std::tuple<float, float> texCoord;
    std::tuple<float, float, float> normal;
};

class Mesh
{
public:
    u32 materialIdx = -1;

    std::vector<Vertex> vertices;
    std::vector<int> indices;

    float scale[3] = {1.0, 1.0, 1.0};
    float rotate[3] = {0.0, 0.0, 0.0};
    float translate[3] = {0.0, 0.0, 0.0};
};

struct OBJReturnResult
{
    std::vector<Vertex> vertices;
    std::vector<int> indices;
};

OBJReturnResult ReadOBJ(std::string fileName);
void OBJToRioModel(std::string fileName);

size_t ConvertGtxToRtx(u8 *fileBuffer, std::vector<u8> &fillBuffer);