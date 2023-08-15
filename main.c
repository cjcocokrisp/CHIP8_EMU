#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.h"
// TODO: include graphics stuff

int main(int argc, char* argv[])
{
    srand(time(NULL));
    // TODO: Set up render system and register input callbacks

    // Initialize the chip8 system and load the program into memory
    CHIP8 hChip8 = chip8_init_default();
    if (hChip8 == NULL)
    {
        printf("Failed to allocate memory a Chip8 Object!\n");
        exit(1);
    }
    FILE* fp = fopen(argv[argc - 1], 'rb');
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

    // Emulation Loop
    for(;;)
    {
        // TODO: emulate one cycle

        // TODO: If draw flag is set, update the screen

        // TODO: Store key press state (Press and release)
    }

    chip8_destory(&hChip8);
    return 0;
}