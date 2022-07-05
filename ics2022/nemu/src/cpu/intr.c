#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  //获取门描述符
  vaddr_t gate_addr=cpu.idtr.base+8*NO;
  //P位校验
  if (cpu.idtr.limit<0){
    assert(0);
  }
  //将eflags、cs、返回地址压栈
  rtl_push(&cpu.eflags.val);
  rtl_push(&cpu.cs);
  rtl_push(&ret_addr);
  //组合中断处理程序入口点
  uint32_t high,low;
  low=vaddr_read(gate_addr,4)&0xffff;
  high=vaddr_read(gate_addr+4,4)&0xffff0000;
  //设置eip跳转
  decoding.jmp_eip=high|low;
  decoding.is_jmp=true;
}

void dev_raise_intr() {
}
