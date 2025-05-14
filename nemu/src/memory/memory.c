#include "nemu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  if (is_mmio(addr) != -1) {
    return mmio_read(addr, len, is_mmio(addr));
  }
  else {
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  if (is_mmio(addr) != -1) {
    mmio_write(addr, len, data, is_mmio(addr));
  }
  else {
    memcpy(guest_to_host(addr), &data, len);
  }
}

#define PTE_ADDR(pte) ((uint32_t)(pte) & ~0xfff)
#define PDX(va)       (((uint32_t)(va) >> 22) & 0x3ff)
#define PTX(va)       (((uint32_t)(va) >> 12) & 0x3ff)
#define OFF(va)       ((uint32_t)(va) & 0xfff)

paddr_t page_translate(vaddr_t vaddr, bool is_write) {
  uint32_t DIR = vaddr >> 22;
  uint32_t PAGE = vaddr >> 12 & 0x000003FF;
  uint32_t OFFSET = vaddr & 0x00000FFF;
  paddr_t PhysicalAddr = vaddr;


  if (cpu.cr0.val & 0x80000000) {
    uint32_t PageTable = paddr_read(cpu.cr3.val + 4 * DIR, 4) & 0xFFFFF000;
    if(!(paddr_read(cpu.cr3.val + 4 * DIR, 4) & 0x00000001)) {
      Log("FATAL: Virtual Address is 0x%08X", vaddr);
      Log("FATAL: eip = 0x%08X at PD", cpu.eip);
    }
    // assert(paddr_read(cpu.cr3.val + 4 * DIR, 4) & 0x00000001); // Present
    paddr_write(cpu.cr3.val + 4 * DIR, 4, (paddr_read(cpu.cr3.val + 4 * DIR, 4) | 0x00000020)); // Set accessed
    uint32_t PageTableEntry = paddr_read(PageTable + 4 * PAGE, 4);
    if(!(PageTableEntry & 0x00000001)) {
      Log("FATAL: Virtual Address is 0x%08X", vaddr);
      Log("FATAL: eip = 0x%08X at PT", cpu.eip);
    }
    // assert(PageTableEntry & 0x00000001); // Present
    paddr_write(PageTable + 4 * PAGE, 4, (paddr_read(PageTable + 4 * PAGE, 4) | 0x00000020)); // Set accessed
    if (is_write) 
      paddr_write(PageTable + 4 * PAGE, 4, (paddr_read(PageTable + 4 * PAGE, 4) | 0x00000040)); // Set dirty
    PhysicalAddr = (paddr_read(PageTable + 4 * PAGE, 4) & 0xFFFFF000) + OFFSET;

    //Log("PhysicalAddr = 0x%08X", PhysicalAddr);
  }
  return PhysicalAddr;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if ((addr & PAGE_MASK) != ((addr + len - 1) & PAGE_MASK)) {
    // Concatenate the data if it cross the page boundary
    uint32_t data = 0x0;
    for (int i = 0; i < len; i++) {
      paddr_t paddr = page_translate(addr + i, false);
      data += (paddr_read(paddr, 1)) << 8 * i;
    }
    return data;
  } else {
    paddr_t paddr = page_translate(addr, false);
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if ((addr & PAGE_MASK) != ((addr + len - 1) & PAGE_MASK)) {
    for (int i = 0; i < len; i++) {
      paddr_t paddr = page_translate(addr + i, true);
      paddr_write(paddr, 1, data >> 8 * i);
    } 
  } else {
    paddr_t paddr = page_translate(addr, true);
    paddr_write(paddr, len, data);
  }
}
