#ifndef __REG_H__
#define __REG_H__

#include "common.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

/* TODO✔️: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

typedef struct {
  /* General purpose registers */
  union {
    union {
      union {
        /* Just a break down of the 32-bit gpr*/
        uint32_t _32;
        uint16_t _16;
        uint8_t _8[2];
      };
      uint32_t reg_val;
    } gpr[8];
    struct {
      /* Do NOT change the order of the GPRs' definitions. */
      /* In NEMU, rtlreg_t is exactly uint32_t. This makes RTL instructions
      * in PA2 able to directly access these registers.
      */
      rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    };
  };

  vaddr_t eip;

  union {
    /* The only five flags we care about are CF, ZF, SF, IF and OF */
    struct {
      uint32_t CF : 1;
      uint32_t    : 1;
      uint32_t    : 4;
      uint32_t ZF : 1;
      uint32_t SF : 1;
      uint32_t    : 1;
      uint32_t IF : 1;
      uint32_t    : 1;
      uint32_t OF : 1;
      uint32_t    : 20;
    };
    uint32_t val;
  } eflags;

  struct {
    uint32_t i_len;
    uint32_t i_base;
  } idtr;

  uint32_t cs;
  
} CPU_state;

extern CPU_state cpu;

static inline int check_reg_index(int index) {
  assert(index >= 0 && index < 8);
  return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];

void reg_display();

static inline const char* reg_name(int index, int width) {
  assert(index >= 0 && index < 8);
  switch (width) {
    case 4: return regsl[index];
    case 1: return regsb[index];
    case 2: return regsw[index];
    default: assert(0);
  }
}

#endif
