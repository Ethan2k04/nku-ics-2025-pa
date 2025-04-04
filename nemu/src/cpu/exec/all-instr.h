#include "cpu/exec.h"

/* arith.c */
make_EHelper(add);
make_EHelper(sub);
make_EHelper(inc);
make_EHelper(dec);
make_EHelper(adc);
make_EHelper(sbb);
make_EHelper(mul);
make_EHelper(imul1);
make_EHelper(imul2);
make_EHelper(imul3);
make_EHelper(div);
make_EHelper(idiv);
make_EHelper(cmp);

/* control.c */
make_EHelper(jmp);
make_EHelper(jcc);
make_EHelper(jmp_rm);
make_EHelper(call);
make_EHelper(ret);

/* data-mov.c */
make_EHelper(mov);
make_EHelper(push);
make_EHelper(pop);
make_EHelper(leave);
make_EHelper(cwtl);
make_EHelper(cltd);
make_EHelper(movsx);
make_EHelper(movzx);
make_EHelper(lea);

/* logic.c */
make_EHelper(and);
make_EHelper(or);
make_EHelper(xor);
make_EHelper(setcc);

/* prefix.c */
make_EHelper(operand_size);

/* special.c */
make_EHelper(nop);
make_EHelper(inv);
make_EHelper(nemu_trap);
