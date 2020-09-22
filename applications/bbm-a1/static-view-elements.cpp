#include "static-view-elements.h"
#include <cppgl.h>
#include "rendering.h"
#include "dynamic-view-elements.h"

// ------------------------------------------------
// globals

extern int player_id;
extern std::vector<std::shared_ptr<Player>> players;

// ------------------------------------------------
// prototypes

Drawelement Floor::prototype;
std::vector<Drawelement> Skybox::prototype;

void init_static_prototypes() {
    { // init floor prototype
        // setup mesh
        glm::vec3 vertices[4] = { {0,0,0}, {0,0,1}, {1,0,1}, {1,0,0} };
        glm::vec3 normals[4] = { {0,1,0}, {0,1,0}, {0,1,0}, {0,1,0} };
        glm::vec2 texcoords[4] = { {0, 0}, {0, 1}, {1, 1}, {1, 0} };
        unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };
        auto mesh = Mesh("floor-mesh");
        mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices);
        mesh->add_vertex_buffer(GL_FLOAT, 3, 4, normals);
        mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texcoords);
        mesh->add_index_buffer(6, indices);
        // setup material
        auto mat = Material("floor-material");
        mat->add_texture("diffuse", Texture2D("floor-diffuse", "render-data/images/floor.png"));
        mat->add_texture("normalmap", Texture2D("floor-normalmap", "render-data/images/floor_normals.png"));
        mesh->material = mat;
        // setup shader
        auto shader = Shader("floor-shader", "shader/floor.vs", "shader/floor.fs");
        Floor::prototype = Drawelement("floor",shader,mesh);
    }
    { // init skybox prototype (don't try this at home, use a cubemap instead!)
        const float e = 0.001f;
        auto shader = Shader("skybox-shader", "shader/skybox.vs", "shader/skybox.fs");
        {
            // front
            glm::vec3 vertices_f[] = { {+1+e,-1-e,-1+e}, {-1-e,-1-e,-1+e}, {+1+e,+1+e,-1+e}, {-1-e,+1+e,-1+e} };
            glm::vec2 texs_f[] = { {0, 0}, {1, 0}, {0, 1}, {1, 1} };
            unsigned int idxs_f[] = { 0, 1, 2, 2, 1, 3 };
            auto mesh = Mesh("skybox-mesh-front");
            mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices_f);
            mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texs_f);
            mesh->add_index_buffer(6, idxs_f);
            auto mat = Material("skybox-mat-front");
            mat->add_texture("diffuse", Texture2D("skybox-tex-front", "render-data/images/skybox_front.png"));
            mesh->material = mat;
            auto drawelement = Drawelement("skybox-front", shader, mesh);
            Skybox::prototype.push_back(drawelement);
        }
        {
            // left
            glm::vec3 vertices_l[] = { {+1-e,-1-e,+1+e}, {+1-e,-1-e,-1-e}, {+1-e,+1+e,+1+e}, {+1-e,+1+e,-1-e} };
            glm::vec2 texs_l[] = { {0, 0}, {1, 0}, {0, 1}, {1, 1} };
            unsigned int idxs_l[] = { 0, 1, 2, 2, 1, 3 };
            auto mesh = Mesh("skybox-mesh-left");
            mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices_l);
            mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texs_l);
            mesh->add_index_buffer(6, idxs_l);
            auto mat = Material("skybox-mat-left");
            mat->add_texture("diffuse", Texture2D("skybox-tex-left", "render-data/images/skybox_left.png"));
            mesh->material = mat;

            auto drawelement = Drawelement("skybox-left", shader, mesh);
            Skybox::prototype.push_back(drawelement);
        }
        {
            // back
            glm::vec3 vertices_b[] = { {-1-e,-1-e,+1-e}, {+1+e,-1-e,+1-e}, {-1-e,+1+e,+1-e}, {+1+e,+1+e,+1-e} };
            glm::vec2 texs_b[] = { {0, 0}, {1, 0}, {0, 1}, {1, 1} };
            unsigned int idxs_b[] = { 0, 1, 2, 2, 1, 3 };
            auto mesh = Mesh("skybox-mesh-back");
            mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices_b);
            mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texs_b);
            mesh->add_index_buffer(6, idxs_b);
            auto mat = Material("skybox-mat-back");
            mat->add_texture("diffuse", Texture2D("skybox-tex-back", "render-data/images/skybox_back.png"));
            mesh->material = mat;
            auto drawelement = Drawelement("skybox-back", shader, mesh);
            Skybox::prototype.push_back(drawelement);
        }
        {
            // right
            glm::vec3 vertices_r[] = { {-1+e,-1-e,-1-e}, {-1+e,-1-e,+1+e}, {-1+e,+1+e,-1-e}, {-1+e,+1+e,+1+e} };
            glm::vec2 texs_r[] = { {0, 0}, {1, 0}, {0, 1}, {1, 1} };
            unsigned int idxs_r[] = { 0, 1, 2, 2, 1, 3 };
            auto mesh = Mesh("skybox-mesh-right");
            mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices_r);
            mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texs_r);
            mesh->add_index_buffer(6, idxs_r);
            auto mat = Material("skybox-mat-right");
            mat->add_texture("diffuse", Texture2D("skybox-tex-right", "render-data/images/skybox_right.png"));
            mesh->material = mat;
            auto drawelement = Drawelement("skybox-right", shader,  mesh);
            Skybox::prototype.push_back(drawelement);
        }
        {
            // top
            glm::vec3 vertices_t[] = { {-1-e,+1-e,-1-e}, {-1-e,+1-e,+1+e}, {+1+e,+1-e,-1-e}, {+1+e,+1-e,+1+e} };
            glm::vec2 texs_t[] = { {0, 0}, {1, 0}, {0, 1}, {1, 1} };
            unsigned int idxs_t[] = { 0, 1, 2, 2, 1, 3 };
            auto mesh = Mesh("skybox-mesh-top");
            mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices_t);
            mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texs_t);
            mesh->add_index_buffer(6, idxs_t);
            auto mat = Material("skybox-mat-top");
            mat->add_texture("diffuse", Texture2D("skybox-tex-top", "render-data/images/skybox_top.png"));
            mesh->material = mat;
            auto drawelement = Drawelement("skybox-top", shader, mesh);
            Skybox::prototype.push_back(drawelement);
        }
        {
            // bottom / down
            glm::vec3 vertices_d[] = { {-1-e,-1+e,-1-e}, {-1-e,-1+e,+1+e}, {+1+e,-1+e,-1-e}, {+1+e,-1+e,+1+e} };
            glm::vec2 texs_d[] = { {1, 1}, {1, 0}, {0, 1}, {0, 0} };
            unsigned int idxs_d[] = { 0, 2, 1, 2, 3, 1 };
            auto mesh = Mesh("skybox-mesh-bottom");
            mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices_d);
            mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texs_d);
            mesh->add_index_buffer(6, idxs_d);
            auto mat = Material("skybox-mat-bottom");
            mat->add_texture("diffuse", Texture2D("skybox-tex-bottom", "render-data/images/skybox_bottom.png"));
            mesh->material = mat;
            auto drawelement = Drawelement("skybox-bottom", shader, mesh);
            Skybox::prototype.push_back(drawelement);
        }
    }
}

