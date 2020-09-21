#include "context.h"
#include "debug.h"
#include "camera.h"
#include "shader.h"
#include "mesh.h"
#include "material.h"
#include "texture.h"
#include "framebuffer.h"
#include "drawelement.h"
#include "query.h"
#include "file_util.h"
#include "stb_image_write.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>

// -------------------------------------------
// helper funcs

static void glfw_error_func(int error, const char *description) {
    fprintf(stderr, "GLFW: Error %i: %s\n", error, description);
}

static bool show_gui = false;
static void draw_gui(); // implementation below
static void parse_gl_header(); // implementation below

static void (*user_keyboard_callback)(int key, int scancode, int action, int mods) = 0;
static void (*user_mouse_callback)(double xpos, double ypos) = 0;
static void (*user_mouse_button_callback)(int button, int action, int mods) = 0;
static void (*user_mouse_scroll_callback)(double xoffset, double yoffset) = 0;
static void (*user_resize_callback)(int w, int h) = 0;

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, 1);
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
        show_gui = !show_gui;
    if (ImGui::GetIO().WantCaptureKeyboard) {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        return;
    }
    if (user_keyboard_callback)
        user_keyboard_callback(key, scancode, action, mods);
}

static void glfw_mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (ImGui::GetIO().WantCaptureMouse)
        return;
    if (user_mouse_callback)
        user_mouse_callback(xpos, ypos);
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        return;
    }
    if (user_mouse_button_callback)
        user_mouse_button_callback(button, action, mods);
}

static void glfw_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse) {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        return;
    }
    Camera::default_camera_movement_speed += Camera::default_camera_movement_speed * yoffset * 0.1;
    Camera::default_camera_movement_speed = std::max(0.00001f, Camera::default_camera_movement_speed);
    if (user_mouse_scroll_callback)
        user_mouse_scroll_callback(xoffset, yoffset);
}

static void glfw_resize_callback(GLFWwindow* window, int w, int h) {
    Context::resize(w, h);
    if (user_resize_callback)
        user_resize_callback(w, h);
}


static void glfw_char_callback(GLFWwindow* window, unsigned int c) {
    ImGui_ImplGlfw_CharCallback(window, c);
}

// -------------------------------------------
// Context

static ContextParameters parameters;
std::string Context::base_path;

Context::Context() {
    if (!glfwInit()) {
        std::cerr << "glfwInit failed!" << std::endl;
        exit(1);
    }
    glfwSetErrorCallback(glfw_error_func);

    // some GL context settings
    if (parameters.gl_major > 0)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, parameters.gl_major);
    if (parameters.gl_minor > 0)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, parameters.gl_minor);
    glfwWindowHint(GLFW_RESIZABLE, parameters.resizable);
    glfwWindowHint(GLFW_VISIBLE, parameters.visible);
    glfwWindowHint(GLFW_DECORATED, parameters.decorated);
    glfwWindowHint(GLFW_FLOATING, parameters.floating);
    glfwWindowHint(GLFW_MAXIMIZED, parameters.maximised);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, parameters.gl_debug_context);

    // create window and context
    glfw_window = glfwCreateWindow(parameters.width, parameters.height, parameters.title.c_str(), 0, 0);
    if (!glfw_window) {
        std::cerr << "ERROR: glfwCreateContext failed!" << std::endl;
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(glfw_window);
    glfwSwapInterval(parameters.swap_interval);

    glewExperimental = GL_TRUE;
    const GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "ERROR: GLEWInit failed: " << glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(glfw_window);
        glfwTerminate();
        exit(1);
    }

    // output configuration
    std::cerr << "GLFW: " << glfwGetVersionString() << std::endl;
    std::cerr << "OpenGL: " << glGetString(GL_VERSION) << ", " << glGetString(GL_RENDERER) << std::endl;
    std::cerr << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // enable debugging output
    enable_strack_trace_on_crash();
    enable_gl_debug_output();

    // setup user ptr
    glfwSetWindowUserPointer(glfw_window, this);

    // install callbacks
    glfwSetKeyCallback(glfw_window, glfw_key_callback);
    glfwSetCursorPosCallback(glfw_window, glfw_mouse_callback);
    glfwSetMouseButtonCallback(glfw_window, glfw_mouse_button_callback);
    glfwSetScrollCallback(glfw_window, glfw_mouse_scroll_callback);
    glfwSetFramebufferSizeCallback(glfw_window, glfw_resize_callback);
    glfwSetCharCallback(glfw_window, glfw_char_callback);

    // set input mode
    glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, 1);
    glfwSetInputMode(glfw_window, GLFW_STICKY_MOUSE_BUTTONS, 1);

    // init imgui
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(glfw_window, false);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // load custom font?
    if (not parameters.font_ttf_filename.empty()) {
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 3;
        std::cout << "Loading: " << parameters.font_ttf_filename << "..." << std::endl;
        ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->AddFontFromFileTTF(
                parameters.font_ttf_filename.c_str(), parameters.font_size_pixels, &config);
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // init GLenum <-> string mapping
    parse_gl_header();

    // set some sane GL defaults
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glClearColor(0, 0, 0, 1);
    glClearDepth(1);

    // setup timer
    last_t = curr_t = glfwGetTime();
    cpu_timer = std::make_shared<TimerQuery>("CPU-overall");
    frame_timer = std::make_shared<TimerQuery>("Frame-time");
    gpu_timer = std::make_shared<TimerQueryGL>("GPU-overall");
    cpu_timer->start();
    frame_timer->start();
    gpu_timer->start();
}

