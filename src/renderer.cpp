#include "renderer.h"
#include "blob.h"
#include "file-ops.h"

namespace organic
{
    static bgfx::ShaderHandle create_shader(const std::string& shader, const char* name)
    {
        const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
        const bgfx::ShaderHandle handle = bgfx::createShader(mem);
        bgfx::setName(handle, name);
        return handle;
    }

    static organic::Vec3 screen_vertices[] = {
        {-1.0f, -1.0f, 0.0f}, // tl
        {1.0f, -1.0f, 0.0f},  // tr
        {1.0f, 1.0f, 0.0f},   // br
        {-1.0f, 1.0f, 0.0f},  // bl
    };
    static const uint16_t screen_tri_list[] = {0, 1, 2, 0, 2, 3};

    Renderer::Renderer()
    {
        m_blobs = std::list<Blob>();
        for (int i = 0; i < 16; i++)
        {
            m_blobs.emplace_back((float)i, 0, 0, 0.5);
        }

        bgfx::VertexLayout pos_col_vert_layout;
        pos_col_vert_layout.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();
        bgfx::VertexBufferHandle vbh =
            bgfx::createVertexBuffer(bgfx::makeRef(screen_vertices, sizeof(screen_vertices)), pos_col_vert_layout);
        bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(bgfx::makeRef(screen_tri_list, sizeof(screen_tri_list)));

        const std::string shader_root = "shaders/build/";

        std::string vshader;
        if (!fileops::read_file(shader_root + "v_simple.bin", vshader))
        {
            printf("Could not find shader vertex shader (ensure shaders have been "
                   "compiled).\n"
                   "Run compile-shaders-<platform>.sh/bat\n");
            return;
        }

        std::string fshader;
        if (!fileops::read_file(shader_root + "f_simple.bin", fshader))
        {
            printf("Could not find shader fragment shader (ensure shaders have "
                   "been compiled).\n"
                   "Run compile-shaders-<platform>.sh/bat\n");
            return;
        }

        bgfx::ShaderHandle vsh = create_shader(vshader, "vshader");
        bgfx::ShaderHandle fsh = create_shader(fshader, "fshader");
        bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, true);

        m_program = program;
        m_vbh = vbh;
        m_ibh = ibh;

        m_u_globals = bgfx::createUniform("u_globals", bgfx::UniformType::Vec4, 1);
        m_u_positions = bgfx::createUniform("u_positions", bgfx::UniformType::Vec4, 16);

        m_valid = true;
    }

    Renderer::~Renderer()
    {
        bgfx::destroy(m_u_positions);
        bgfx::destroy(m_u_globals);

        bgfx::destroy(m_vbh);
        bgfx::destroy(m_ibh);
        bgfx::destroy(m_program);
    }

    void Renderer::Loop(Context* context)
    {
        // TEST
        int j = 0;
        for (Blob& blob : m_blobs)
        {
            blob.Position.x = (float)j - 8.0f;
            blob.Position.y = sin(j * context->time * 0.1f);
            blob.Position.z = 0;
            blob.Radius = 0.2f + abs(sin(j * context->time * 0.05f)) * 0.5f;
            j++;
        }

        // Camera
        float cam_rotation[16];
        bx::mtxRotateXYZ(cam_rotation, context->cam_pitch, context->cam_yaw, 0.0f);

        float cam_translation[16];
        bx::mtxTranslate(cam_translation, 0.0f, 0.0f, -8.0f);

        float cam_transform[16];
        bx::mtxMul(cam_transform, cam_translation, cam_rotation);

        // Matrices
        float view[16];
        bx::mtxInverse(view, cam_transform);

        float proj[16];
        bx::mtxProj(proj, 60.0f, float(context->width) / float(context->height), 0.1f, 100.0f,
                    bgfx::getCaps()->homogeneousDepth);

        bgfx::setViewTransform(0, view, proj);

        float model[16];
        bx::mtxIdentity(model);
        bgfx::setTransform(model);

        // Uniforms
        float globals[4] = {context->time, 0, 0, 0};
        bgfx::setUniform(m_u_globals, globals, 1);

        organic::Vec4 positions[16] = {};
        int i = 0;
        for (Blob& blob : m_blobs)
        {
            positions[i] = {blob.Position.x, blob.Position.y, blob.Position.z, blob.Radius};
            i++;
            if (i >= 16)
                break;
        }
        bgfx::setUniform(m_u_positions, positions, 16);

        // Buffers
        bgfx::setVertexBuffer(0, m_vbh);
        bgfx::setIndexBuffer(m_ibh);

        // Draw
        bgfx::submit(0, m_program);
        bgfx::frame();
    }

    void Renderer::AddBlobs(std::vector<Blob> blobs)
    {
        m_blobs.insert(m_blobs.end(), blobs.begin(), blobs.end());
    }

    void Renderer::RemoveBlob(Blob blob)
    {
        m_blobs.remove(blob);
    }
}