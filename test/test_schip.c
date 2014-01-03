#include <stdio.h>
#include "test.h"
#include "schip.h"

TEST(schip_new) {
  unsigned int i;
  schip_t * schip = schip_new();

  assert(schip->program_counter == 0x200);
  assert(schip->index_register  == 0);
  assert(schip->stack_pointer   == 0);
  assert(schip->opcode          == 0);

  i = 0;
  while (i < sizeof(schip->keys) / sizeof(schip->keys[0])) {
    assert(schip->keys[i++] == 0);
  }

  i = 0;
  while (i < sizeof(schip->stack) / sizeof(schip->stack[0])) {
    assert(schip->stack[i++] == 0);
  }

  i = 0;
  while (i < sizeof(schip->screen) / sizeof(schip->screen[0])) {
    assert(schip->screen[i++] == 0);
  }

  assert(schip->program_counter == 0x200);

  schip_free(schip);
}

TEST(schip_reset) {
  unsigned int i;
  schip_t * schip = schip_new();

  assert(schip->program_counter == 0x200);
  assert(schip->index_register  == 0);
  assert(schip->stack_pointer   == 0);
  assert(schip->opcode          == 0);

  i = 0;
  while (i < sizeof(schip->keys) / sizeof(schip->keys[0])) {
    assert(schip->keys[i++] == 0);
  }

  i = 0;
  while (i < sizeof(schip->stack) / sizeof(schip->stack[0])) {
    assert(schip->stack[i++] == 0);
  }

  i = 0;
  while (i < sizeof(schip->screen) / sizeof(schip->screen[0])) {
    assert(schip->screen[i++] == 0);
  }

  assert(schip->program_counter == 0x200);

  schip_free(schip);
}

TEST(schip_clear_screen) {
  unsigned int i;
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x00;
  schip->memory[0x201] = 0xE0;

  schip_tick(schip);

  i = 0;
  while (i < sizeof(schip->screen) / sizeof(schip->screen[0])) {
    assert(schip->screen[i++] == 0);
  }

  assert(schip->program_counter == 0x202);

  schip_free(schip);
}

TEST(schip_return) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x00;
  schip->memory[0x201] = 0xEE;

  schip->stack[1] = 0x200;
  schip->stack_pointer = 2;

  schip_tick(schip);

  assert(schip->program_counter == 0x202);
  assert(schip->stack_pointer == 1);

  schip_free(schip);
}

TEST(schip_instruction_jump) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x11;
  schip->memory[0x201] = 0x23;

  schip_tick(schip);

  assert(schip->program_counter == 0x123);

  schip_free(schip);
}

TEST(schip_call) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x21;
  schip->memory[0x201] = 0x23;

  schip_tick(schip);

  assert(schip->stack[schip->stack_pointer - 1] == 0x200);
  assert(schip->program_counter == 0x123);

  schip_free(schip);
}

TEST(schip_skip_next_if_vx_is_kk) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x31;
  schip->memory[0x201] = 0x42;

  schip->memory[0x202] = 0x00;
  schip->memory[0x203] = 0xEE;

  schip->memory[0x204] = 0x31;
  schip->memory[0x205] = 0x43;

  schip->registers[1] = 0x42;

  schip_tick(schip);

  assert(schip->program_counter == 0x204);

  schip_tick(schip);

  assert(schip->program_counter == 0x206);

  schip_free(schip);
}

TEST(schip_skip_next_if_vx_isnt_kk) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x41;
  schip->memory[0x201] = 0x42;

  schip->memory[0x202] = 0x41;
  schip->memory[0x203] = 0x43;

  schip->registers[1] = 0x42;

  schip_tick(schip);

  assert(schip->program_counter == 0x202);

  schip_tick(schip);

  assert(schip->program_counter == 0x206);

  schip_free(schip);
}

TEST(schip_skip_next_if_vx_is_vy) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x51;
  schip->memory[0x201] = 0x20;

  schip->memory[0x202] = 0x41;
  schip->memory[0x203] = 0x43;

  schip->memory[0x204] = 0x51;
  schip->memory[0x205] = 0x30;

  schip->registers[1] = 0x42;
  schip->registers[2] = 0x42;
  schip->registers[3] = 0x41;

  schip_tick(schip);

  assert(schip->program_counter == 0x204);

  schip_tick(schip);

  assert(schip->program_counter == 0x206);

  schip_free(schip);
}