Context::~Context() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwDestroyWindow(glfw_window);
    glfwTerminate();
}

Context& Context::init(const ContextParameters& params) {
    parameters = params;
    return instance();
}

Context& Context::instance() {
    static Context ctx;
    return ctx;
}

bool Context::running() { return !glfwWindowShouldClose(instance().glfw_window); }

void Context::swap_buffers() {
    draw_gui();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    instance().cpu_timer->end();
    instance().gpu_timer->end();
    glfwSwapBuffers(instance().glfw_window);
    instance().frame_timer->end();
    instance().frame_timer->start();
    instance().cpu_timer->start();
    instance().gpu_timer->start();
    instance().last_t = instance().curr_t;
    instance().curr_t = glfwGetTime() * 1000; // s to ms
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

double Context::frame_time() { return instance().curr_t - instance().last_t; }

void Context::screenshot(const std::string& filepath) {
    stbi_flip_vertically_on_write(1);
    glm::ivec2 size = resolution();
    std::vector<uint8_t> pixels(size.x * size.y * 3);
    glReadPixels(0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    // check file extension
    const std::string ext = get_extension(filepath);
    if (ext == "png") {
        stbi_write_png(filepath.c_str(), size.x, size.y, 3, pixels.data(), 0);
        std::cout << filepath << " written." << std::endl;
    } else if (ext == "jpg") {
        stbi_write_jpg(filepath.c_str(), size.x, size.y, 3, pixels.data(), 0);
        std::cout << filepath << " written." << std::endl;
    } else {
        std::cerr << "WARN: Context::screenshot: unknown extension, changing to .png!" << std::endl;
        const std::string file = change_extension(filepath, ".png");
        std::cout << file << std::endl;
        stbi_write_png(file.c_str(), size.x, size.y, 3, pixels.data(), 0);
        std::cout << file << " written." << std::endl;
    }
}

void Context::show() { glfwShowWindow(instance().glfw_window); }

void Context::hide() { glfwHideWindow(instance().glfw_window); }

glm::ivec2 Context::resolution() {
    int w, h;
    glfwGetFramebufferSize(instance().glfw_window, &w, &h);
    return glm::ivec2(w, h);
}

void Context::resize(int w, int h) {
    glfwSetWindowSize(instance().glfw_window, w, h);
    glViewport(0, 0, w, h);
}

void Context::set_title(const std::string& name) { glfwSetWindowTitle(instance().glfw_window, name.c_str()); }

void Context::set_swap_interval(uint32_t interval) { glfwSwapInterval(interval); }

void Context::capture_mouse(bool on) { glfwSetInputMode(instance().glfw_window, GLFW_CURSOR, on ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL); }

glm::vec2 Context::mouse_pos() {
    double xpos, ypos;
    glfwGetCursorPos(instance().glfw_window, &xpos, &ypos);
    return glm::vec2(xpos, ypos);
}

bool Context::mouse_button_pressed(int button) { return glfwGetMouseButton(instance().glfw_window, button) == GLFW_PRESS; }

bool Context::key_pressed(int key) { return glfwGetKey(instance().glfw_window, key) == GLFW_PRESS; }

void Context::set_keyboard_callback(void (*fn)(int key, int scancode, int action, int mods)) { user_keyboard_callback = fn; }

void Context::set_mouse_callback(void (*fn)(double xpos, double ypos)) { user_mouse_callback = fn; }

void Context::set_mouse_button_callback(void (*fn)(int button, int action, int mods)) { user_mouse_button_callback = fn; }

void Context::set_mouse_scroll_callback(void (*fn)(double xoffset, double yoffset)) { user_mouse_scroll_callback = fn; }

void Context::set_resize_callback(void (*fn)(int w, int h)) { user_resize_callback = fn; }

// -------------------------------------------
// GUI

static void display_camera(Camera* cam) {
    ImGui::Indent();
    ImGui::DragFloat3("pos", &cam->pos.x, 0.001f);
    ImGui::DragFloat3("dir", &cam->dir.x, 0.001f);
    ImGui::DragFloat3("up", &cam->up.x, 0.001f);
    ImGui::Checkbox("fix_up_vector", &cam->fix_up_vector);
    ImGui::Checkbox("perspective", &cam->perspective);
    if (cam->perspective) {
        ImGui::DragFloat("fov", &cam->fov_degree, 0.01f);
        ImGui::DragFloat("near", &cam->near, 0.001f);
        ImGui::DragFloat("far", &cam->far, 0.001f);
    } else {
        ImGui::DragFloat("left", &cam->left, 0.001f);
        ImGui::DragFloat("right", &cam->right, 0.001f);
        ImGui::DragFloat("top", &cam->top, 0.001f);
        ImGui::DragFloat("bottom", &cam->bottom, 0.001f);
    }
    if (ImGui::Button("Make current")) cam->make_current();
    ImGui::Unindent();
}

static void display_texture(const Texture2D* tex, ImVec2 size = ImVec2(300, 300)) {
    ImGui::Indent();
    ImGui::Text("ID: %u, size: %ux%u", tex->id, tex->w, tex->h);
    ImGui::Text("internal_format: %s", Context::enum_to_string(tex->internal_format).c_str());
    ImGui::Text("format: %s", Context::enum_to_string(tex->format).c_str());
    ImGui::Text("type: %s", Context::enum_to_string(tex->type).c_str());
    ImGui::Image((void*)tex->id, size, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5));
    if (ImGui::Button("Save PNG")) tex->save_png(change_extension(tex->name, ".png"));
    ImGui::SameLine();
    if (ImGui::Button("Save JPEG")) tex->save_jpg(change_extension(tex->name, ".jpg"));
    ImGui::Unindent();
}

static void display_material(const Material* mat) {
    ImGui::Indent();
    for (auto& tex_entry : mat->texture_map) {
        if (ImGui::CollapsingHeader(tex_entry.first.c_str()))
            display_texture(tex_entry.second.get(), ImVec2(200, 200));
    }
    ImGui::Unindent();
}

static void display_shader(Shader* shader) {
    ImGui::Indent();
    ImGui::Text("ID: %u", shader->id);
    if (shader->source_files.count(GL_VERTEX_SHADER))
        ImGui::Text("vertex source: %s", shader->source_files[GL_VERTEX_SHADER].c_str());
    if (shader->source_files.count(GL_TESS_CONTROL_SHADER))
        ImGui::Text("tess_control source: %s", shader->source_files[GL_TESS_CONTROL_SHADER].c_str());
    if (shader->source_files.count(GL_TESS_EVALUATION_SHADER))
        ImGui::Text("tess_eval source: %s", shader->source_files[GL_TESS_EVALUATION_SHADER].c_str());
    if (shader->source_files.count(GL_GEOMETRY_SHADER))
        ImGui::Text("geometry source: %s", shader->source_files[GL_GEOMETRY_SHADER].c_str());
    if (shader->source_files.count(GL_FRAGMENT_SHADER))
        ImGui::Text("fragment source: %s", shader->source_files[GL_FRAGMENT_SHADER].c_str());
    if (shader->source_files.count(GL_COMPUTE_SHADER))
        ImGui::Text("compute source: %s", shader->source_files[GL_COMPUTE_SHADER].c_str());
    if (ImGui::Button("Compile"))
        shader->compile();
    ImGui::Unindent();
}

static void display_mesh(const Mesh* mesh) {
    ImGui::Indent();
    ImGui::Text("#vertices: %i", mesh->num_vertices);
    ImGui::Text("#indices: %i", mesh->num_indices);
    ImGui::Text("primitive_type: %i", mesh->primitive_type);
    for (uint32_t i = 0; i < mesh->vbo_ids.size(); ++i) {
        if (ImGui::CollapsingHeader((std::string("vbo_") + std::to_string(i) + "##" + mesh->name).c_str())) {
            ImGui::Text("ID: %u", mesh->vbo_ids[i]);
            ImGui::Text("type: %s", Context::enum_to_string(mesh->vbo_types[i]).c_str());
            ImGui::Text("elem dim: %u", mesh->vbo_dims[i]);
        }
    }
    ImGui::Unindent();
}

static void display_framebuffer(const Framebuffer* fbo) {
    ImGui::Indent();
    ImGui::Text("ID: %u", fbo->id);
    ImGui::Text("size: %ux%u", fbo->w, fbo->h);
    if (ImGui::CollapsingHeader(("depth attachment##" + fbo->name).c_str()) && fbo->depth_texture) {
        display_texture(fbo->depth_texture.get());
    }
    for (uint32_t i = 0; i < fbo->color_textures.size(); ++i) {
        if (ImGui::CollapsingHeader(std::string("color attachment " + std::to_string(i) + "##" + fbo->name).c_str()))
            display_texture(fbo->color_textures[i].get());
    }
    ImGui::Unindent();
}

/*
// TODO display matrices?
static void display_mat4(glm::mat4* mat) {
    ImGui::Indent();
    ImGui::Text("Matrix:");
    glm::mat4 row_maj = glm::transpose(*mat);
    bool modified = false;
    if (ImGui::DragFloat4("row0", &row_maj[0][0], 0.001f))
        modified = true;
    if (ImGui::DragFloat4("row1", &row_maj[1][0], 0.001f))
        modified = true;
    if (ImGui::DragFloat4("row2", &row_maj[2][0], 0.001f))
        modified = true;
    if (ImGui::DragFloat4("row3", &row_maj[3][0], 0.001f))
        modified = true;
    if (modified)
        *mat = glm::transpose(row_maj);
    ImGui::Unindent();
}
*/

static void display_drawelement(Drawelement* draw) {
    ImGui::Indent();
    if (draw->shader)
        if (ImGui::CollapsingHeader(std::string("Shader: " + draw->shader->name).c_str()))
            display_shader(draw->shader.get());
    if (draw->material)
        if (ImGui::CollapsingHeader(std::string("Material: " + draw->material->name).c_str()))
            display_material(draw->material.get());
    for (uint32_t i = 0; i < draw->meshes.size(); ++i)
        if (ImGui::CollapsingHeader(std::string("Mesh " + std::to_string(i) + ": " + draw->meshes[i]->name).c_str()))
            display_mesh(draw->meshes[i].get());
    ImGui::Unindent();
}

static void display_timer_buffer(RingBuffer<float>* buf) {
    ImGui::Indent();
    const float avg = buf->exp_avg;
    const float lower = buf->min();
    const float upper = buf->max();
    ImGui::Text("avg: %.2fms, min: %.2fms, max: %.2fms", avg, lower, upper);
    const std::string label = std::to_string(avg).substr(0, 4) + " [" + std::to_string(lower).substr(0, 4) + ", " + std::to_string(upper).substr(0, 4) + "] ms";
    // style
    const glm::vec3 ok = glm::vec3(0, 2, 0), bad = glm::vec3(2, 0, 0);
    const float f = (avg - lower) / (upper - lower);
    const glm::vec3 col = mix(ok, bad, f);
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(col.x, col.y, col.z, 1.f));
    // plot
    ImGui::PlotLines(label.c_str(), buf->data.data(), buf->data.size(), buf->curr, 0, lower, upper, ImVec2(0, 30));
    ImGui::PopStyleColor();
    ImGui::Unindent();
}

