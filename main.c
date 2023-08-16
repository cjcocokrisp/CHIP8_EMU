#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "chip8.h"

unsigned int create_shader(const char* vertex_shader, const char* fragment_shader);
unsigned int compile_shader(unsigned int type, const char* source);

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Program Usage: CHIP8_EMU <rom_path>\n");
        exit(1);
    }
    srand(time(NULL));
    int size_modifer = 10;

    // Initialize the chip8 system and load the program into memory
    CHIP8 hChip8 = chip8_init_default();
    if (hChip8 == NULL)
    {
        printf("Failed to allocate memory a Chip8 Object!\n");
        exit(1);
    }
    FILE* fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        printf("Rom does not exist or failed to read!\n");
        exit(1);
    }
    Status load_status = chip8_load_rom(hChip8, fp); // ROM PROCESSING NEEDS TO BE ADDED
    if (load_status == FAILURE)
    {
        printf("Failed to load rom!\n");
        exit(1);
    }
    fclose(fp);

    // Init GLFW and create window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH * size_modifer, SCREEN_HEIGHT * size_modifer, "CHIP8 EMU", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to allocate space for the GLFW window!\n");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);

    // Load Glad and GL
    gladLoadGL();
    glViewport(0, 0, SCREEN_WIDTH * size_modifer, SCREEN_HEIGHT * size_modifer);

    float vertices[] = {
        -0.5f, 0.5f,
        -0.5f, -0.5f,
        0.5f, -0.5,
        0.5f, -0.5,
        0.5, 0.5,
        -0.5f, 0.5f
    };
    
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);
   
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);


    const char* vertex_shader_source = 
    "#version 330 core\n"
    "layout(location = 0) in vec4 position;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = position;\n"
    "}\0";
    const char* fragment_shader_source = 
    "#version 330 core\n"
    "layout(location = 0) out vec4 color;\n"
    "void main()\n"
    "{\n"
    "   color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
    "}\0";
    unsigned int shader = create_shader(vertex_shader_source, fragment_shader_source);
    glUseProgram(shader);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Emulation Loop
    while (!glfwWindowShouldClose(window))
    {
        // TODO: emulate one cycle
        chip8_emulate_cycle(hChip8);

        // TODO: If draw flag is set, update the screen
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // TODO: Store key press state (Press and release)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    chip8_destory(&hChip8);
    glDeleteProgram(shader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

unsigned int create_shader(const char* vertex_shader, const char* fragment_shader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);
    unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

unsigned int compile_shader(unsigned int type, const char* source)
{
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    return id;
}