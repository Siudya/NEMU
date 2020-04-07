#include "cc.h"

// dest <- and result
static inline void and_internal(DecodeExecState *s) {
  rtl_and(s, s0, ddest, dsrc1);
#ifdef LAZY_CC
  rtl_set_lazycc(s, s0, NULL, NULL, LAZYCC_LOGIC, id_dest->width);
#else
  rtl_update_ZFSF(s, s0, id_dest->width);
  rtl_mv(s, &cpu.CF, rz);
  rtl_mv(s, &cpu.OF, rz);
#endif
}

static inline make_EHelper(test) {
  and_internal(s);
  print_asm_template2(test);
}

static inline make_EHelper(and) {
  and_internal(s);
  operand_write(s, id_dest, s0);
  print_asm_template2(and);
}

static inline make_EHelper(xor) {
  rtl_xor(s, s0, ddest, dsrc1);
#ifdef LAZY_CC
  rtl_set_lazycc(s, s0, NULL, NULL, LAZYCC_LOGIC, id_dest->width);
#else
  rtl_update_ZFSF(s, s0, id_dest->width);
  rtl_mv(s, &cpu.CF, rz);
  rtl_mv(s, &cpu.OF, rz);
#endif
  operand_write(s, id_dest, s0);
  print_asm_template2(xor);
}

static inline make_EHelper(or) {
  rtl_or(s, s0, ddest, dsrc1);
#ifdef LAZY_CC
  rtl_set_lazycc(s, s0, NULL, NULL, LAZYCC_LOGIC, id_dest->width);
#else
  rtl_update_ZFSF(s, s0, id_dest->width);
  rtl_mv(s, &cpu.CF, rz);
  rtl_mv(s, &cpu.OF, rz);
#endif
  operand_write(s, id_dest, s0);
  print_asm_template2(or);
}

static inline make_EHelper(sar) {
  rtl_sext(s, s0, ddest, id_dest->width);
  rtl_sar(s, s0, s0, dsrc1);
  operand_write(s, id_dest, s0);
#ifndef LAZY_CC
  rtl_update_ZFSF(s, s0, id_dest->width);
  // unnecessary to update CF and OF in NEMU
#endif
  //difftest_skip_eflags(EFLAGS_MASK_CF | EFLAGS_MASK_OF);
  print_asm_template2(sar);
}

static inline make_EHelper(shl) {
  rtl_shl(s, s0, ddest, dsrc1);
  operand_write(s, id_dest, s0);
#ifndef LAZY_CC
  rtl_update_ZFSF(s, s0, id_dest->width);
  // unnecessary to update CF and OF in NEMU
#endif
  //difftest_skip_eflags(EFLAGS_MASK_CF | EFLAGS_MASK_OF | EFLAGS_MASK_ZF);
  print_asm_template2(shl);
}

static inline make_EHelper(shr) {
  rtl_shr(s, s0, ddest, dsrc1);
  operand_write(s, id_dest, s0);
#ifndef LAZY_CC
  rtl_update_ZFSF(s, s0, id_dest->width);
  // unnecessary to update CF and OF in NEMU
#endif
  //difftest_skip_eflags(EFLAGS_MASK_CF | EFLAGS_MASK_OF);
  print_asm_template2(shr);
}

static inline make_EHelper(rol) {
  rtl_shl(s, s0, ddest, dsrc1);
  rtl_li(s, s1, id_dest->width * 8);
  rtl_sub(s, s1, s1, dsrc1);
  rtl_shr(s, s1, ddest, s1);
  rtl_or(s, s1, s0, s1);

  operand_write(s, id_dest, s1);
  // unnecessary to update eflags in NEMU
  //difftest_skip_eflags(EFLAGS_MASK_ALL);
  print_asm_template2(rol);
}

static inline make_EHelper(ror) {
  rtl_shr(s, s0, ddest, dsrc1);
  rtl_li(s, s1, id_dest->width * 8);
  rtl_sub(s, s1, s1, dsrc1);
  rtl_shl(s, s1, ddest, s1);
  rtl_or(s, s1, s0, s1);

  operand_write(s, id_dest, s1);
  // unnecessary to update eflags in NEMU
  //difftest_skip_eflags(EFLAGS_MASK_ALL);
  print_asm_template2(ror);
}

static inline make_EHelper(setcc) {
  uint32_t cc = s->opcode & 0xf;

#ifdef LAZY_CC
  rtl_lazy_setcc(s, s0, cc);
#else
  rtl_setcc(s, s0, cc);
#endif
  operand_write(s, id_dest, s0);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

static inline make_EHelper(not) {
  rtl_not(s, s0, ddest);
  operand_write(s, id_dest, s0);

  print_asm_template1(not);
}

static inline make_EHelper(shld) {
  rtl_andi(s, dsrc1, dsrc1, 31);
  rtl_shl(s, s0, ddest, dsrc1);

  rtl_li(s, s1, 31);
  rtl_sub(s, s1, s1, dsrc1);
  // shift twice to deal with dsrc1 = 0
  rtl_shr(s, s1, dsrc2, s1);
  rtl_shri(s, s1, s1, 1);

  rtl_or(s, s0, s0, s1);

  operand_write(s, id_dest, s0);
#ifndef LAZY_CC
  rtl_update_ZFSF(s, s0, id_dest->width);
  // unnecessary to update CF and OF in NEMU
#endif
  print_asm_template3(shld);
}

static inline make_EHelper(shrd) {
  rtl_andi(s, dsrc1, dsrc1, 31);
  rtl_shr(s, s0, ddest, dsrc1);

  rtl_li(s, s1, 31);
  rtl_sub(s, s1, s1, dsrc1);
  // shift twice to deal with dsrc1 = 0
  rtl_shl(s, s1, dsrc2, s1);
  rtl_shli(s, s1, s1, 1);

  rtl_or(s, s0, s0, s1);

  operand_write(s, id_dest, s0);
#ifndef LAZY_CC
  rtl_update_ZFSF(s, s0, id_dest->width);
  // unnecessary to update CF and OF in NEMU
#endif
  print_asm_template3(shrd);
}