TEST(schip_skip_next_if_vx_isnt_vy) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x91;
  schip->memory[0x201] = 0x20;

  schip->memory[0x202] = 0x41;
  schip->memory[0x203] = 0x43;

  schip->memory[0x204] = 0x91;
  schip->memory[0x205] = 0x30;

  schip->registers[1] = 0x42;
  schip->registers[2] = 0x12;
  schip->registers[3] = 0x42;

  schip_tick(schip);

  assert(schip->program_counter == 0x204);

  schip_tick(schip);

  assert(schip->program_counter == 0x206);

  schip_free(schip);
}

TEST(schip_set_vx_to_kk) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x61;
  schip->memory[0x201] = 0x42;

  schip_tick(schip);

  assert(schip->registers[1] == 0x42);

  schip_free(schip);
}

TEST(schip_set_vx_to_vx_plus_kk) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x61;
  schip->memory[0x201] = 0x42;

  schip->memory[0x202] = 0x71;
  schip->memory[0x203] = 0x42;

  schip_tick(schip);

  assert(schip->registers[1] == 0x42);

  schip_tick(schip);

  assert(schip->registers[1] == (0x42 + 0x42));

  schip_free(schip);
}

TEST(schip_set_vx_to_vy) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x81;
  schip->memory[0x201] = 0x20;

  schip->registers[1] = 0x00;
  schip->registers[2] = 0x42;

  schip_tick(schip);

  assert(schip->registers[1] == 0x42);

  schip_free(schip);
}

TEST(schip_set_vx_to_vx_or_vy) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x81;
  schip->memory[0x201] = 0x21;

  schip->registers[1] = 0x00;
  schip->registers[2] = 0x42;

  schip_tick(schip);

  assert(schip->registers[1] == (0x00 | 0x42));

  schip_free(schip);
}

TEST(schip_set_vx_to_vx_and_vy) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x81;
  schip->memory[0x201] = 0x22;

  schip->registers[1] = 0x00;
  schip->registers[2] = 0x42;

  schip_tick(schip);

  assert(schip->registers[1] == 0x00);

  schip_free(schip);
}

TEST(schip_set_vx_to_vx_xor_vy) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x81;
  schip->memory[0x201] = 0x23;

  schip->registers[1] = 0x11;
  schip->registers[2] = 0x42;

  schip_tick(schip);

  assert(schip->registers[1] == (0x11 ^ 0x42));

  schip_free(schip);
}

TEST(schip_set_vx_to_vx_plus_vy) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x81;
  schip->memory[0x201] = 0x24;

  schip->registers[1] = 0xFF;
  schip->registers[2] = 0xFF;

  schip_tick(schip);

  assert(schip->registers[1] == (unsigned char) (0xFF + 0xFF));
  assert(schip->registers[15] == 1);

  schip_free(schip);
}

TEST(schip_set_vx_to_vx_sub_vy) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x81;
  schip->memory[0x201] = 0x25;

  schip->registers[1] = 0x42;
  schip->registers[2] = 0xFF;

  schip_tick(schip);

  assert(schip->registers[1] == (unsigned char) (0x42 - 0xFF));
  assert(schip->registers[15] == 0);

  schip_free(schip);
}

TEST(schip_set_vx_to_vx_shm_vy) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x81;
  schip->memory[0x201] = 0x26;

  schip->registers[1] = 0x05;

  schip_tick(schip);

  assert(schip->registers[1] == (unsigned char) (0x05 >> 1));
  assert(schip->registers[15] == 1);

  schip_free(schip);
}

TEST(schip_set_vx_to_vy_sub_vx) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x81;
  schip->memory[0x201] = 0x27;

  schip->registers[1] = 0x42;
  schip->registers[2] = 0xFF;

  schip_tick(schip);

  assert(schip->registers[1] == (unsigned char) (0xFF - 0x42));
  assert(schip->registers[15] == 1);

  schip_free(schip);
}

TEST(schip_set_vx_to_vx_shl_vy) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0x81;
  schip->memory[0x201] = 0x2E;

  schip->registers[1] = 0x81;

  schip_tick(schip);

  assert(schip->registers[1] == (unsigned char) (0x81 << 1));
  assert(schip->registers[15] == 1);

  schip_free(schip);
}

TEST(schip_instruction_jump_plus_v0) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0xB1;
  schip->memory[0x201] = 0x23;

  schip->registers[0] = 0x42;

  schip_tick(schip);

  assert(schip->program_counter == (0x123 + 0x42));

  schip_free(schip);
}