static void draw_gui() {
    if (!show_gui) return;

    static bool gui_show_cameras = false;
    static bool gui_show_textures = false;
    static bool gui_show_fbos = false;
    static bool gui_show_meshes = false;
    static bool gui_show_materials = false;
    static bool gui_show_shaders = false;
    static bool gui_show_drawelements = false;
    static bool gui_show_timer = false;

    if (ImGui::BeginMainMenuBar()) {
        ImGui::Checkbox("cameras", &gui_show_cameras);
        ImGui::Separator();
        ImGui::Checkbox("textures", &gui_show_textures);
        ImGui::Separator();
        ImGui::Checkbox("fbos", &gui_show_fbos);
        ImGui::Separator();
        ImGui::Checkbox("meshes", &gui_show_meshes);
        ImGui::Separator();
        ImGui::Checkbox("materials", &gui_show_materials);
        ImGui::Separator();
        ImGui::Checkbox("shaders", &gui_show_shaders);
        ImGui::Separator();
        ImGui::Checkbox("drawelements", &gui_show_drawelements);
        ImGui::Separator();
        ImGui::Checkbox("timer", &gui_show_timer);
        ImGui::Separator();
        if (ImGui::Button("Screenshot"))
            Context::screenshot("screenshot.png");
        ImGui::EndMainMenuBar();
    }

    if (gui_show_cameras) {
        if (ImGui::Begin(std::string("Cameras (" + std::to_string(Camera::map.size()) + ")").c_str(), &gui_show_cameras)) {
            ImGui::Text("Current: %s", Camera::current()->name.c_str());
            for (auto& entry : Camera::map) {
                if (ImGui::CollapsingHeader(entry.first.c_str()))
                    display_camera(entry.second);
            }
        }
        ImGui::End();
    }

    if (gui_show_textures) {
        if (ImGui::Begin(std::string("Textures (" + std::to_string(Texture2D::map.size()) + ")").c_str(), &gui_show_textures)) {
            for (auto& entry : Texture2D::map) {
                if (ImGui::CollapsingHeader(entry.first.c_str()))
                    display_texture(entry.second, ImVec2(300, 300));
            }
        }
        ImGui::End();
    }

    if (gui_show_meshes) {
        if (ImGui::Begin(std::string("Meshes (" + std::to_string(Mesh::map.size()) + ")").c_str(), &gui_show_meshes)) {
            for (auto& entry : Mesh::map)
                if (ImGui::CollapsingHeader(entry.first.c_str()))
                    display_mesh(entry.second);
        }
        ImGui::End();
    }

    if (gui_show_materials) {
        if (ImGui::Begin(std::string("Materials (" + std::to_string(Material::map.size()) + ")").c_str(), &gui_show_materials)) {
            for (auto& entry : Material::map)
                if (ImGui::CollapsingHeader(entry.first.c_str()))
                    display_material(entry.second);
        }
        ImGui::End();
    }

    if (gui_show_shaders) {
        if (ImGui::Begin(std::string("Shaders (" + std::to_string(Shader::map.size()) + ")").c_str(), &gui_show_shaders)) {
            for (auto& entry : Shader::map)
                if (ImGui::CollapsingHeader(entry.first.c_str()))
                    display_shader(entry.second);
            if (ImGui::Button("Reload modified")) Shader::reload();
        }
        ImGui::End();
    }

    if (gui_show_fbos) {
        if (ImGui::Begin(std::string("Framebuffers (" + std::to_string(Framebuffer::map.size()) + ")").c_str(), &gui_show_fbos)) {
            for (auto& entry : Framebuffer::map)
                if (ImGui::CollapsingHeader(entry.first.c_str()))
                    display_framebuffer(entry.second);
        }
        ImGui::End();
    }

    if (gui_show_drawelements) {
        if (ImGui::Begin(std::string("Drawelements (" + std::to_string(Drawelement::map.size()) + ")").c_str(), &gui_show_drawelements)) {
            for (auto& entry : Drawelement::map)
                if (ImGui::CollapsingHeader(entry.first.c_str()))
                    display_drawelement(entry.second);
        }
        ImGui::End();
    }

    if (gui_show_timer) {
        if (ImGui::Begin(std::string("Timer (" + std::to_string(TimerQuery::map.size() + TimerQueryGL::map.size()) + ")").c_str(), &gui_show_timer)) {
            for (auto& entry : TimerQuery::map) {
                ImGui::Separator();
                ImGui::Text("%s:", entry.first.c_str());
                display_timer_buffer(&entry.second->buf);
            }
            for (auto& entry : TimerQueryGL::map) {
                ImGui::Separator();
                ImGui::Text("%s:", entry.first.c_str());
                display_timer_buffer(&entry.second->buf);
            }
        }
        ImGui::End();
    }
}

