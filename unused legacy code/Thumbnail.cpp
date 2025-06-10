#include "../../include/GUI/Thumbnail.hpp"
#include <stdio.h>  
#include <cmath>
#include <thread>
#include <functional>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"
#include "../../external/stb/stb_image_resize2.h"
#include "../../external/stb/stb_image_write.h"

/*
 *  Return value is void because the texture is created on a separate thread.
    The thread sets the member variable status that is later checked.
 *  The function deletes the existing texture if one already exists.
 *  If the function is destroyed by the user, they must use Thumbnail::destroy()
    or an equivalent function.
 */
namespace NxSh::GUI {
    vint Thumbnail::_create(const char* const filename) {
        if (this->thumbnail) {
            this->destroy();
            this->status = NXSH_THUMBNAIL_UNINITIALIZED;
        }

        int channels = 0;
        int _w = 0, _h = 0;
        unsigned char* data = stbi_load(filename, &_w, &_h, &channels, 4);
        if (!data) {
            fprintf(stderr, "Failed to load image %s\n", stbi_failure_reason());
            this->status = -1;
        }
        fprintf(stderr, "Texture requires %d bytes and has %d channels.\n", channels * _w * _h , channels);
        unsigned char* resized = nullptr;

        if (_h > 512) {
            int fixed_height = 512;
            int fixed_width = static_cast<int>(std::round(_w * (512.0f / _h)));

            printf("New width: %d\n", fixed_width);
            printf("Resized image requires %d bytes.\n", fixed_width * fixed_height * 4);

            resized = new unsigned char[fixed_width * fixed_height * 4];

            int stride = _w * 4;
            int stride_new = fixed_width * 4;

            if (!stbir_resize_uint8_linear(data, _w, _h, stride, resized, fixed_width, fixed_height,
                                      stride_new, static_cast<stbir_pixel_layout>(4))) {
                fprintf(stderr, "Image resize failed.\n");
            }

            stbi_write_png("img.png", fixed_width, fixed_height, 4, resized, fixed_width * 4);

            _w = fixed_width;
            _h = fixed_height;
        }

        stbi_image_free(data);

        GLuint tmp;
        glGenTextures(1, &tmp);
        glBindTexture(GL_TEXTURE_2D, tmp);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _w, _h, 0, GL_RGBA, GL_UNSIGNED_BYTE, resized);

        delete[] resized;

        this->thumbnail = tmp;
        this->w = _w;
        this->h = _h;

        this->status = 0;

        // TODO: implement keep aspect ratio, padding and compression
    }

    vint Thumbnail::create(const char* const filename) {
        thread generator([this, filename]() {
            _create(filename);
        });
        generator.detach();
    }

    const GLuint* const Thumbnail::get() const {
        return &this->thumbnail;
    }

    inline int Thumbnail::get_width() const {
        return this->w;
    }

    inline int Thumbnail::get_height() const {
        return this->h;
    }

    inline bool Thumbnail::is_null() const {
        return !this->thumbnail;
    }

    inline bool Thumbnail::is_uninit() const {
        return this->is_null()                              ||
               this->status == NXSH_THUMBNAIL_UNINITIALIZED ||
               this->status == -1;
    }

    void Thumbnail::destroy() {
        if (this->thumbnail) {
            glDeleteTextures(1, &this->thumbnail);
            this->thumbnail = NULL;
        }
        this->status = 0xFFFF;
    }
}