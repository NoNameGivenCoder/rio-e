#pragma once

#include <gpu/rio_Texture.h>

#include <memory>
#include <span>

class Texture2DUtil
{
public:
    enum GTXError
    {
        GTX_ERROR_OK = 0,
        GTX_ERROR_SRC_TOO_SHORT,
        GTX_ERROR_SRC_INVALID,
        GTX_ERROR_SRC_EMPTY,
        GTX_ERROR_UNSUPPORTED_FORMAT
    };

public:
    static GTXError createFromGTX(const u8 *file_data, std::unique_ptr<rio::Texture2D> *pp_texture);
    static void destroy(std::unique_ptr<rio::Texture2D> *pp_texture);
};
