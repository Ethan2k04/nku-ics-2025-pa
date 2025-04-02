#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

make_EHelper(add);
make_EHelper(sub);
make_EHelper(xor);
make_EHelper(inc);
make_EHelper(dec);

make_EHelper(push);
make_EHelper(pop);
make_EHelper(leave);
make_EHelper(cwtl);
make_EHelper(cltd);

make_EHelper(call);
make_EHelper(ret);
