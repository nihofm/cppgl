#include "context.h"
#include "debug.h"
#include "camera.h"
#include "shader.h"
#include "texture.h"
#include "framebuffer.h"
#include "material.h"
#include "geometry.h"
#include "drawelement.h"
#include "anim.h"
#include "query.h"
#include "stb_image_write.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <iostream>

// -------------------------------------------
// helper funcs

static void glfw_error_func(int error, const char *description) {
    fprintf(stderr, "GLFW: Error %i: %s\n", error, description);
}

static bool show_gui = false;
static void draw_gui(); // implementation below

static void (*user_keyboard_callback)(int key, int scancode, int action, int mods) = 0;
static void (*user_mouse_callback)(double xpos, double ypos) = 0;
static void (*user_mouse_button_callback)(int button, int action, int mods) = 0;
static void (*user_mouse_scroll_callback)(double xoffset, double yoffset) = 0;
static void (*user_resize_callback)(int w, int h) = 0;
static void (*user_gui_callback)(void) = 0;
static std::vector<void (*)()> hook_gui_callbacks;

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
        show_gui = !show_gui;
    if (ImGui::GetIO().WantCaptureKeyboard) {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        return;
    }
    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, 1);
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
    CameraImpl::default_camera_movement_speed += CameraImpl::default_camera_movement_speed * float(yoffset * 0.1);
    CameraImpl::default_camera_movement_speed = std::max(0.00001f, CameraImpl::default_camera_movement_speed);
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

