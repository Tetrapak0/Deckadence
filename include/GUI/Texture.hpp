#pragma once

#include <thread>
#include <string>
#include <filesystem>

#include "../../external/GLFW/include/glfw3.h"

using std::string;
using std::thread;
namespace fs = std::filesystem;

// Function returns void but sets the internal status variable
typedef void vint;

struct Texture {
private:
    GLuint                 texture_id = 0;
    enum class Status {
        Failure = -1,
        Success = 0,
        ReadyToBind = 1,
        Uninitialized = 0xFFFF
    };
    Status                  status = Status::Uninitialized;
    int                     w  = 0;
    int                     h  = 0;
    uint8_t*                data    = nullptr;
    void                    _shrink_to_fit();
    vint                    _load_from_file(string filename);
    vint                    _load_from_memory(const uint8_t* raw, uint64_t size);
    vint                    finalize_creation(const char* out_path);
public:
    vint                    create_from_file(string filename, const char* out_path = nullptr);
    vint                    create_from_memory(const uint8_t* data, uint64_t size, const char* out_path = nullptr);
    int                     bind_to_context();
    void                    write_png(const char* path);
    void                    garbage_collect();
    const GLuint* const     get() const;
    int                     get_width() const;
    int                     get_height() const;
    bool                    is_init() const;
    bool                    ready_to_bind() const;
    bool                    is_null() const;
    void                    destroy();

    Texture() = default;
    Texture(fs::path path, fs::path out_path = "");
    Texture(const uint8_t* data, int size);

    bool operator==(const Texture& other) const {
        return this->texture_id == other.texture_id;
    }
    bool operator!=(const Texture& other) const {
        return this->texture_id != other.texture_id;
    }
    ~Texture() {
        this->destroy();
    }
};
