#include "cpu/exec/template-start.h"

#define instr jmp
static void do_execute(){
   cpu.eip+=op_src->val;
   print_asm(str(instr)" %x", cpu.eip+1+DATA_BYTE);

}

make_instr_helper(si)
#if DATA_BYTE==4
make_helper(jmp_rm_l){
     int length=decode_rm_l(eip+1);
     cpu.eip=op_src->val - (length +1);
     print_asm(str(instr) " *%s", op_src->str);
     return length+1;
}

#endif
#include "cpu/exec/template-end.h"
