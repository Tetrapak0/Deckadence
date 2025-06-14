#pragma once

#include <thread>
using std::thread;

#include "../common.hpp"
#include <windows.h>
#include "../../external/GLFW/include/glfw3.h"

#define NXSH_THUMBNAIL_UNINITIALIZED 0xFFFF

namespace NxSh::GUI {
    struct Thumbnail {
    private:
        GLuint                 thumbnail;
        /*
         * status - result of the create() function ran on a separate thread
                  * 0xFFFF = uninitialized;
                  * 0      = success;
                  * -1     = failure;
        */
        int                    status = NXSH_THUMBNAIL_UNINITIALIZED;
        int                    w;
        int                    h;
        vint                   _create(const char* const filename);
    public:
        vint                   create(const char* const filename);
        const GLuint* const    get() const;
        int                    get_width() const;
        int                    get_height() const;
        uint32_t               get_format() const;
        bool                   is_uninit() const;
        bool                   is_null() const;
        void                   destroy();
        ~Thumbnail() {
            this->destroy();
        };
    };
}