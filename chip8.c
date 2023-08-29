#include <stdio.h>
#include <stdlib.h>
#include "chip8.h"

typedef struct chip8
{
    unsigned short opcode;
    unsigned char* memory;
    unsigned char* V;
    unsigned short I;
    unsigned short pc;
    unsigned char* gfx;
    unsigned short* stack;
    unsigned short sp;
    unsigned char* key;
    Boolean draw_flag;
    unsigned char delay_timer;
    unsigned char sound_timer;
} Chip8;

// Chip8 Fontset
unsigned char chip8_fontset[80] =
{ 
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

CHIP8 chip8_init_default(void)
{
    Chip8* pChip8 = (Chip8*)malloc(sizeof(Chip8));
    if (pChip8 != NULL)
    {
        pChip8->opcode = 0;
        pChip8->memory = (unsigned char*)calloc(sizeof(unsigned char) * MEMORY_SIZE, sizeof(unsigned char));
        if (pChip8->memory == NULL)
        {
            free(pChip8);
            return NULL;
        }
        pChip8->V = (unsigned char*)calloc(sizeof(unsigned char) * CPU_REGISTERS, sizeof(unsigned char));
        if (pChip8->V == NULL)
        {
            free(pChip8->memory);
            free(pChip8);
            return NULL;
        }
        pChip8->I = 0;
        pChip8->pc = 0x200;
        pChip8->gfx = (unsigned char*)calloc((SCREEN_WIDTH * SCREEN_HEIGHT) * sizeof(unsigned char), sizeof(unsigned char));
        if (pChip8->gfx == NULL)
        {
			free(pChip8->V);
			free(pChip8->memory);
            free(pChip8);
            return NULL;
        }
        pChip8->stack = (unsigned short*)calloc(sizeof(unsigned short) * STACK_SIZE, sizeof(unsigned char));
		if (pChip8->stack == NULL)
		{
			free(pChip8->gfx);
			free(pChip8->V);
			free(pChip8->memory);
            free(pChip8);
			return NULL;
		}
        pChip8->sp = 0;
        pChip8->key = (unsigned char*)calloc(sizeof(unsigned char) * NUM_OF_KEYS, sizeof(unsigned char));
		if (pChip8->key == NULL)
		{
			free(pChip8->stack);
			free(pChip8->gfx);
			free(pChip8->V);
			free(pChip8->memory);
            free(pChip8);
            return NULL;
		}
        pChip8->draw_flag = FALSE;
        pChip8->delay_timer = 0;
        pChip8->sound_timer = 0;
        // Load font into memory
        for (int i = 0; i < 80; i++)
            pChip8->memory[i] = chip8_fontset[i];
    }
    return pChip8;
}

Status chip8_load_rom(CHIP8 hChip8, FILE* fp)
{
    Chip8* pChip8 = (Chip8*)hChip8;
    fseek(fp, 0, SEEK_END);
    long rom_size = ftell(fp);
    rewind(fp);

    char* buffer = (char*)malloc(sizeof(char) * rom_size);
    if (buffer == NULL)
    {
        return FAILURE;
    }
    size_t buffer_size = fread(buffer, 1, rom_size, fp);
    if (buffer_size != rom_size)
    {
        return FAILURE;
    }

    if (MEMORY_SIZE - 0x200 > rom_size)
    {
        for (int i = 0; i < rom_size; i++)
        {
            pChip8->memory[i + 0x200] = buffer[i];
        }
    }
    else
    {
        return FAILURE;
    }

    free(buffer);
    return SUCCESS;
}

void chip8_emulate_cycle(CHIP8 hChip8)
{
    Chip8* pChip8 = (Chip8*)hChip8;
    // Fetch Opcode
    pChip8->opcode = pChip8->memory[pChip8->pc] << 8 | pChip8->memory[pChip8->pc + 1];
    // Decode and Execute Opcode
    // Opcodes from https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
    switch (pChip8->opcode & 0xF000)
    {
    case 0x0000: // 0NNN - Calls machine code routine at NNN
        switch (pChip8->opcode & 0x000F)
        {
            case 0x0000: // 00E0 - Clears the Screen
                for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
                {
                    pChip8->gfx[i] = 0;
                }
                pChip8->draw_flag = TRUE;
                pChip8->pc += 2;
                break;
            case 0x000E: // 00EE - Returns from a Subroutine
                pChip8->sp--;
                pChip8->pc = pChip8->stack[pChip8->sp];
                pChip8->pc += 2;
                break;
            default:
                printf("Unknown Opcode [0x0000]: 0x%x\n", pChip8->opcode);
        }
        break;
    case 0x1000: // 1NNN - Jumps to address NNN
        pChip8->pc = pChip8->opcode & 0x0FFF;
        break;
    case 0x2000: // 2NNN - Calls subroutine at NNN
        pChip8->stack[pChip8->sp] = pChip8->pc;
        pChip8->sp++;
        pChip8->pc = pChip8->opcode & 0x0FFF;
        break;
    case 0x3000: // 3XNN - Skips the next instruction if VX equals NN (usually the next instruction is a jump to skip a code block)
        if ((pChip8->V[(pChip8->opcode & 0x0F00) >> 8]) == (pChip8->opcode & 0x00FF))
            pChip8->pc += 4;
        else
            pChip8->pc += 2;
        break;
    case 0x4000: // 4XNN - Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block)
        if ((pChip8->V[(pChip8->opcode & 0x0F00) >> 8]) != (pChip8->opcode & 0x00FF))
            pChip8->pc += 4;
        else
            pChip8->pc += 2;
        break;
    case 0x5000: // 5XY0 - Skips the next instruction if VX equals VY (usually the next instruction is a jump to skip a code block)
        if ((pChip8->V[(pChip8->opcode & 0x0F00) >> 8]) == (pChip8->V[(pChip8->opcode & 0x00F0) >> 4]))
            pChip8->pc += 4;
        else
            pChip8->pc += 2;
        break;
    case 0x6000: // 6XNN - Sets VX to NN
        pChip8->V[(pChip8->opcode & 0x0F00) >> 8] = pChip8->opcode & 0x00FF;
        pChip8->pc += 2;
        break;
    case 0x7000: // 7XNN - Adds NN to VX (carry flag is not changed)
        pChip8->V[(pChip8->opcode & 0x0F00) >> 8] += pChip8->opcode & 0x00FF;
        pChip8->pc += 2;
        break;
    case 0x8000:
        switch(pChip8->opcode & 0x000F)
        {
            case 0x0000: // 8XY0 - Sets VX to the value of XY
                pChip8->V[(pChip8->opcode & 0x0F00) >> 8] = pChip8->V[(pChip8->opcode & 0x00F0) >> 4];
                pChip8->pc += 2;
                break;
            case 0x0001: // 8XY1 - Sets VX to VX or VY (bitwise OR operation)
                pChip8->V[(pChip8->opcode & 0x0F00) >> 8] |= pChip8->V[(pChip8->opcode & 0x00F0) >> 4];
                pChip8->V[0xF] = 0;
                pChip8->pc += 2;
                break;
            case 0x0002: // 8XY2 - Sets VX to VX and VY (bitwise AND operation)
                pChip8->V[(pChip8->opcode & 0x0F00) >> 8] &= pChip8->V[(pChip8->opcode & 0x00F0) >> 4];
                pChip8->V[0xF] = 0;
                pChip8->pc += 2;
                break;
            case 0x0003: // 8XY3 - Sets VX to VX xor VY (bitwise XOR operation)
                pChip8->V[(pChip8->opcode & 0x0F00) >> 8] ^= pChip8->V[(pChip8->opcode & 0x00F0) >> 4];
                pChip8->V[0xF] = 0;
                pChip8->pc += 2;
                break;
            case 0x0004: // 8XY4 - Adds VY to VX, VF is set to 1 when there's a carry, and to 0 when there is not
                if (pChip8->V[(pChip8->opcode & 0x0F00) >> 8] + pChip8->V[(pChip8->opcode & 0x00F0) >> 4] > 0xFF)
                {
                    pChip8->V[(pChip8->opcode & 0x0F00) >> 8] += pChip8->V[(pChip8->opcode & 0x00F0) >> 4];
                    pChip8->V[0xF] = 1;
                }
                else
                {
                    pChip8->V[(pChip8->opcode & 0x0F00) >> 8] += pChip8->V[(pChip8->opcode & 0x00F0) >> 4];
                    pChip8->V[0xF] = 0;
                }
                pChip8->pc += 2;
                break;
            case 0x0005: // 8XY5 - VY is subtracted from VX, VF is set to 0 when there's a borrow, and 1 when there is not
                if (pChip8->V[(pChip8->opcode & 0x0F00) >> 8] - pChip8->V[(pChip8->opcode & 0x00F0) >> 4] < 0x00)
                {
                    pChip8->V[(pChip8->opcode & 0x0F00) >> 8] -= pChip8->V[(pChip8->opcode & 0x00F0) >> 4];
                    pChip8->V[0xF] = 0;
                }
                else
                {
                    pChip8->V[(pChip8->opcode & 0x0F00) >> 8] -= pChip8->V[(pChip8->opcode & 0x00F0) >> 4];
                    pChip8->V[0xF] = 1; 
                }
                pChip8->pc += 2;
                break;
            case 0x0006: // 8XY6 - Stores the least significant bit of VX in VF and then shifts VX to the right by 1
                pChip8->V[0xF] = pChip8->V[(pChip8->opcode & 0x0F00) >> 8] & 0x1;
                pChip8->V[(pChip8->opcode & 0x0F00) >> 8] >>= 1;
                pChip8->pc += 2;
                break;
            case 0x0007: // 8XY7 - Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not
                pChip8->V[(pChip8->opcode & 0x0F00) >> 8] = pChip8->V[(pChip8->opcode & 0x00F0) >> 4] - pChip8->V[(pChip8->opcode & 0x0F00) >> 8];
                if (pChip8->V[(pChip8->opcode & 0x0F00) >> 8] > pChip8->V[(pChip8->opcode & 0x00F0) >> 4])
                    pChip8->V[0xF] = 0;
                else 
                    pChip8->V[0xF] = 1;
                pChip8->pc += 2;
                break;
            case 0x000E: // 8XYE - Stores the most significant bit of VX in VF and then shifts VX to the left by 1
                pChip8->V[0xF] = pChip8->V[(pChip8->opcode & 0x0F00) >> 8] >> 0x7;
                pChip8->V[(pChip8->opcode & 0x0F00) >> 8] <<= 1;
                pChip8->pc += 2;
                break;
            default:
                printf("Unknown Opcode [0x8000]: 0x%x\n", pChip8->opcode);
        }
        break;
    case 0x9000: // 9XY0 - Skips the next instruction if VX does not equal VY (usually the next instruction is a jump to skip a code block)
        if ((pChip8->V[(pChip8->opcode & 0x0F00) >> 8]) != (pChip8->V[(pChip8->opcode & 0x00F0) >> 4]))
            pChip8->pc += 4;
        else
            pChip8->pc += 2;
        break;
    case 0xA000: // ANNN - Sets I to the address NNN
        pChip8->I = pChip8->opcode & 0x0FFF;
        pChip8->pc += 2;
        break;
    case 0xB000: // BNNN - Jumps to the address NNN plus V0
        pChip8->pc = (pChip8->opcode & 0x0FFF) + pChip8->V[0x0];
        break; 
    case 0xC000: // CXNN - Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN
        pChip8->V[(pChip8->opcode & 0x0F00) >> 8] = (rand() % (255 + 1 - 0) + 0) & (pChip8->opcode & 0x00FF);
        pChip8->pc += 2;
        break;
    case 0xD000: // DXYN - Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels
                 // Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction 
                 // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
        {
            unsigned char x = pChip8->V[(pChip8->opcode & 0x0F00) >> 8];
            unsigned char y = pChip8->V[(pChip8->opcode & 0x00F0) >> 4];
            unsigned char height = pChip8->opcode & 0x000F;
            unsigned char current_row;

            pChip8->V[0xF] = 0;
            for (int current_y = 0; current_y < height; current_y++)
            {
                current_row = pChip8->memory[pChip8->I + current_y];
                for (int current_x = 0; current_x < 8; current_x++)
                {
                    if ((current_row & (0x80 >> current_x)) != 0)
                    { 
                        if (pChip8->gfx[x + current_x + ((y + current_y) * SCREEN_WIDTH)] == 1)
                            pChip8->V[0xF] = 1;
                        pChip8->gfx[x + current_x + ((y + current_y) * SCREEN_WIDTH)] ^= 1;
                    }
                }   
            }

            pChip8->draw_flag = TRUE;
            pChip8->pc += 2;
        } 
        break;
    case 0xE000:
        switch(pChip8->opcode & 0x00FF)
        {
        case 0x009E: // EX9E - Skips the next instruction if the key stored in VX is pressed (usually the next instruction is a jump to skip a code block)
            if (pChip8->key[pChip8->V[(pChip8->opcode & 0x0F00) >> 8]] != 0)
                pChip8->pc += 4;
            else
                pChip8->pc += 2;
            break;
        case 0x00A1: // EXA1 - Skips the next instruction if the key stored in VX is not pressed (usually the next instruction is a jump to skip a code block)
            if (pChip8->key[pChip8->V[(pChip8->opcode & 0x0F00) >> 8]] == 0)
                pChip8->pc += 4;
            else
                pChip8->pc += 2;
            break;
        default:
            printf("Unknown Opcode [0xE000]: 0x%x\n", pChip8->opcode);
        }
        break;
    case 0xF000:
        switch (pChip8->opcode & 0x00FF)
        {
        case 0x0007: // FX07 - Sets VX to the value of the delay timer
            pChip8->V[(pChip8->opcode & 0x0F00) >> 8] = pChip8->delay_timer;
            pChip8->pc += 2;
            break;
        case 0x000A: // FX0A - A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event)
            {
                Boolean key_pressed = FALSE;
                for (int i = 0; i < NUM_OF_KEYS; i++)
                {
                    if (pChip8->key[i] == 1)
                    {
                        key_pressed = TRUE;
                        pChip8->V[(pChip8->opcode & 0x0F00) >> 8] = i;
                    }
                }
                if (!key_pressed)
                    return;
                pChip8->pc += 2;
            }
            break;
        case 0x0015: // FX15 - Sets the delay timer to VX
            pChip8->delay_timer = pChip8->V[(pChip8->opcode & 0x0F00) >> 8];
            pChip8->pc += 2;
            break;
        case 0x0018: // FX18 - Sets the sound timer to VX
            pChip8->sound_timer = pChip8->V[(pChip8->opcode & 0x0F00) >> 8];
            pChip8->pc += 2;
            break;
        case 0x001E: // FX1E - Adds VX to I. VF is not affected
            pChip8->I += pChip8->V[(pChip8->opcode & 0x0F00) >> 8];
            pChip8->pc += 2;
            break;
        case 0x0029: // FX29 - Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
            pChip8->I = pChip8->V[(pChip8->opcode & 0x0F00) >> 8] * 0x5;
            pChip8->pc += 2;
            break;
        case 0x0033: // FX33 - Stores the binary-coded decimal representation of VX, with the hundreds digit in
                     // memory at location in I, the tens digit at location I+1, and the ones digit at location I+2
            pChip8->memory[pChip8->I] = pChip8->V[(pChip8->opcode & 0x0F00) >> 8] / 100;
            pChip8->memory[(pChip8->I) + 1] = (pChip8->V[(pChip8->opcode & 0x0F00) >> 8] / 10) % 10;
            pChip8->memory[(pChip8->I) + 2] = (pChip8->V[(pChip8->opcode & 0x0F00) >> 8] % 100) % 10;
            pChip8->pc += 2;
            break;
        case 0x0055: // FX55 - Stores from V0 to VX (including VX) in memory, starting at address I
                     // The offset from I is increased by 1 for each value written, but I itself is left unmodified
            {
                int offset = (pChip8->opcode & 0x0F00) >> 8;
                for (int i = 0; i <= offset; i++)
                    pChip8->memory[pChip8->I + i] = pChip8->V[i];
                pChip8->pc += 2;
            }
            break;
        case 0x0065:; // Fills from V0 to VX (including VX) with values from memory, starting at address I
                     // The offset from I is increased by 1 for each value read, but I itself is left unmodified
            {
                int offset = (pChip8->opcode & 0x0F00) >> 8;
                for (int i = 0; i <= offset; i++)
                    pChip8->V[i] = pChip8->memory[pChip8->I + i];
                pChip8->pc += 2;
            }
            break;
        default:
            printf("Unknown Opcode [0xF000]: 0x%x\n", pChip8->opcode);
        }
        break;
    default:
        printf("Unknown Opcode 0x%X\n", pChip8->opcode);
    }

    // Update timers
    if (pChip8->delay_timer > 0)
        pChip8->delay_timer--;
    if (pChip8->sound_timer > 0)
    {
        pChip8->sound_timer--;
    }
}

unsigned char* chip8_get_gfx(CHIP8 hChip8)
{
    Chip8* pChip8 = (Chip8*)hChip8;
    return pChip8->gfx;
}

Boolean chip8_get_draw_flag(CHIP8 hChip8)
{
    Chip8* pChip8 = (Chip8*)hChip8;
    return pChip8->draw_flag;
}

int chip8_get_sound_timer(CHIP8 hChip8)
{
    Chip8* pChip8 = (Chip8*)hChip8;
    return pChip8->sound_timer;
}

void chip8_set_draw_flag(CHIP8 hChip8, Boolean value)
{
    Chip8* pChip8 = (Chip8*)hChip8;
    pChip8->draw_flag = value;
}

void chip8_set_key(CHIP8 hChip8, int key_index, int state)
{
    Chip8* pChip8 = (Chip8*)hChip8;
    pChip8->key[key_index] = state;
}

void chip8_debug(CHIP8 hChip8, Boolean* enabled)
{
    Chip8* pChip8 = (Chip8*)hChip8;

    int input, noc, set;
    char confirm;
    Boolean done = FALSE;
    
    printf("----------------------------------------------------------\n");
    printf("Next Opcode: 0x%x\n", (pChip8->memory[pChip8->pc] << 8 | pChip8->memory[pChip8->pc + 1]) & 0xFFFF);
    printf("I = 0x%x\n", pChip8->I);
    printf("PC = 0x%x\n", pChip8->pc);
    printf("SP = %d\n", pChip8->sp);
    printf("Delay Timer = %d\n", pChip8->delay_timer);
    printf("Sound Timer = %d\n\n", pChip8->sound_timer);

    while (!done)
    {
        printf("1 - Memory, 2 - Registers, 3 - GFX, 4 - Stack, 5 - Keys, 6 - Set Keys, 7 - Continue Operation, 8 - Exit Debug\n");
        printf("Input the number of what you would like to view: ");
        noc = scanf("%d", &input);
        while (noc != 1 | (input != 1 && input != 2 && input != 3 && input != 4 && input != 5 && input != 6 && input != 7 && input != 8))
        {
            printf("Invalid input! ");
            noc = scanf("%d", &input); 
        }
        printf("\n");

        switch (input)
        {
        case 1:
            printf("Memory:\n");
            for (int i = 0; i < MEMORY_SIZE; i++)
            {
                printf("0x%x ", pChip8->memory[i]);
                if (i % 32 == 0 && i != 0)
                    printf("\n");
            }
            printf("\n\n");
            break;
        case 2:
            printf("Registers:\n");
            for (int i = 0; i < CPU_REGISTERS; i++)
            {
                printf("V[%d] = 0x%x\n", i, pChip8->V[i]);
            }
            printf("\n");
            break;
        case 3:
            printf("GFX:\n");
            for (int r = 0; r < SCREEN_HEIGHT; r++)
            {
                for (int c = 0; c < SCREEN_WIDTH; c++)
                {
                    printf("%d ", pChip8->gfx[r * SCREEN_WIDTH + c]);
                }
                printf("\n");
            }
            break;
        case 4:
            printf("Stack:\n");
            for (int i = 0; i < pChip8->sp; i++)
            {
                printf("Stack %d = 0x%x\n", i, pChip8->stack[i]);
            }
            printf("\n");
            break;
        case 5:
            printf("Keys:\n");
            for (int i = 0; i < NUM_OF_KEYS; i++)
            {
                printf("Key %x = %d\n", i, pChip8->key[i]);
            }
            printf("\n");
            break;
        case 6:
            printf("Which key to set? (0 - 15, corresponds with the index of the array that holds the key) ");
            noc = scanf("%d", &set);
            if (set < 16 && set >= 0)
            {
                pChip8->key[set] = 1;
                printf("Key %d set!\n", set);
            }
            else
                printf("Invalid input!\n");
            break;
        case 7:
            done = TRUE;
            break;
        case 8:
            done = TRUE;
            *enabled = FALSE;
            break;
        }

        if (!done)
        {
            printf("Would you like to view anything else or continue execution? (y/n) ");
            scanf("%c", &confirm);
            while (confirm != 'y' && confirm != 'n')
                scanf("%c", &confirm);

            if (confirm == 'n')
                done = TRUE;
        }
    }
    printf("Continuing execution...\n");
}

void chip8_destory(CHIP8* phChip8)
{
    Chip8* pChip8 = (Chip8*)*phChip8;
    free(pChip8->key);
    free(pChip8->stack);
    free(pChip8->gfx);
    free(pChip8->V);
    free(pChip8->memory);
    free(pChip8);
    *phChip8 = NULL;
}