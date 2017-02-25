#include <iostream>
#include <cstdlib>

#include <cmath>

#include <glm/glm.hpp>
#include <stb/stb_image.h>
#include "gl_includes.h"
#include "Perf.h"
#include "gjk.h"

using namespace std;
using namespace glm;

GLuint compileShader(const char *vertSrc, const char *fragSrc);
void loadTexture(GLuint texname, const char *filename);

GLFWwindow *window;


const char *vertShader = GLSL(
    layout(location = 0) uniform vec2 inv;

    layout(location = 0) in vec2 pos;

    void main() {
        gl_Position = vec4(pos.x * inv.x, pos.y * inv.y, 0.0, 1.0);
    }
);

// just make everything white for now.
const char *fragShader = GLSL(
    void main() {
        gl_FragColor = vec4(1.0);
    }
);


int nVerts = 0;

void setup() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    PolygonCollider2D line;
    line.points.emplace_back(-1,0);
    line.points.emplace_back(-1,3);
    line.points.emplace_back( 1,2);

    CircleCollider2D circle;
    circle.center = vec2(0, -1);
    circle.radius = 1;

    AddCollider2D longCircle;
    longCircle.a = &line;
    longCircle.b = &circle;

    vector<vec2> bounds;
    findBounds(&longCircle, bounds, 0.01);

    GLuint shader = compileShader(vertShader, fragShader);
    glUseProgram(shader);

    nVerts = bounds.size();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * nVerts, bounds.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);
    checkError();

}

void draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_LINE_LOOP, 0, nVerts);
}

static void glfw_resize_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    if (width != 0 && height != 0) {
        const float scale = 128.f;
        glUniform2f(0, scale / width, scale / height);
    }
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    } else if (key == GLFW_KEY_W) {
        static bool wireframe = false;
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }
}

static void glfw_click_callback(GLFWwindow *window, int button, int action, int mods) {
    if (action != GLFW_RELEASE) return;

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    // TODO
    // mouse clicks
}

void glfw_error_callback(int error, const char* description) {
    cerr << "GLFW Error: " << description << " (error " << error << ")" << endl;
}

void checkShaderError(GLuint shader) {
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success) return;

    cout << "Shader Compile Failed." << endl;

    GLint logSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
    if (logSize == 0) {
        cout << "No log found." << endl;
        return;
    }

    GLchar *log = new GLchar[logSize];

    glGetShaderInfoLog(shader, logSize, &logSize, log);

    cout << log << endl;

    delete[] log;
}

void checkLinkError(GLuint program) {
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success) return;

    cout << "Shader link failed." << endl;

    GLint logSize = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
    if (logSize == 0) {
        cout << "No log found." << endl;
        return;
    }

    GLchar *log = new GLchar[logSize];

    glGetProgramInfoLog(program, logSize, &logSize, log);
    cout << log << endl;

    delete[] log;
}

GLuint compileShader(const char *vertSrc, const char *fragSrc) {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertSrc, nullptr);
    glCompileShader(vertex);
    checkShaderError(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragSrc, nullptr);
    glCompileShader(fragment);
    checkShaderError(fragment);

    GLuint shader = glCreateProgram();
    glAttachShader(shader, vertex);
    glAttachShader(shader, fragment);
    glLinkProgram(shader);
    checkLinkError(shader);

    return shader;
}

void loadTexture(GLuint texname, const char *filename) {
    glBindTexture(GL_TEXTURE_2D, texname);

    int width, height, bpp;
    unsigned char *pixels = stbi_load(filename, &width, &height, &bpp, STBI_default);
    if (pixels == nullptr) {
        cout << "Failed to load image " << filename << " (" << stbi_failure_reason() << ")" << endl;
        return;
    }
    cout << "Loaded " << filename << ", " << height << 'x' << width << ", comp = " << bpp << endl;

    GLenum format;
    switch(bpp) {
        case STBI_rgb:
            format = GL_RGB;
            break;
        case STBI_rgb_alpha:
            format = GL_RGBA;
            break;
        default:
            cout << "Unsupported format: " << bpp << endl;
            return;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(pixels);
}

int main() {
    if (!glfwInit()) {
        cout << "Failed to init GLFW" << endl;
        exit(-1);
    }
    cout << "GLFW Successfully Started" << endl;

    glfwSetErrorCallback(glfw_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(640, 480, "2D GJK Collision Demo", NULL, NULL);
    if (!window) {
        cout << "Failed to create window" << endl;
        exit(-1);
    }
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetMouseButtonCallback(window, glfw_click_callback);

    glfwMakeContextCurrent(window);
    glewInit();

    glfwSwapInterval(1);

    initPerformanceData();

    setup();
    checkError();

    glfwSetFramebufferSizeCallback(window, glfw_resize_callback); // do this after setup
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glfw_resize_callback(window, width, height); // call resize once with the initial size

    // make sure performance data is clean going into main loop
    markPerformanceFrame();
    printPerformanceData();
    double lastPerfPrintTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {

        {
            Perf stat("Poll events");
            glfwPollEvents();
            checkError();
        }
        {
            Perf stat("Draw");
            draw();
            checkError();
        }
        {
            Perf stat("Swap buffers");
            glfwSwapBuffers(window);
            checkError();
        }

        markPerformanceFrame();

        double now = glfwGetTime();
        if (now - lastPerfPrintTime > 10.0) {
            printPerformanceData();
            lastPerfPrintTime = now;
        }
    }

    return 0;
}
