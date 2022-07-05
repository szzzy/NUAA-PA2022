#include "cpu/exec.h"

make_EHelper(test) {
  //TODO();
  rtl_and(&id_dest->val, &id_dest->val, &id_src->val);
  
  rtl_set_CF(&tzero);//CF=0
  rtl_set_OF(&tzero);//OF=0
  rtl_update_ZFSF(&id_dest->val, id_dest->width);//更新ZF、SF
  
  print_asm_template2(test);
}

make_EHelper(and) {
  //TODO();
  rtl_and(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);
  
  rtl_set_CF(&tzero);//CF=0
  rtl_set_OF(&tzero);//OF=0
  rtl_update_ZFSF(&id_dest->val, id_dest->width);//更新ZF、SF
  
  print_asm_template2(and);
}

make_EHelper(xor) {
  //TODO();
  //rtl_xor(&id_dest->val,&id_src->val,&id_src2->val);//异或
  rtl_xor(&id_dest->val,&id_dest->val, &id_src->val);//异或
  operand_write(id_dest,&id_dest->val);//赋值
  
  rtl_set_CF(&tzero);//CF=0
  rtl_set_OF(&tzero);//OF=0
  rtl_update_ZFSF(&id_dest->val,id_dest->width);//更新ZF、SF
  
  print_asm_template2(xor);
}

make_EHelper(or) {
  //TODO();

  rtl_or(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);

  rtl_set_CF(&tzero);//CF<--0
  rtl_set_OF(&tzero);//OF<--0

  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  
  print_asm_template2(or);
}

make_EHelper(sar) {
  //TODO();
  // unnecessary to update CF and OF in NEMU

  rtl_sar(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
   
  print_asm_template2(sar);
}

make_EHelper(shl) {
  //TODO();
  // unnecessary to update CF and OF in NEMU

  rtl_shl(&id_dest->val, &id_dest->val, &id_src->val);//逻辑左移
  operand_write(id_dest, &id_dest->val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
   
  print_asm_template2(shl);
}

make_EHelper(shr) {
  //TODO();
  // unnecessary to update CF and OF in NEMU

  rtl_shr(&id_dest->val, &id_dest->val, &id_src->val);//逻辑右移
  operand_write(id_dest, &id_dest->val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
   
  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  //TODO();

  rtl_not(&id_dest->val);//取反
  operand_write(id_dest, &id_dest->val);
   
  print_asm_template1(not);
}

make_EHelper(rol) {
  //TODO();
  for (t0=0;t0<id_src->val;t0++) {
    //先获取最高位，即即将移出的位
    rtl_shri(&t1, &id_dest->val, id_dest->width*8-1);
    //左移一位
    rtl_shli(&t2, &id_dest->val, 1);
    //把移出的位补在最低位
    id_dest->val=t1+t2;
  }
  //设置CF
  rtl_set_CF(&t1);
  //记录到目的操作数
  operand_write(id_dest, &id_dest->val);
  
  print_asm_template2(rol);
}