void draw_gui() {
    const glm::ivec2 screen_size = Context::resolution();
    ImVec2 size = ImVec2(glm::clamp(screen_size.x / 4, 200, 400), glm::clamp(screen_size.y / 30, 20, 40));
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(size.x, size.y * (players.size()+1)));
    if (ImGui::Begin("GUI", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground)) {
        ImGui::SetWindowFontScale(1.5f);
        for (uint32_t i = 0; i < players.size(); ++i) {
            auto& player = players[i];
            const glm::vec3 col = glm::mix(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), player->health / 100.f);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(col.x, col.y, col.z, 1));
            const std::string label = player->name + " (frags: " + std::to_string(player->frags) + ") ";
            ImGui::ProgressBar(player->health / 100.f, size, label.c_str());
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        }
    }
    ImGui::End();
}

// ------------------------------------------------
// Floor

Floor::Floor(int w, int h) : trafo(1) {
    // setup modelmatrix
    trafo[0][0] = float(w) * render_settings::tile_size;
    trafo[1][1] = 0.5 * render_settings::tile_size;
    trafo[2][2] = float(h) * render_settings::tile_size;
    trafo[3][0] = -0.5 * render_settings::tile_size;
    trafo[3][2] = -0.5 * render_settings::tile_size;
}

void Floor::draw() {
    prototype->model = trafo;
    prototype->bind();
    setup_light(prototype->shader);
    prototype->shader->uniform("tc_scale", glm::vec2(trafo[0][0], trafo[2][2]) / render_settings::tile_size);
    prototype->unbind();
}

// ------------------------------------------------
// Skybox

Skybox::Skybox(int w, int h, int d) : trafo(1) {
    // build modelmatrix
	trafo[0][0] = d * render_settings::tile_size;
	trafo[1][1] = d * render_settings::tile_size;
	trafo[2][2] = d * render_settings::tile_size;
	trafo[3][0] = 0.5f * w * render_settings::tile_size;
    trafo[3][1] = -0.5 * d * render_settings::tile_size;
	trafo[3][2] = 0.5f * h * render_settings::tile_size;
}

void Skybox::draw() {
    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CW);
    for (auto& elem : prototype) {
        elem->bind();
        setup_light(elem->shader);
        elem->model = trafo;
        elem->draw();
        elem->unbind();
    }
    glFrontFace(GL_CCW);
    glDepthFunc(GL_LESS);
}
