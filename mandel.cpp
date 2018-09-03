#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>

#define MAJOR_VERSION 3
#define MINOR_VERSION 1

using namespace glm;

vec3 resolution = glm::vec3(1900, 1000, 0.0);
int running = 1, lock = 0;
int max_steps = 100;
int selection = 0;
vec4 background_color = vec4(0.0, 0.0, 0.0, 1.0);

vec3 color0 = vec3(0.1, 0.1, 0.1);
vec3 color1 = vec3(0.3, 0.3, 0.3);
vec3 color2 = vec3(0.5, 0.5, 0.5);
vec3 color3 = vec3(0.75, 0.75, 0.75);
vec3 color_base = vec3(0.25, 0.0, 0.25);
float base_color_strength = 0.5;
float dist0to1 = 0.2;
float dist1to2 = 0.5;
float dist2to3 = 0.7;
float dist3to0 = 0.1;
vec4 orbit_strength = vec4(-1.0, -1.8, -1.5, 1.3);
float palette_offset = 0;
float cycle_intensity = 1.0;

float ambient_intensity = 1.0;
float diffuse_intensity = 1.0;
float specular_intenisty = 1.0;
float shine = 30;
int gamma_correction = 0;

int mandelbulb_iterations = 100;
float mandelbulb_power = 2.0f;
float mandelbulb_bailout = 8.0f;

float mandelbox_scale = -2.77f;
float mandelbox_fixed_radius = 1.1f;
float mandelbox_min_radius = 0.025f;

int light_method = 1;
vec3 light_position = vec3(2.0, -5.0, 3.0);

Camera camera;

char *file_to_string(const char *file_name);
GLuint build_shader();
void compile_shader(const char *str, GLuint id);
void input(Camera &camera, double delta);

