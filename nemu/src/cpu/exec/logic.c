#include "cpu/exec.h"

make_EHelper(test) {
  // TODO();
  rtl_and(&t0, &id_dest->val, &id_src->val);
  t1 = 0;
  rtl_set_CF(&t1);
  rtl_set_OF(&t1);
  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(test);
}

make_EHelper(and) {
  // TODO();
  rtl_and(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);
  t1 = 0;
  rtl_set_CF(&t1);
  rtl_set_OF(&t1);
  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(and);
}

make_EHelper(xor) {
  // TODO();
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, id_dest->width);

  t1 = 0;
  rtl_set_CF(&t1);
  rtl_set_OF(&t1);

  print_asm_template2(xor);
}

make_EHelper(or) {
  // TODO();
  rtl_or(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);
  t1 = 0;
  rtl_set_CF(&t1);
  rtl_set_OF(&t1);
  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(or);
}

make_EHelper(sar) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_sext(&t1, &id_dest->val, id_dest->width);
  rtl_sar(&t1, &t1, &id_src->val);
  operand_write(id_dest, &t1);

  rtl_update_ZFSF(&t1, id_dest->width);
  
  print_asm_template2(sar);
}

make_EHelper(shl) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);

  rtl_update_ZFSF(&t1, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shr(&t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);

  rtl_update_ZFSF(&t1, id_dest->width);

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  // TODO();
  rtl_not(&id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(not);
}

make_EHelper(rol) {
  rtl_shri(&t2, &id_dest->val, id_dest->width * 8 - id_src->val);
  rtl_shl(&t3, &id_dest->val, &id_src->val);
  rtl_or(&t1, &t2, &t3);

  print_asm_template2(rol);
}
