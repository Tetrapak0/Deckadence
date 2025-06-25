#pragma once

#include <thread>
using std::thread;

#include "../../external/GLFW/include/glfw3.h"

#define DX_THUMBNAIL_UNINITIALIZED 0xFFFF

// Function returns void but sets the internal status variable
typedef void vint;

struct Thumbnail {
private:
    GLuint                 thumbnail = 0;
    /*
     * status - result of the create() function ran on a separate thread
              * 0xFFFF = uninitialized;
              * 0      = success;
              * -1     = failure;
    */
    int                    status = DX_THUMBNAIL_UNINITIALIZED;
    int                    w = 0;
    int                    h = 0;
    vint                   _create(const char* const filename);
public:
    vint                   create(const char* const filename);
    const GLuint* const    get() const;
    int                    get_width() const;
    int                    get_height() const;
    uint32_t               get_format() const;
    bool                   is_init() const;
    bool                   is_null() const;
    void                   destroy();
    ~Thumbnail() {
        this->destroy();
    };
};
