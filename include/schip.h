#ifndef _SCHIP_H
#define _SCHIP_H

#include <stdint.h>
#include <pthread.h>

#define BYTE3(blob) ((blob & 0x0F00) >> 8)
#define BYTE2(blob) ((blob & 0x00F0) >> 4)
#define BYTE1(blob) ((blob & 0x000F))

#define PC(schip) schip->program_counter
#define V(schip)  schip->registers
#define V0(schip) schip->registers[0]
#define VF(schip) schip->registers[0xF]
#define I(schip)  schip->index_register
#define DT(schip) schip->delay_timer->value
#define ST(schip) schip->sound_timer->value
#define SP(schip) schip->stack_pointer

#define SCHIP_MEMORY_IN_BYTES 4096
#define SCHIP_SCREEN_SIZE 64 * 32
#define SCHIP_STACK_LEVELS 16
#define SCHIP_REGISTERS 16
#define SCHIP_KEYS 16

typedef struct schip_timer {
  uint8_t   value;
  int       active;
  int       done;
  pthread_t thread;
} schip_timer_t;

schip_timer_t * schip_timer_new(void);
void schip_timer_reset(schip_timer_t *);
void * schip_timer_loop(schip_timer_t *);
void schip_timer_free(schip_timer_t *);

typedef struct schip {
  schip_timer_t * delay_timer;
  schip_timer_t * sound_timer;
  uint16_t opcode;
  uint8_t  memory[SCHIP_MEMORY_IN_BYTES];
  uint8_t  registers[SCHIP_REGISTERS];
  uint16_t index_register;
  uint16_t program_counter;
  uint8_t  screen[SCHIP_SCREEN_SIZE];
  uint16_t stack[SCHIP_STACK_LEVELS];
  uint16_t stack_pointer;
  uint8_t  keys[SCHIP_KEYS];
} schip_t;

static const uint8_t schip_hex_font[] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
  0x20, 0x60, 0x20, 0x20, 0xF0, /* 1 */
  0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
  0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
  0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
  0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
  0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
  0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
  0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
  0xF0, 0x90, 0xF0, 0x10, 0x10, /* 9 */
  0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
  0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
  0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
  0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
  0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
  0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

schip_t * schip_new(void);
void schip_tick(schip_t *);
void schip_reset(schip_t *);
void schip_no_such_opcode(schip_t *);
void schip_free(schip_t *);

#endif