// -------------------------------------------
// Enum <-> string conversion

static std::map<std::string, GLint> string_enum_map;
static std::map<GLint, std::string> enum_string_map;

static std::vector<std::string> tokenize(const std::string& str, const std::string& delim) {
    std::vector<std::string> out;
    size_t start, end = 0;
    while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
        end = str.find_first_of(delim, start);
        out.push_back(str.substr(start, end - start));
    }
    return out;
}

static void parse_gl_header() {
    std::stringstream input(read_file(concat(OPENGL_INCLUDE_PATH, "GL/glew.h")) + read_file(concat(OPENGL_INCLUDE_PATH, "GL/gl.h")));
    std::string line;
    while (getline(input, line)) {
        if (line.find("#define ") != std::string::npos) {
            line = line.substr(std::string("#define ").size());
            const auto& token = tokenize(line, " \t");
            if (token.size() == 2) {
                try {
                    const std::string s = token[0];
                    const GLint value = std::stoi(token[1], 0, 0);
                    string_enum_map[s] = value;
                    enum_string_map[value] = s;
                } catch (std::exception& e) {}
            }
        }
    }
}

std::string Context::enum_to_string(GLint value) {
    return enum_string_map.at(value);
}

GLint Context::string_to_enum(const std::string& value) {
    return string_enum_map.at(value);
}