int main(int argc, char *argv[])
{

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, MINOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window *window = NULL;

    window = SDL_CreateWindow(
        "Ray March", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED,
        resolution.x,
        resolution.y,
        SDL_WINDOW_OPENGL
    );

    assert(window != NULL);

    SDL_GLContext glcon = SDL_GL_CreateContext(window);
    assert(glcon);

    GLenum err = glewInit();
    assert(err == GLEW_OK);

    SDL_GL_SetSwapInterval(1);

    GLuint shaders = 0;
    shaders = build_shader();

    float vertex_data[] =
    {
        -1.0f,  1.0f,  0.0f,
	    -1.0f,  -1.0f,  0.0f,
	    1.0f,  -1.0f,  0.0f,

	    -1.0f,  1.0f,  0.0f,
	    1.0f,  -1.0f,  0.0f,
        1.0f, 1.0f, 0.0f,
    };

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), vertex_data, GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glUseProgram(shaders);

    glBindVertexArray(vao);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplSDL2_InitForOpenGL(window, glcon);
    ImGui_ImplOpenGL3_Init("#version 400");
    ImGui::StyleColorsClassic();

    Uint64 now;
    Uint64 last_time = SDL_GetPerformanceCounter();
    double delta = 0;
    double ns = 1000000000.0 / 60.0;
    while(running)
    {
        now = SDL_GetPerformanceCounter();
        delta += (now - last_time) / ns;
        while(delta >= 1.0)
        {
            input(camera, delta);
            delta--;
        }

        glUniform3fv(glGetUniformLocation(shaders, "resolution"), 1, value_ptr(resolution));
        glUniform3fv(glGetUniformLocation(shaders, "center"), 1, value_ptr(camera.get_center()));
        glUniform3fv(glGetUniformLocation(shaders, "eye"), 1, value_ptr(camera.get_eye()));
        glUniform3fv(glGetUniformLocation(shaders, "up"), 1, value_ptr(camera.get_up()));

        glUniform1i(glGetUniformLocation(shaders, "selection"), selection);
        glUniform1i(glGetUniformLocation(shaders, "max_steps"), max_steps);
        glUniform3fv(glGetUniformLocation(shaders, "background_color"), 1, value_ptr(background_color));

        glUniform3fv(glGetUniformLocation(shaders, "color0"), 1, value_ptr(color0));
        glUniform3fv(glGetUniformLocation(shaders, "color1"), 1, value_ptr(color1));
        glUniform3fv(glGetUniformLocation(shaders, "color2"), 1, value_ptr(color2));
        glUniform3fv(glGetUniformLocation(shaders, "color3"), 1, value_ptr(color3));
        glUniform3fv(glGetUniformLocation(shaders, "color_base"), 1, value_ptr(color_base));
        glUniform1f(glGetUniformLocation(shaders, "base_color_strength"), base_color_strength);
        glUniform1f(glGetUniformLocation(shaders, "dist0to1"), dist0to1);
        glUniform1f(glGetUniformLocation(shaders, "dist1to2"), dist1to2);
        glUniform1f(glGetUniformLocation(shaders, "dist2to3"), dist2to3);
        glUniform1f(glGetUniformLocation(shaders, "dist3to0"), dist3to0);
        glUniform1f(glGetUniformLocation(shaders, "cycle_intensity"), cycle_intensity);
        glUniform1f(glGetUniformLocation(shaders, "palette_offset"), palette_offset);
        glUniform4fv(glGetUniformLocation(shaders, "orbit_strength"), 1, value_ptr(orbit_strength));

        glUniform1f(glGetUniformLocation(shaders, "shine"), shine);
        glUniform1f(glGetUniformLocation(shaders, "ambient_intensity"), ambient_intensity);
        glUniform1f(glGetUniformLocation(shaders, "diffuse_intensity"), diffuse_intensity);
        glUniform1f(glGetUniformLocation(shaders, "specular_intensity"), specular_intenisty);

        glUniform1i(glGetUniformLocation(shaders, "mandelbulb_iterations"), mandelbulb_iterations);
        glUniform1f(glGetUniformLocation(shaders, "mandelbulb_bailout"), mandelbulb_bailout);
        glUniform1f(glGetUniformLocation(shaders, "mandelbulb_power"), mandelbulb_power);

        glUniform1f(glGetUniformLocation(shaders, "mandelbox_scale"), mandelbox_scale);
        glUniform1f(glGetUniformLocation(shaders, "mandelbox_fixed_radius"), mandelbox_fixed_radius);
        glUniform1f(glGetUniformLocation(shaders, "mandelbox_min_radius"), mandelbox_min_radius);

        glUniform1i(glGetUniformLocation(shaders, "light_method"), light_method);
        glUniform3fv(glGetUniformLocation(shaders, "light_position"), 1, value_ptr(light_position));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0, 1.0, 1.0, 1.0);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        ImGui::Begin("Raymarch Viewer");
        ImGui::Text("Basic settings");
        ImGui::SliderInt("Shape selection", &selection, 0, 2);
        ImGui::SliderInt("Max steps", &max_steps, 10, 500);
        ImGui::SliderFloat("Zoom modifier", camera.getScrollFactor(), 0.001, 0.1);
        ImGui::SliderFloat("Camera speed", camera.getCameraSpeed(), 0.001, 10.0);
        ImGui::ColorEdit3("Background color", (float*)&background_color);
        ImGui::Separator();
        ImGui::Text("Orbit trap coloring");
        ImGui::ColorEdit3("Color 0", (float*)&color0);
        ImGui::ColorEdit3("Color 1", (float*)&color1);
        ImGui::ColorEdit3("Color 2", (float*)&color2);
        ImGui::ColorEdit3("Color 3", (float*)&color3);
        ImGui::ColorEdit3("Base color", (float*)&color_base);
        ImGui::SliderFloat("Base color strength", &base_color_strength, 0, 3.0);
        ImGui::SliderFloat("Distance 0 -> 1", &dist0to1, 0, 3.0);
        ImGui::SliderFloat("Distance 1 -> 2", &dist1to2, 0, 3.0);
        ImGui::SliderFloat("Distance 2 -> 3", &dist2to3, 0, 3.0);
        ImGui::SliderFloat("Distance 3 -> 0", &dist3to0, 0, 3.0);
        ImGui::SliderFloat("Cycle intensity", &cycle_intensity, 0.01, 6.0);
        ImGui::SliderFloat("Palette offset", &palette_offset, 0.0, 100.0);
        ImGui::SliderFloat("Orbit x strength", &orbit_strength[0], -3.0, 3.0);
        ImGui::SliderFloat("Orbit y strength", &orbit_strength[1], -3.0, 3.0);
        ImGui::SliderFloat("Orbit z strength", &orbit_strength[2], -3.0, 3.0);
        ImGui::SliderFloat("Orbit w strength", &orbit_strength[3], -3.0, 3.0);
        ImGui::Separator();
        ImGui::Text("Lighting");
        ImGui::SliderInt("Lighting method", &light_method, 0, 1);
        ImGui::SliderFloat("Light position x", &light_position.x, -10.0, 10.0);
        ImGui::SliderFloat("Light position y", &light_position.y, -10.0, 10.0);
        ImGui::SliderFloat("Light position z", &light_position.z, -10.0, 10.0);
        ImGui::SliderFloat("Shine", &shine, 1.0, 100.0);
        ImGui::SliderFloat("Ambient", &ambient_intensity, 0.0, 1.0);
        ImGui::SliderFloat("Diffuse", &diffuse_intensity, 0.0, 1.0);
        ImGui::SliderFloat("Specular", &specular_intenisty, 0.0, 1.0);
        ImGui::Checkbox("Gamma correction", (bool *)&gamma_correction);
        ImGui::End();

        switch(selection)
        {
            case 0:
                ImGui::Begin("Mandelbox Settings");
                ImGui::SliderFloat("Scale", &mandelbox_scale, -5.0, 5.0);
                ImGui::SliderFloat("Fixed radius", &mandelbox_fixed_radius, 0.001, 3.0);
                ImGui::SliderFloat("Min radius", &mandelbox_min_radius, 0.001, 1.0);
                ImGui::End();
                break;
            case 1:
                ImGui::Begin("Mandelbulb Settings");
                ImGui::SliderInt("Iterations", &mandelbulb_iterations, 1, 1000);
                ImGui::SliderFloat("Bailout", &mandelbulb_bailout, 1.0, 32.0);
                ImGui::SliderFloat("Power", &mandelbulb_power, 1.0, 8.0);
                ImGui::End();
                break;
            case 2:
                break;
            default:
                break;
        }

        ImGui::Render();

        SDL_GL_MakeCurrent(window, glcon);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(glcon);
    SDL_DestroyWindow(window);
    SDL_Quit();
    glDeleteProgram(shaders);
    glDeleteVertexArrays(1, &vao);
    return 0;
}

