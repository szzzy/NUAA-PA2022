#include "common.h"
#include "fs.h"
#define DEFAULT_ENTRY ((void *)0x8048000)
#include "memory.h"
extern void ramdisk_read(const void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();

  //读取文件位置
  int index=fs_open(filename,0,0);
  //读取长度
  size_t length=fs_filesz(index);
  //读取内容
  //fs_read(index,DEFAULT_ENTRY,length);

  void *va;
  void *pa;
  int page_count = length/4096 + 1;//获取页数量
  
  for (int i=0;i<page_count;i++){
    va = DEFAULT_ENTRY + 4096*i;
    pa = new_page();
    //Log("Map va to pa: 0x%08x to 0x%08x",va,pa);
    _map(as,va,pa);
    fs_read(index,pa,4096);
  }
  //关闭文件
  fs_close(index);
  return (uintptr_t)DEFAULT_ENTRY;
}