TEST(schip_draw_xor_mode) {
  schip_t * schip = schip_new();

  schip->registers[0] = 0;

  schip->memory[0x200] = 0xF0;
  schip->memory[0x201] = 0x29;

  schip->memory[0x202] = 0xD0;
  schip->memory[0x203] = 0x05;

  schip->memory[0x204] = 0x12;
  schip->memory[0x205] = 0x02;

  schip_tick(schip);
  schip_tick(schip);

  assert(schip->screen[0]      == 1);
  assert(schip->registers[0xF] == 0);

  schip_tick(schip);
  schip_tick(schip);

  assert(schip->screen[0]      == 0);
  assert(schip->registers[0xF] == 1);

  schip_free(schip);
}

TEST(schip_get_hex_sprite_location) {
  unsigned int i;
  schip_t * schip = schip_new();

  for (i = 0; i < 16; i++) {
    schip->registers[i] = i;
  }

  for (i = 0; i < 16; i++) {
    schip->memory[0x200 + (i * 2)]     = 0xF0 + i;
    schip->memory[0x200 + (i * 2) + 1] = 0x29;
  }

  for (i = 0; i < 16; i++) {
    schip_tick(schip);

    assert(schip->index_register == i * 5);
  }

  schip_free(schip);
}

TEST(schip_binary_coded_decimal) {
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0xF0;
  schip->memory[0x201] = 0x33;

  schip->registers[0] = 255;

  schip_tick(schip);

  assert(schip->memory[schip->index_register++] == 0x02);
  assert(schip->memory[schip->index_register++] == 0x05);
  assert(schip->memory[schip->index_register]   == 0x05);

  schip_free(schip);
}

TEST(schip_copy_memory_into_registers) {
  unsigned int i;
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0xA2;
  schip->memory[0x201] = 0x04;
  schip->memory[0x202] = 0xFF;
  schip->memory[0x203] = 0x65;

  for (i = 0; i < 16; i++) {
    schip->memory[0x204 + i] = i;
  }

  schip_tick(schip);

  assert(schip->index_register == 0x204);

  schip_tick(schip);

  for (i = 0; i < 16; i++) {
    assert(schip->registers[i] == i);
  }

  assert(schip->index_register == (0x204 + 16));

  schip_free(schip);
}

TEST(schip_copy_registers_into_memory) {
  unsigned int i;
  schip_t * schip = schip_new();

  schip->memory[0x200] = 0xA2;
  schip->memory[0x201] = 0x04;
  schip->memory[0x202] = 0xFF;
  schip->memory[0x203] = 0x55;

  for (i = 0; i < 16; i++) {
    schip->registers[i] = i;
  }

  schip_tick(schip);

  assert(schip->index_register == 0x204);

  schip_tick(schip);

  for (i = 0; i < 16; i++) {
    assert(schip->memory[0x204 + i] == i);
  }

  assert(schip->index_register == (0x204 + 16));

  schip_free(schip);
}

static test_t schip_test_suite[] = {
  UNIT(schip_new),
  UNIT(schip_clear_screen),
  UNIT(schip_return),
  UNIT(schip_instruction_jump),
  UNIT(schip_call),
  UNIT(schip_skip_next_if_vx_is_kk),
  UNIT(schip_skip_next_if_vx_isnt_kk),
  UNIT(schip_skip_next_if_vx_is_vy),
  UNIT(schip_skip_next_if_vx_isnt_vy),
  UNIT(schip_set_vx_to_kk),
  UNIT(schip_set_vx_to_vx_plus_kk),
  UNIT(schip_set_vx_to_vy),
  UNIT(schip_set_vx_to_vx_or_vy),
  UNIT(schip_set_vx_to_vx_and_vy),
  UNIT(schip_set_vx_to_vx_xor_vy),
  UNIT(schip_set_vx_to_vx_plus_vy),
  UNIT(schip_set_vx_to_vx_sub_vy),
  UNIT(schip_set_vx_to_vx_shm_vy),
  UNIT(schip_set_vx_to_vy_sub_vx),
  UNIT(schip_set_vx_to_vx_shl_vy),
  UNIT(schip_instruction_jump_plus_v0),
  UNIT(schip_draw_xor_mode),
  UNIT(schip_get_hex_sprite_location),
  UNIT(schip_binary_coded_decimal),
  UNIT(schip_copy_memory_into_registers),
  UNIT(schip_copy_registers_into_memory)
};
