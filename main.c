#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "chip8.h"
#include <windows.h>

unsigned int create_shader(const char* vertex_shader, const char* fragment_shader);
unsigned int compile_shader(unsigned int type, const char* source);
void draw_frame(unsigned char* gfx, int window_width, int window_height);
void handle_keys(GLFWwindow* window, Boolean* exit_flag);
void sleep(unsigned int mseconds);


CHIP8 hChip8;
Boolean debug;
Boolean debug_enabled;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Program Usage: CHIP8_EMU <rom_path>\n");
        exit(1);
    }

    debug_enabled = FALSE;
    int size_modifer = 10;
    if (argc > 2)
    {
        if (!strcmp(argv[2], "--debug") || !strcmp(argv[3], "--debug"))
        {
            printf("Program launched with debug enabled press P to start debugging!\n");
            debug_enabled = TRUE;
        }
    }

    Boolean exit_flag = FALSE;
    
    srand(time(NULL));

    // Initialize the chip8 system and load the program into memory
    hChip8 = chip8_init_default();
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
    Status load_status = chip8_load_rom(hChip8, fp);
    if (load_status == FAILURE)
    {
        printf("Failed to load rom!\n");
        exit(1);
    }
    fclose(fp);

    const int WINDOW_WIDTH = SCREEN_WIDTH * size_modifer;
    const int WINDOW_HEIGHT = SCREEN_HEIGHT * size_modifer;
    // Init GLFW and create window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CHIP8 EMU", NULL, NULL);
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

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
   
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);;

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
    glfwSwapBuffers(window);

    // Emulation Loop
    while (!glfwWindowShouldClose(window))
    {
        if (debug)
            chip8_debug(hChip8, &debug);
        chip8_emulate_cycle(hChip8);

        if(chip8_get_draw_flag(hChip8))
        {
            draw_frame(chip8_get_gfx(hChip8), WINDOW_WIDTH, WINDOW_HEIGHT);
            glfwSwapBuffers(window);
            chip8_set_draw_flag(hChip8, FALSE);
        }
        
        handle_keys(window, &exit_flag);
        if (chip8_get_sound_timer(hChip8) == 1)
            PlaySound("beep.wav", NULL, SND_FILENAME | SND_ASYNC);
        glfwPollEvents();
        if (exit_flag)
            break;

        sleep(2);
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

void draw_frame(unsigned char* gfx, int window_width, int window_height)
{
    float width = (float)window_width / 2;
    float height = (float)window_height / 2;
    float width_modifer = (float)window_width / (float)SCREEN_WIDTH;
    float height_modifer = (float)window_height / (float)SCREEN_HEIGHT;
    float vertices[8];
    unsigned int elements[] = {0, 3, 1, 1, 2, 0}; // Order of drawing verticies to get the triangle

    glClear(GL_COLOR_BUFFER_BIT);
    for (int r = 0; r < SCREEN_HEIGHT; r++)
    {
        for (int c = 0; c < SCREEN_WIDTH; c++)
        {
            if (gfx[r * SCREEN_WIDTH + c] != 0)
            {
                vertices[0] = ((((float)c * width_modifer) - width) / width);
                vertices[1] = -((((float)r * height_modifer) - height) / height) - 0.05;
                // Bottom Right X and Y
                vertices[2] = ((((float)c * width_modifer) - width) / width) + (width_modifer / width);
                vertices[3] = -((((float)r * height_modifer) - height) / height) + (height_modifer / height) - 0.05;
                // Top Right X and Y
                vertices[4] = ((((float)c * width_modifer) - width) / width) + (width_modifer / width);
                vertices[5] = -((((float)r * height_modifer) - height) / height) - 0.05;
                // Bottom Left X and Y
                vertices[6] = ((((float)c * width_modifer) - width) / width);
                vertices[7] = -((((float)r * height_modifer) - height) / height) + (height_modifer / height) - 0.05;

                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_DYNAMIC_DRAW);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
    }
}

void handle_keys(GLFWwindow* window, Boolean* exit_flag)
{
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) chip8_set_key(hChip8, 0x1, 1);
    else                                              chip8_set_key(hChip8, 0x1, 0);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) chip8_set_key(hChip8, 0x2, 1);
    else                                              chip8_set_key(hChip8, 0x2, 0);
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) chip8_set_key(hChip8, 0x3, 1);
    else                                              chip8_set_key(hChip8, 0x3, 0);
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) chip8_set_key(hChip8, 0xC, 1);
    else                                              chip8_set_key(hChip8, 0xC, 0);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) chip8_set_key(hChip8, 0x4, 1);
    else                                              chip8_set_key(hChip8, 0x4, 0);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) chip8_set_key(hChip8, 0x5, 1);
    else                                              chip8_set_key(hChip8, 0x5, 0);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) chip8_set_key(hChip8, 0x6, 1);
    else                                              chip8_set_key(hChip8, 0x6, 0);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) chip8_set_key(hChip8, 0xD, 1);
    else                                              chip8_set_key(hChip8, 0xD, 0);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) chip8_set_key(hChip8, 0x7, 1);
    else                                              chip8_set_key(hChip8, 0x7, 0);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) chip8_set_key(hChip8, 0x8, 1);
    else                                              chip8_set_key(hChip8, 0x8, 0);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) chip8_set_key(hChip8, 0x9, 1);
    else                                              chip8_set_key(hChip8, 0x9, 0);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) chip8_set_key(hChip8, 0xE, 1);
    else                                              chip8_set_key(hChip8, 0xE, 0);
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) chip8_set_key(hChip8, 0xA, 1);
    else                                              chip8_set_key(hChip8, 0xA, 0);
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) chip8_set_key(hChip8, 0x0, 1);
    else                                              chip8_set_key(hChip8, 0x0, 0);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) chip8_set_key(hChip8, 0xB, 1);
    else                                              chip8_set_key(hChip8, 0xB, 0);
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) chip8_set_key(hChip8, 0xF, 1);
    else                                              chip8_set_key(hChip8, 0xF, 0);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) debug = TRUE;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) *exit_flag = TRUE;
}

void sleep(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}