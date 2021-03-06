#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "schip.h"

schip_timer_t * schip_timer_new(void) {
  schip_timer_t * self = (schip_timer_t *) malloc(sizeof(schip_timer_t));

  if (self) {
    self->value  = 0;
    self->active = true;
    self->done   = false;

    pthread_create(&self->thread, NULL, (void *) schip_timer_loop, self);
  }

  return self;
}

void schip_timer_reset(schip_timer_t * self) {
  self->value = 0;
}

void * schip_timer_loop(schip_timer_t * self) {
  while (self->active) {
    if (self->value) self->value--;
    usleep(16667);
  }
  self->done = true;
  return NULL;
}

void schip_timer_free(schip_timer_t * self) {
  self->active = false;
  while (!self->done);
  pthread_detach(self->thread);
  free(self);
}

schip_t * schip_new(void) {
  schip_t * self = (schip_t *) malloc(sizeof(schip_t));

  if (self) {
    self->program_counter = 0x200;
    self->index_register  = 0;
    self->stack_pointer   = 0;
    self->opcode          = 0;
    self->sound_timer     = schip_timer_new();
    self->delay_timer     = schip_timer_new();

    memcpy(self->memory, schip_hex_font, sizeof(schip_hex_font));
    memset(self->registers, 0, sizeof(self->registers));
    memset(self->screen, 0, sizeof(self->screen));
    memset(self->stack, 0, sizeof(self->stack));
    memset(self->keys, 0, sizeof(self->keys));

    srand(time(NULL));
  }

  return self;
}

