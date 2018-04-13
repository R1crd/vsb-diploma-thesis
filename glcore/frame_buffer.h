#ifndef VSB_SEMESTRAL_PROJECT_FRAME_BUFFER_H
#define VSB_SEMESTRAL_PROJECT_FRAME_BUFFER_H

#include <GL/glew.h>

namespace tless {
    class FrameBuffer {
    private:
        GLuint RBO, Texture;

    public:
        GLuint id;
        int width, height;

        explicit FrameBuffer(int width, int height);
        ~FrameBuffer();

        void bind() const;

        void unbind() const;
    };
}

#endif