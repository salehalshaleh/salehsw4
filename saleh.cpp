#include <iostream>
#include <cmath>
#include <vector>
#include <ctime>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 700;

// SHADERS
const char* vertexShaderSource = "#version 330 core\n"
"layout(location=0) in vec2 aPos;\n"
"uniform vec2 offset;\n"
"uniform vec2 scale;\n"
"uniform float rotation;\n"
"void main() {\n"
"   float s = sin(rotation);\n"
"   float c = cos(rotation);\n"
"   vec2 rotated = vec2(c * aPos.x - s * aPos.y, s * aPos.x + c * aPos.y);\n"
"   vec2 pos = rotated * scale + offset;\n"
"   gl_Position = vec4(pos,0.0,1.0);\n"
"}";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec3 color;\n"
"void main(){ FragColor=vec4(color,1.0); }";

// UTIL
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
static void processInput(GLFWwindow* window) { if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true); }
static void getMouseNDC(GLFWwindow* window, float& x, float& y) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    x = (2.0f * xpos) / SCR_WIDTH - 1.0f;
    y = 1.0f - (2.0f * ypos) / SCR_HEIGHT;
}

// Create circle vertices
std::vector<float> createCircle(float radius, int segments) {
    std::vector<float> verts;
    for (int i = 0; i < segments; i++) {
        float theta = 2.0f * (float)M_PI * float(i) / float(segments);
        float x = radius * cosf(theta);
        float y = radius * sinf(theta);
        verts.push_back(x); verts.push_back(y);
    }
    return verts;
}

int main() {
    srand(static_cast<unsigned int>(time(0)));
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Radar Mouse Signal", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return -1;

    // Shaders
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL); glCompileShader(vs);
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL); glCompileShader(fs);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs); glAttachShader(shaderProgram, fs); glLinkProgram(shaderProgram);
    glDeleteShader(vs); glDeleteShader(fs);
    glUseProgram(shaderProgram);

    // Unit line (radar beam)
    float line[] = { 0.0f,0.0f,1.0f,0.0f };
    unsigned int VAO_line, VBO_line;
    glGenVertexArrays(1, &VAO_line); glGenBuffers(1, &VBO_line);
    glBindVertexArray(VAO_line);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Circle radar
    std::vector<float> circle = createCircle(0.9f, 64);
    unsigned int VAO_circle, VBO_circle;
    glGenVertexArrays(1, &VAO_circle); glGenBuffers(1, &VBO_circle);
    glBindVertexArray(VAO_circle);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_circle);
    glBufferData(GL_ARRAY_BUFFER, circle.size() * sizeof(float), circle.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Circle for signal
    float point[] = { -0.05f,-0.05f, 0.05f,-0.05f, 0.05f,0.05f, 0.05f,0.05f, -0.05f,0.05f, -0.05f,-0.05f };
    unsigned int VAO_point, VBO_point;
    glGenVertexArrays(1, &VAO_point); glGenBuffers(1, &VBO_point);
    glBindVertexArray(VAO_point);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_point);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Uniforms
    int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    int scaleLoc = glGetUniformLocation(shaderProgram, "scale");
    int rotationLoc = glGetUniformLocation(shaderProgram, "rotation");
    int colorLoc = glGetUniformLocation(shaderProgram, "color");

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        float time = glfwGetTime();

        float mouseX, mouseY;
        getMouseNDC(window, mouseX, mouseY);

        glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw radar circle
        glBindVertexArray(VAO_circle);
        glUniform2f(offsetLoc, 0.0f, 0.0f);
        glUniform2f(scaleLoc, 1.0f, 1.0f);
        glUniform1f(rotationLoc, 0.0f);
        glUniform3f(colorLoc, 0.0f, 0.5f, 0.0f);
        glDrawArrays(GL_LINE_LOOP, 0, static_cast<GLsizei>(circle.size() / 2));

        // Draw radar rotating beam
        glBindVertexArray(VAO_line);
        glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f);
        glUniform2f(offsetLoc, 0.0f, 0.0f);
        glUniform2f(scaleLoc, 0.9f, 0.9f);
        glUniform1f(rotationLoc, time * 1.5f);
        glDrawArrays(GL_LINES, 0, 2);

        // Check if beam hits mouse
        float dx = mouseX;
        float dy = mouseY;
        float beamAngle = fmod(time * 1.5f, 2.0f * (float)M_PI);
        float mouseAngle = atan2f(dy, dx);
        float dist = sqrtf(dx * dx + dy * dy);

        bool hit = fabsf(mouseAngle - beamAngle) < 0.1f && dist < 0.9f;

        // Draw signal on mouse
        glBindVertexArray(VAO_point);
        glUniform2f(offsetLoc, mouseX, mouseY);
        glUniform2f(scaleLoc, 0.50f, 0.50f); // أكبر حجم للنقطة        glUniform1f(rotationLoc, 0.0f);
        if (hit) {
            glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f); // Red when beam hits
        }
        else {
            glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f);
        }
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}