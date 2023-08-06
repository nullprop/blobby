#pragma once

#include <list>
#include <string>
#include <vector>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <bx/timer.h>

#include "blob.h"
#include "context.h"
#include "math.h"
#include "terrain.h"

namespace organic
{
    class Renderer
    {
      public:
        Renderer();
        ~Renderer();

        void Loop(Context* context);

        bool IsValid()
        {
            return m_valid;
        }

      private:
        bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;
        bgfx::VertexBufferHandle m_vbh = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle m_ibh = BGFX_INVALID_HANDLE;

        bgfx::UniformHandle m_u_globals = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle m_u_positions = BGFX_INVALID_HANDLE;

        bool m_valid;
    };
}