void input(Camera &camera, double delta)
{
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
        ImGui_ImplSDL2_ProcessEvent(&e);
        switch(e.type)
        {
            case SDL_QUIT: running = 0; break;
            case SDL_MOUSEWHEEL:
                camera.scroll(e.wheel.y);
                break;
            case SDL_MOUSEMOTION:
                if(!lock)
                    camera.rotate(e.motion.xrel, e.motion.yrel);
                break;
            case SDL_KEYDOWN:
                switch(e.key.keysym.sym)
                {
                    case SDLK_ESCAPE: running = 0; break;
                    case SDLK_w: camera.move(0.0, -1.0); break;
                    case SDLK_s: camera.move(0.0, 1.0); break;
                    case SDLK_a: camera.move(1.0, 0.0); break;
                    case SDLK_d: camera.move(-1.0, 0.0); break;
                    case SDLK_q: lock = !lock; break;
                    default: break;
                }
            default: break;
        }
    }
}

void compile_shader(const char *str, GLuint id)
{
    assert(id);
    GLint result = GL_FALSE;
    int error_length;
    const char *ptr = str;
    glShaderSource(id, 1, &ptr, NULL);
    glCompileShader(id);
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &error_length);
    if(error_length > 0 && result == GL_FALSE)
    {
        char msg[error_length + 1];
        glGetShaderInfoLog(id, error_length, NULL, &msg[0]);
        printf("%s\n", msg);
        exit(1);
    }
}

GLuint build_shader()
{
    GLuint vs_id = 0;
    GLuint fs_id = 0;

    vs_id = glCreateShader(GL_VERTEX_SHADER);
    fs_id = glCreateShader(GL_FRAGMENT_SHADER);

    assert(vs_id);
    assert(fs_id);

    char *vs_str = file_to_string("vertex.glsl");
    char *fs_str = file_to_string("fragment.glsl");

    assert(fs_str);

    compile_shader(vs_str, vs_id);
    compile_shader(fs_str, fs_id);

    GLuint shader_program_id = glCreateProgram();
    glAttachShader(shader_program_id, vs_id);
    glAttachShader(shader_program_id, fs_id);
    glLinkProgram(shader_program_id);

    GLint result = GL_FALSE;
    int error_length;
    glGetProgramiv(shader_program_id, GL_LINK_STATUS, &result);
    glGetProgramiv(shader_program_id, GL_INFO_LOG_LENGTH, &error_length);
    if(error_length > 0 && result == GL_FALSE)
    {
        char msg[error_length + 1];
        glGetProgramInfoLog(shader_program_id, error_length, NULL, &msg[0]);
        printf("%s\n", msg);
        exit(1);
    }

    glDetachShader(shader_program_id, vs_id);
    glDetachShader(shader_program_id, fs_id);
    glDeleteShader(fs_id);
    glDeleteShader(vs_id);

    assert(shader_program_id > 0);

    free(vs_str);
    free(fs_str);

    return shader_program_id;
}

char *file_to_string(const char *file_name)
{
    char *buffer = 0;
    unsigned long len;
    FILE *fp = fopen(file_name, "r");
    assert(fp);

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer = (char *)malloc(len + 1);
    assert(buffer);

    fread(buffer, 1, len, fp);
    fclose(fp);
    buffer[len] = '\0';

    return buffer;
}