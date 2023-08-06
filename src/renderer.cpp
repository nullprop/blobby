#include <algorithm>
#include <bx/math.h>
#include <imgui.h>
#include <vector>

#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "sdl-imgui/imgui_impl_sdl2.h"

#include "blob.h"
#include "constants.h"
#include "file-ops.h"
#include "renderer.h"

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
        m_u_positions = bgfx::createUniform("u_positions", bgfx::UniformType::Vec4, MAX_SDF);

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

        // imgui
        ImGui_Implbgfx_NewFrame();
        ImGui_ImplSDL2_NewFrame();

        ImGui::NewFrame();
        ImGui::ShowDemoWindow(); // your drawing here
        ImGui::Render();
        ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());

        // Matrices
        float view[16];
        bx::mtxInverse(view, context->camTransform);

        float proj[16];
        bx::mtxProj(proj, 75.0f, float(context->width) / float(context->height), NEAR_PLANE, FAR_PLANE,
                    bgfx::getCaps()->homogeneousDepth);

        bgfx::setViewTransform(0, view, proj);

        float model[16];
        bx::mtxIdentity(model);
        bgfx::setTransform(model);

        // Uniforms
        int maxSDF = m_blobs.size();
        if (maxSDF > MAX_SDF)
        {
            printf("Exceeded max SDF limit, some will not be rendered! (%d/%d)", maxSDF, MAX_SDF);
            maxSDF = MAX_SDF;
        }

        std::vector<Vec4> positions = std::vector<Vec4>(maxSDF);
        int numSDF = 0;
        for (Blob& blob : m_blobs)
        {
            // Distance cull
            if (distance(blob.Position, context->camPosition) > FAR_PLANE + blob.Radius)
                continue;

            // TODO: frustrum cull

            positions[numSDF] = {blob.Position.x, blob.Position.y, blob.Position.z, blob.Radius};
            numSDF++;
            if (numSDF >= maxSDF)
                break;
        }

        float globals[4] = {context->time, (float)numSDF, 0, 0};
        bgfx::setUniform(m_u_globals, globals, 1);
        bgfx::setUniform(m_u_positions, &positions[0], numSDF);

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