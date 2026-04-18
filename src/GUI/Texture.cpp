#include "Config/Config.hpp"
#include "GUI/Texture.hpp"

#include "../../external/stb/stb_image.h"
#include "../../external/stb/stb_image_resize2.h"
#include "../../external/stb/stb_image_write.h"

#include <stdio.h>
#include <thread>
#include <functional>
#include <cmath>

using std::string;

Texture::Texture(fs::path path, fs::path out_path) {
    create_from_file(path.generic_string(), out_path.generic_string().c_str());
}
Texture::Texture(const uint8_t* data, int size) {
    create_from_memory(data, size);
}

vint Texture::_load_from_file(string filename) {
    int channels;
    data = stbi_load(filename.c_str(), &this->w, &this->h, &channels, 4);
    if (!data) {
        fprintf(stderr, "Failed to load image %s\n", stbi_failure_reason());
        fprintf(stderr, "filename %s\n", filename.c_str());
        this->status = Status::Failure;
        return;
    }
    printf("Texture requires %d bytes.\n", 4 * this->w * this->h);
}

vint Texture::_load_from_memory(const uint8_t* raw, uint64_t size) {
    data = stbi_load_from_memory(raw, size, &this->w, &this->h, nullptr, 4);
    if (!data) {
        fprintf(stderr, "Failed to load image %s\n", stbi_failure_reason());
        this->status = Status::Failure;
        return;
    }
    printf("Texture requires %d bytes.\n", 4 * this->w * this->h);
}

// This is allowed to fail. The VRAM usage will just be unnecessarily higher.
void Texture::_shrink_to_fit() {
    if (this->h > 512) {
        int max_h = 512;
        int max_w = std::round(static_cast<float>(this->w) * (static_cast<float>(max_h) / static_cast<float>(this->h)));

        printf("New width: %d\n", max_w);
        printf("Resized image requires %d bytes.\n", max_w * max_h * 4);

        uint8_t* resized = new uint8_t[max_w * max_h * 4];

        int stride = this->w * 4;
        int stride_new = max_w * 4;

        if (!stbir_resize_uint8_linear(data, this->w, this->h, stride, resized, max_w, max_h,
                                  stride_new, static_cast<stbir_pixel_layout>(4))) {
            fprintf(stderr, "Image resize failed.\n");
            delete[] resized;
            return;
        }

        this->w = max_w;
        this->h = max_h;

        stbi_image_free(data);
        data = resized;
    }
}

vint Texture::finalize_creation(const char* out_path) {
    if (this->status == Status::Failure) {
        this->destroy();
        this->status = Status::Failure;
        return;
    }

    this->_shrink_to_fit();

    if (out_path) {
        stbi_write_png(out_path, this->w, this->h, 4, data, this->w * 4);
    }

    this->status = Status::ReadyToBind;
}

vint Texture::create_from_file(string filename, const char* out_path) {
    if (this->texture_id)
        this->destroy();
    // TODO: Look at async, future and promise
    thread generator([this, filename, out_path] {
        _load_from_file(filename);
        finalize_creation(out_path);
    });
    generator.detach();
}
vint Texture::create_from_memory(const uint8_t* data, uint64_t size, const char* out_path) {
    if (this->texture_id)
        this->destroy();
    thread generator([this, data, size, out_path] {
        _load_from_memory(data, size);
        finalize_creation(out_path);
    });
    //generator.detach();
    generator.join();
}

int Texture::bind_to_context() {
    glGenTextures(1, &this->texture_id);
    if (this->texture_id) {
        glBindTexture(GL_TEXTURE_2D, this->texture_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        } else {
            this->destroy();
            this->status = Status::Failure;
            return -1;
        }

        this->status = Status::Success;
    } else {
        this->status = Status::Failure;
    }

    this->garbage_collect();
    return 0;
}

void Texture::write_png(const char* path) {
    if (this->status == Status::ReadyToBind)
        this->bind_to_context();
    if (this->status == Status::Success) {
        uint8_t* texture_data = new uint8_t[w*h*4];
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
        stbi_write_png(path, this->w, this->h, 4, texture_data, 4*w);
        printf("%s\n", stbi_failure_reason());
        delete[] texture_data;
    }
}

void Texture::garbage_collect() {
    if (data) {
        stbi_image_free(data);
        data = nullptr;
    }
}

const GLuint* const Texture::get() const {
    return &this->texture_id;
}

int Texture::get_width() const {
    return this->w;
}

int Texture::get_height() const {
    return this->h;
}

bool Texture::is_init() const {
    return this->status == Status::Success;
}

bool Texture::ready_to_bind() const {
    return this->status == Status::ReadyToBind;
}

bool Texture::is_null() const {
    return !this->texture_id;
}

void Texture::destroy() {
    if (this->texture_id) {
        glDeleteTextures(1, &this->texture_id);
        this->texture_id = NULL;
    }
    this->garbage_collect();
    this->w = 0;
    this->h = 0;
    this->status = Status::Uninitialized;
}