void schip_tick(schip_t * self) {
  self->opcode = self->memory[PC(self)] << 8 | self->memory[PC(self) + 1];

  switch (self->opcode & 0xF000) {
  case 0x0000:
    switch (self->opcode & 0x00FF) {
    case 0x00E0:
      memset(self->screen, 0, sizeof(self->screen));
      PC(self) += 2;
      break;
    case 0x00EE:
      PC(self) = self->stack[--SP(self)] + 2;
      break;
    default:
      schip_no_such_opcode(self);
    }
    break;
  case 0x1000:
    PC(self) = self->opcode & 0x0FFF;
    break;
  case 0x2000:
    self->stack[SP(self)++] = PC(self);
    PC(self) = self->opcode & 0x0FFF;
    break;
  case 0x3000:
    (V(self)[BYTE3(self->opcode)] == (self->opcode & 0x00FF))
      ? (PC(self) += 4)
      : (PC(self) += 2);
    break;
  case 0x4000:
    (V(self)[BYTE3(self->opcode)] == (self->opcode & 0x00FF))
      ? (PC(self) += 2)
      : (PC(self) += 4);
    break;
  case 0x5000:
    (V(self)[BYTE3(self->opcode)] == V(self)[BYTE2(self->opcode)])
      ? (PC(self) += 4)
      : (PC(self) += 2);
    break;
  case 0x6000:
    V(self)[BYTE3(self->opcode)] = self->opcode & 0x00FF;
    PC(self) += 2;
    break;
  case 0x7000:
    V(self)[BYTE3(self->opcode)] += self->opcode & 0x00FF;
    PC(self) += 2;
    break;
  case 0x8000:
    switch (self->opcode & 0x000F) {
    case 0x0000:
      V(self)[BYTE3(self->opcode)] = V(self)[BYTE2(self->opcode)];
      PC(self) += 2;
      break;
    case 0x0001:
      V(self)[BYTE3(self->opcode)] |= V(self)[BYTE2(self->opcode)];
      PC(self) += 2;
      break;
    case 0x0002:
      V(self)[BYTE3(self->opcode)] &= V(self)[BYTE2(self->opcode)];
      PC(self) += 2;
      break;
    case 0x0003:
      V(self)[BYTE3(self->opcode)] ^= V(self)[BYTE2(self->opcode)];
      PC(self) += 2;
      break;
    case 0x0004:
      VF(self) = V(self)[BYTE3(self->opcode)] + V(self)[BYTE2(self->opcode)] > 0xFF;
      V(self)[BYTE3(self->opcode)] += V(self)[BYTE2(self->opcode)];
      PC(self) += 2;
      break;
    case 0x0005:
      VF(self) = V(self)[BYTE2(self->opcode)] <= V(self)[BYTE3(self->opcode)];
      V(self)[BYTE3(self->opcode)] -= V(self)[BYTE2(self->opcode)];
      PC(self) += 2;
      break;
    case 0x0006:
      VF(self) = V(self)[BYTE3(self->opcode)] & 1;
      V(self)[BYTE3(self->opcode)] >>= 1;
      PC(self) += 2;
      break;
    case 0x0007:
      VF(self) = V(self)[BYTE3(self->opcode)] <= V(self)[BYTE2(self->opcode)];
      V(self)[BYTE3(self->opcode)] = V(self)[BYTE2(self->opcode)] - V(self)[BYTE3(self->opcode)];
      PC(self) += 2;
      break;
    case 0x000E:
      VF(self) = V(self)[BYTE3(self->opcode)] >> 7;
      V(self)[BYTE3(self->opcode)] <<= 1;
      PC(self) += 2;
      break;
    default:
      schip_no_such_opcode(self);
    }
    break;
  case 0x9000:
    (V(self)[BYTE3(self->opcode)] == V(self)[BYTE2(self->opcode)])
      ? (PC(self) += 2)
      : (PC(self) += 4);
    break;
  case 0xA000:
    I(self) = self->opcode & 0x0FFF;
    PC(self) += 2;
    break;
  case 0xB000:
    PC(self) = V0(self) + (self->opcode & 0x0FFF);
    break;
  case 0xC000:
    V(self)[BYTE3(self->opcode)] = rand() & (self->opcode & 0x00FF);
    PC(self) += 2;
    break;
  case 0xD000:
    {
      uint8_t  i, j, row, pixel;
      uint16_t location;
      uint8_t  x_coord = V(self)[BYTE3(self->opcode)];
      uint8_t  y_coord = V(self)[BYTE2(self->opcode)];
      uint8_t  height  = BYTE1(self->opcode);
      uint8_t  width   = 8;

      VF(self) = 0;
      for (i = 0; i < height; i++) {
        row = self->memory[I(self) + i];
        for (j = 0; j < width; j++) {
          if (!(pixel = row & (0x80 >> j))) continue;
          if ((location = 32 * (x_coord + j) + (y_coord + i)) > (64 * 32)) continue;
          if (self->screen[location]) VF(self) = 1;
          self->screen[location] ^= 1;
        }
      }
    }
    PC(self) += 2;
    break;
  case 0xE000:
    switch (self->opcode & 0xF0FF) {
    case 0xE09E:
      self->keys[V(self)[BYTE3(self->opcode)]]
        ? (PC(self) += 4)
        : (PC(self) += 2);
      break;
    case 0xE0A1:
      self->keys[V(self)[BYTE3(self->opcode)]]
        ? (PC(self) += 2)
        : (PC(self) += 4);
      break;
    default:
      schip_no_such_opcode(self);
    }
    break;
  case 0xF000:
    switch (self->opcode & 0xF0FF) {
    case 0xF007:
      V(self)[BYTE3(self->opcode)] = DT(self);
      PC(self) += 2;
      break;
    case 0xF00A:
      {
        uint8_t i;

        for (i = 0; i < 16; i++) {
          if (self->keys[i]) {
            V(self)[BYTE3(self->opcode)] = i;
            PC(self) += 2;
            break;
          }
        }
      }
      break;
    case 0xF015:
      DT(self) = V(self)[BYTE3(self->opcode)];
      PC(self) += 2;
      break;
    case 0xF018:
      ST(self) = V(self)[BYTE3(self->opcode)];
      PC(self) += 2;
      break;
    case 0xF01E:
      VF(self) = (I(self) + V(self)[BYTE3(self->opcode)]) > 0xFFF;
      I(self) += V(self)[BYTE3(self->opcode)];
      PC(self) += 2;
      break;
    case 0xF029:
      I(self) = V(self)[BYTE3(self->opcode)] * 5;
      PC(self) += 2;
      break;
    case 0xF033:
      self->memory[I(self)]     = (V(self)[BYTE3(self->opcode)] / 100);
      self->memory[I(self) + 1] = (V(self)[BYTE3(self->opcode)] / 10) % 10;
      self->memory[I(self) + 2] = (V(self)[BYTE3(self->opcode)] % 100) % 10;
      PC(self) += 2;
      break;
    case 0xF055:
      {
        uint8_t i;

        for (i = 0; i <= BYTE3(self->opcode); i++) {
          self->memory[I(self)++] = V(self)[i];
        }
      }
      PC(self) += 2;
      break;
    case 0xF065:
      {
        uint8_t i;

        for (i = 0; i <= BYTE3(self->opcode); i++) {
          V(self)[i] = self->memory[I(self)++];
        }
      }
      PC(self) += 2;
      break;
    default:
      schip_no_such_opcode(self);
    }
    break;
  default:
    schip_no_such_opcode(self);
  }
}

void schip_reset(schip_t * self) {
  self->program_counter    = 0x200;
  self->index_register     = 0;
  self->stack_pointer      = 0;
  self->opcode             = 0;

  schip_timer_reset(self->sound_timer);
  schip_timer_reset(self->delay_timer);

  memset(self->registers, 0, sizeof(self->registers));
  memset(self->screen, 0, sizeof(self->screen));
  memset(self->stack, 0, sizeof(self->stack));
}

void schip_no_such_opcode(schip_t * self) {
  fprintf(stderr, "Unknown opcode: 0x%X\n", self->opcode);
}

void schip_free(schip_t * self) {
  if (self->delay_timer) schip_timer_free(self->delay_timer);
  if (self->sound_timer) schip_timer_free(self->sound_timer);
  free(self);
}