Context::Context() {
    if (!glfwInit())
        throw std::runtime_error("glfwInit failed!");
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
        glfwTerminate();
        throw std::runtime_error("glfwCreateContext failed!");
    }
    glfwMakeContextCurrent(glfw_window);
    glfwSwapInterval(parameters.swap_interval);

    glewExperimental = GL_TRUE;
    const GLenum err = glewInit();
    if (err != GLEW_OK) {
        glfwDestroyWindow(glfw_window);
        glfwTerminate();
        throw std::runtime_error(std::string("GLEWInit failed: ") + (const char*)glewGetErrorString(err));
    }

    // output configuration
    std::cout << "GLFW: " << glfwGetVersionString() << std::endl;
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << ", " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

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
    // ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // load custom font?
    if (fs::exists(parameters.font_ttf_filename)) {
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 3;
        std::cout << "Loading: " << parameters.font_ttf_filename << "..." << std::endl;
        ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->AddFontFromFileTTF(
                parameters.font_ttf_filename.string().c_str(), float(parameters.font_size_pixels), &config);
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // set some sane GL defaults
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glClearColor(0, 0, 0, 1);
    glClearDepth(1);

    // setup timer
    last_t = curr_t = glfwGetTime();
    cpu_timer = TimerQuery("CPU-time");
    frame_timer = TimerQuery("Frame-time");
    gpu_timer = TimerQueryGL("GPU-time");
    prim_count = PrimitiveQueryGL("#Primitives");
    frag_count = FragmentQueryGL("#Fragments");
    cpu_timer->begin();
    frame_timer->begin();
    gpu_timer->begin();
    prim_count->begin();
    frag_count->begin();
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
    instance().prim_count->end();
    instance().frag_count->end();
    glfwSwapBuffers(instance().glfw_window);
    instance().frame_timer->end();
    instance().frame_timer->begin();
    instance().cpu_timer->begin();
    instance().gpu_timer->begin();
    instance().prim_count->begin();
    instance().frag_count->begin();
    instance().last_t = instance().curr_t;
    instance().curr_t = glfwGetTime() * 1000; // s to ms
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

double Context::frame_time() { return instance().curr_t - instance().last_t; }

void Context::screenshot(const fs::path& path) {
    stbi_flip_vertically_on_write(1);
    const glm::ivec2 size = resolution();
    std::vector<uint8_t> pixels(size.x * size.y * 3);
    glReadPixels(0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    // check file extension
    if (path.extension() == ".png") {
        stbi_write_png(path.string().c_str(), size.x, size.y, 3, pixels.data(), 0);
        std::cout << path << " written." << std::endl;
    } else if (path.extension() == ".jpg") {
        stbi_write_jpg(path.string().c_str(), size.x, size.y, 3, pixels.data(), 0);
        std::cout << path << " written." << std::endl;
    } else {
        std::cerr << "WARN: Context::screenshot: unknown extension, changing to .png!" << std::endl;
        stbi_write_png(fs::path(path).replace_extension(".png").string().c_str(), size.x, size.y, 3, pixels.data(), 0);
        std::cout << fs::path(path).replace_extension(".png") << " written." << std::endl;
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

void Context::set_gui_callback(void (*fn)(void)) { user_gui_callback = fn; }

void Context::add_hook_callback(void (*fn)(void)) { hook_gui_callbacks.push_back(fn); }

// -------------------------------------------
// GUI

static void display_camera(Camera& cam) {
    ImGui::Indent();
    ImGui::Text("name: %s", cam->name.c_str());
    ImGui::DragFloat3(("pos##" + cam->name).c_str(), &cam->pos.x, 0.001f);
    ImGui::DragFloat3(("dir##" + cam->name).c_str(), &cam->dir.x, 0.001f);
    ImGui::DragFloat3(("up##" + cam->name).c_str(), &cam->up.x, 0.001f);
    ImGui::Checkbox(("fix_up_vector##" + cam->name).c_str(), &cam->fix_up_vector);
    ImGui::Checkbox(("perspective##" + cam->name).c_str(), &cam->perspective);
    if (cam->perspective) {
        ImGui::DragFloat(("fov##" + cam->name).c_str(), &cam->fov_degree, 0.01f);
        ImGui::DragFloat(("near##" + cam->name).c_str(), &cam->near, 0.001f);
        ImGui::DragFloat(("far##" + cam->name).c_str(), &cam->far, 0.001f);
    } else {
        ImGui::DragFloat(("left##" + cam->name).c_str(), &cam->left, 0.001f);
        ImGui::DragFloat(("right##" + cam->name).c_str(), &cam->right, 0.001f);
        ImGui::DragFloat(("top##" + cam->name).c_str(), &cam->top, 0.001f);
        ImGui::DragFloat(("bottom##" + cam->name).c_str(), &cam->bottom, 0.001f);
    }
    if (ImGui::Button(("Make current##" + cam->name).c_str())) make_camera_current(cam);
    ImGui::Unindent();
}

static void display_texture(const Texture2D& tex, ImVec2 size = ImVec2(300, 300)) {
    ImGui::Indent();
    ImGui::Text("name: %s", tex->name.c_str());
    ImGui::Text("ID: %u, size: %ux%u", tex->id, tex->w, tex->h);
    ImGui::Text("internal_format: %u, format %u, type: %u", tex->internal_format, tex->format, tex->type);
    long long int texID = tex->id; //to satisfy complaints casting possible 32bit int to 64 bit pointer
    ImGui::Image((ImTextureID)texID, size, ImVec2(0.f, 1.f), ImVec2(1.f, 0.f), ImVec4(1.f, 1.f, 1.f, 1.f), ImVec4(1.f, 1.f, 1.f, 0.5f));
    if (ImGui::Button(("Save PNG##" + tex->name).c_str())) tex->save_png(fs::path(tex->name).replace_extension(".png"));
    ImGui::SameLine();
    if (ImGui::Button(("Save JPEG##" + tex->name).c_str())) tex->save_jpg(fs::path(tex->name).replace_extension(".jpg"));
    ImGui::Unindent();
}

static void display_shader(Shader& shader) {
    ImGui::Indent();
    ImGui::Text("name: %s", shader->name.c_str());
    ImGui::Text("ID: %u", shader->id);
    if (shader->source_files.count(GL_VERTEX_SHADER))
        ImGui::Text("vertex source: %s", shader->source_files[GL_VERTEX_SHADER].string().c_str());
    if (shader->source_files.count(GL_TESS_CONTROL_SHADER))
        ImGui::Text("tess_control source: %s", shader->source_files[GL_TESS_CONTROL_SHADER].string().c_str());
    if (shader->source_files.count(GL_TESS_EVALUATION_SHADER))
        ImGui::Text("tess_eval source: %s", shader->source_files[GL_TESS_EVALUATION_SHADER].string().c_str());
    if (shader->source_files.count(GL_GEOMETRY_SHADER))
        ImGui::Text("geometry source: %s", shader->source_files[GL_GEOMETRY_SHADER].string().c_str());
    if (shader->source_files.count(GL_FRAGMENT_SHADER))
        ImGui::Text("fragment source: %s", shader->source_files[GL_FRAGMENT_SHADER].string().c_str());
    if (shader->source_files.count(GL_COMPUTE_SHADER))
        ImGui::Text("compute source: %s", shader->source_files[GL_COMPUTE_SHADER].string().c_str());
    if (ImGui::Button("Compile"))
        shader->compile();
    ImGui::Unindent();
}

static void display_framebuffer(const Framebuffer& fbo) {
    ImGui::Indent();
    ImGui::Text("name: %s", fbo->name.c_str());
    ImGui::Text("ID: %u", fbo->id);
    ImGui::Text("size: %ux%u", fbo->w, fbo->h);
    if (ImGui::CollapsingHeader(("depth attachment##" + fbo->name).c_str()) && fbo->depth_texture)
        display_texture(fbo->depth_texture);
    for (uint32_t i = 0; i < fbo->color_textures.size(); ++i)
        if (ImGui::CollapsingHeader(std::string("color attachment " + std::to_string(i) + "##" + fbo->name).c_str()))
            display_texture(fbo->color_textures[i]);
    ImGui::Unindent();
}

static void display_material(const Material& mat) {
    ImGui::Indent();
    ImGui::Text("name: %s", mat->name.c_str());

    ImGui::Text("int params: %lu", mat->int_map.size());
    ImGui::Indent();
    for (const auto& entry : mat->int_map)
        ImGui::Text("%s: %i", entry.first.c_str(), entry.second);
    ImGui::Unindent();

    ImGui::Text("float params: %lu", mat->float_map.size());
    ImGui::Indent();
    for (const auto& entry : mat->float_map)
        ImGui::Text("%s: %f", entry.first.c_str(), entry.second);
    ImGui::Unindent();

    ImGui::Text("vec2 params: %lu", mat->vec2_map.size());
    ImGui::Indent();
    for (const auto& entry : mat->vec2_map)
        ImGui::Text("%s: (%f, %f)", entry.first.c_str(), entry.second.x, entry.second.y);
    ImGui::Unindent();

    ImGui::Text("vec3 params: %lu", mat->vec3_map.size());
    ImGui::Indent();
    for (const auto& entry : mat->vec3_map)
        ImGui::Text("%s: (%f, %f, %f)", entry.first.c_str(), entry.second.x, entry.second.y, entry.second.z);
    ImGui::Unindent();

    ImGui::Text("vec4 params: %lu", mat->vec4_map.size());
    ImGui::Indent();
    for (const auto& entry : mat->vec4_map)
        ImGui::Text("%s: (%f, %f, %f, %.f)", entry.first.c_str(), entry.second.x, entry.second.y, entry.second.z, entry.second.w);
    ImGui::Unindent();

    ImGui::Text("textures: %lu", mat->texture_map.size());
    ImGui::Indent();
    for (const auto& entry : mat->texture_map) {
        ImGui::Text("%s:", entry.first.c_str());
        display_texture(entry.second);
    }
    ImGui::Unindent();

    ImGui::Unindent();
}

static void display_geometry(const Geometry& geom) {
    ImGui::Indent();
    ImGui::Text("name: %s", geom->name.c_str());
    ImGui::Text("AABB min: (%.3f, %.3f, %.3f)", geom->bb_min.x, geom->bb_min.y, geom->bb_min.z);
    ImGui::Text("AABB max: (%.3f, %.3f, %.3f)", geom->bb_max.x, geom->bb_max.y, geom->bb_max.z);
    ImGui::Text("#Vertices: %lu", geom->positions.size());
    ImGui::Text("#Indices: %lu", geom->indices.size());
    ImGui::Text("#Normals: %lu", geom->normals.size());
    ImGui::Text("#Texcoords: %lu", geom->texcoords.size());
    ImGui::Unindent();
}

static void display_mesh(const Mesh& mesh) {
    ImGui::Indent();
    ImGui::Text("name: %s", mesh->name.c_str());
    if (ImGui::CollapsingHeader(("geometry: " + mesh->geometry->name).c_str()))
        display_geometry(mesh->geometry);
    if (ImGui::CollapsingHeader(("material: " + mesh->material->name).c_str()))
        display_material(mesh->material);
    ImGui::Unindent();
}

static void display_mat4(glm::mat4& mat) {
    ImGui::Indent();
    glm::mat4 row_maj = glm::transpose(mat);
    bool modified = false;
    if (ImGui::DragFloat4("row0", &row_maj[0][0], .01f)) modified = true;
    if (ImGui::DragFloat4("row1", &row_maj[1][0], .01f)) modified = true;
    if (ImGui::DragFloat4("row2", &row_maj[2][0], .01f)) modified = true;
    if (ImGui::DragFloat4("row3", &row_maj[3][0], .01f)) modified = true;
    if (modified) mat = glm::transpose(row_maj);
    ImGui::Unindent();
}

static void display_drawelement(Drawelement& elem) {
    ImGui::Indent();
    ImGui::Text("name: %s", elem->name.c_str());
    if (ImGui::CollapsingHeader("modelmatrix"))
        display_mat4(elem->model);
    if (ImGui::CollapsingHeader(("shader: " + elem->shader->name).c_str()))
        display_shader(elem->shader);
    if (ImGui::CollapsingHeader(("mesh: " + elem->mesh->name).c_str()))
        display_mesh(elem->mesh);
    ImGui::Unindent();
}

static void display_animation(const Animation& anim) {
    ImGui::Indent();
    ImGui::Text("name: %s", anim->name.c_str());
    ImGui::Text("curr time: %.3f / %lu, ms_per_node: %.3f", anim->time, anim->camera_path.size(), anim->ms_between_nodes);
    ImGui::Text("camera path:");
    ImGui::Indent();
    for (const auto& [pos, rot] : anim->camera_path)
        ImGui::Text("pos: (%.3f, %.3f, %.3f), rot: (%.3f, %.3f, %.3f, %.3f)", pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, rot.w);
    ImGui::Unindent();
    // TODO add anim node data?
    ImGui::Unindent();
}

static void display_query_timer(const Query& query, const char* label="") {
    const float avg = query.exp_avg;
    const float lower = query.min();
    const float upper = query.max();
    ImGui::Text("avg: %.1fms, min: %.1fms, max: %.1fms", avg, lower, upper);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(.7f, .7f, 0.f, 1.f));
    ImGui::PlotHistogram(label, query.data.data(), int(query.data.size()), int(query.curr), 0, 0.f, std::max(upper, 17.f), ImVec2(0, 30));
    ImGui::PopStyleColor();
}

static void display_query_counter(const Query& query, const char* label="") {
    const float avg = query.exp_avg;
    const float lower = query.min();
    const float upper = query.max();
    ImGui::Text("avg: %uK, min: %uK, max: %uK", uint32_t(avg / 1000), uint32_t(lower / 1000), uint32_t(upper / 1000));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.f, .7f, .7f, 1.f));
    ImGui::PlotHistogram(label, query.data.data(), int(query.data.size()), int(query.curr), 0, 0.f, std::max(upper, 17.f), ImVec2(0, 30));
    ImGui::PopStyleColor();
}

static void draw_gui() {
    if (!show_gui) return;

    // show timers/queries in top left corner
    ImGui::SetNextWindowPos(ImVec2(10, 20));
    ImGui::SetNextWindowSize(ImVec2(350, 500));
    if (ImGui::Begin("CPU/GPU Timer", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground)) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, .9f);
        for (const auto& [name, query] : TimerQuery::map) {
            ImGui::Separator();
            display_query_timer(*query, name.c_str());
        }
        for (const auto& [name, query] : TimerQueryGL::map) {
            ImGui::Separator();
            display_query_timer(*query, name.c_str());
        }
        for (const auto& [name, query] : PrimitiveQueryGL::map) {
            ImGui::Separator();
            display_query_counter(*query, name.c_str());
        }
        for (const auto& [name, query] : FragmentQueryGL::map) {
            ImGui::Separator();
            display_query_counter(*query, name.c_str());
        }
        ImGui::PopStyleVar();
    }
    ImGui::End();

    static bool gui_show_cameras = false;
    static bool gui_show_textures = false;
    static bool gui_show_fbos = false;
    static bool gui_show_shaders = false;
    static bool gui_show_materials = false;
    static bool gui_show_geometries = false;
    static bool gui_show_drawelements = false;
    static bool gui_show_animations = false;

    if (ImGui::BeginMainMenuBar()) {
        // camera menu
        ImGui::Checkbox("cameras", &gui_show_cameras);
        ImGui::Separator();
        ImGui::Checkbox("textures", &gui_show_textures);
        ImGui::Separator();
        ImGui::Checkbox("fbos", &gui_show_fbos);
        ImGui::Separator();
        ImGui::Checkbox("shaders", &gui_show_shaders);
        ImGui::Separator();
        ImGui::Checkbox("materials", &gui_show_materials);
        ImGui::Separator();
        ImGui::Checkbox("geometries", &gui_show_geometries);
        ImGui::Separator();
        ImGui::Checkbox("drawelements", &gui_show_drawelements);
        ImGui::Separator();
        ImGui::Checkbox("animations", &gui_show_animations);
        ImGui::Separator();
        if (ImGui::Button("Screenshot"))
            Context::screenshot("screenshot.png");
        ImGui::EndMainMenuBar();
    }

    if (gui_show_cameras) {
        if (ImGui::Begin(std::string("Cameras (" + std::to_string(Camera::map.size()) + ")").c_str(), &gui_show_cameras)) {
            ImGui::Text("Current: %s", current_camera()->name.c_str());
            for (auto& [name, cam] : Camera::map) {
                if (ImGui::CollapsingHeader(name.c_str()))
                    display_camera(cam);
            }
        }
        ImGui::End();
    }

    if (gui_show_textures) {
        if (ImGui::Begin(std::string("Textures (" + std::to_string(Texture2D::map.size()) + ")").c_str(), &gui_show_textures)) {
            for (const auto& [name, tex] : Texture2D::map) {
                if (ImGui::CollapsingHeader(name.c_str()))
                    display_texture(tex, ImVec2(300, 300));
            }
        }
        ImGui::End();
    }

    if (gui_show_shaders) {
        if (ImGui::Begin(std::string("Shaders (" + std::to_string(Shader::map.size()) + ")").c_str(), &gui_show_shaders)) {
            for (auto& [name, shader] : Shader::map)
                if (ImGui::CollapsingHeader(name.c_str()))
                    display_shader(shader);
            if (ImGui::Button("Reload modified")) reload_modified_shaders();
        }
        ImGui::End();
    }

    if (gui_show_fbos) {
        if (ImGui::Begin(std::string("Framebuffers (" + std::to_string(Framebuffer::map.size()) + ")").c_str(), &gui_show_fbos)) {
            for (const auto& [name, fbo] : Framebuffer::map)
                if (ImGui::CollapsingHeader(name.c_str()))
                    display_framebuffer(fbo);
        }
        ImGui::End();
    }

    if (gui_show_materials) {
        if (ImGui::Begin(std::string("Materials (" + std::to_string(Material::map.size()) + ")").c_str(), &gui_show_materials)) {
            for (const auto& [name, mat] : Material::map)
                if (ImGui::CollapsingHeader(name.c_str()))
                    display_material(mat);
        }
        ImGui::End();
    }

    if (gui_show_geometries) {
        if (ImGui::Begin(std::string("Geometries (" + std::to_string(Geometry::map.size()) + ")").c_str(), &gui_show_geometries)) {
            for (const auto& [name, geom] : Geometry::map)
                if (ImGui::CollapsingHeader(name.c_str()))
                    display_geometry(geom);
        }
        ImGui::End();
    }

    if (gui_show_drawelements) {
        if (ImGui::Begin(std::string("Drawelements (" + std::to_string(Drawelement::map.size()) + ")").c_str(), &gui_show_drawelements)) {
            for (auto& [name, elem] : Drawelement::map)
                if (ImGui::CollapsingHeader(name.c_str()))
                    display_drawelement(elem);
        }
        ImGui::End();
    }

    if (gui_show_animations) {
        if (ImGui::Begin(std::string("Animation (" + std::to_string(Animation::map.size()) + ")").c_str(), &gui_show_animations)) {
            for (auto& [name, anim] : Animation::map)
                if (ImGui::CollapsingHeader(name.c_str()))
                    display_animation(anim);
        }
        ImGui::End();
    }

    for (auto fn : hook_gui_callbacks)
        fn();

    if (user_gui_callback)
        user_gui_callback();
}
