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

namespace blobby
{
    static bgfx::ShaderHandle create_shader(const std::string& shader, const char* name)
    {
        const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
        const bgfx::ShaderHandle handle = bgfx::createShader(mem);
        bgfx::setName(handle, name);
        return handle;
    }

    static blobby::Vec3 screen_vertices[] = {
        {-1.0f, -1.0f, 0.0f}, // tl
        {1.0f, -1.0f, 0.0f},  // tr
        {1.0f, 1.0f, 0.0f},   // br
        {-1.0f, 1.0f, 0.0f},  // bl
    };
    static const uint16_t screen_tri_list[] = {0, 1, 2, 0, 2, 3};

    Renderer::Renderer()
    {
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

        // Update terrain culling
        context->terrain->Update(context, proj);

        // Uniforms
        std::vector<Blob*> blobs = context->terrain->GetBlobsToRender();
        int numSDF = blobs.size();
        if (numSDF > MAX_SDF)
        {
            printf("Exceeded max SDF limit, some will not be rendered! (%d/%d)\n", numSDF, MAX_SDF);
            numSDF = MAX_SDF;
        }

        std::vector<Vec4> positions = std::vector<Vec4>(numSDF);
        for (int i = 0; i < numSDF; i++)
        {
            positions[i] = {blobs[i]->Position.x, blobs[i]->Position.y, blobs[i]->Position.z, blobs[i]->Radius};
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
}