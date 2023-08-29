#ifndef chip8_H
#define CHIP8_H

// System Parameters
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define MEMORY_SIZE 4096
#define CPU_REGISTERS 16
#define STACK_SIZE 16
#define NUM_OF_KEYS 16

typedef void* CHIP8;

// Status Enums
typedef enum status {FAILURE, SUCCESS} Status;
typedef enum boolean {FALSE, TRUE} Boolean;

// Chip8 Opaque Object Functions
CHIP8 chip8_init_default(void);
Status chip8_load_rom(CHIP8 hChip8, FILE* fp);
void chip8_emulate_cycle(CHIP8 hChip8);
unsigned char* chip8_get_gfx(CHIP8 hChip8);
Boolean chip8_get_draw_flag(CHIP8 hChip8);
int chip8_get_sound_timer(CHIP8 hChip8);
void chip8_set_draw_flag(CHIP8 hChip8, Boolean value);
void chip8_set_key(CHIP8 hChip8, int key_index, int state);
void chip8_debug(CHIP8 hChip8, Boolean* enabled);
void chip8_destory(CHIP8* phChip8);

#endif