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

paddr_t page_translate(vaddr_t addr, bool iswrite) {
  PDE pde, *pgdir;
  PTE pte, *pgtab;
  if (cpu.cr0.protect_enable && cpu.cr0.paging) {
    pgdir = (PDE *)(PTE_ADDR(cpu.cr3.val));
    pde.val = paddr_read((paddr_t)&pgdir[PDX(addr)], 4);
    assert(pde.present);
    pde.accessed = 1;
    
    pgtab = (PTE *)(PTE_ADDR(pde.val));
    pte.val = paddr_read((paddr_t)&pgtab[addr], 4);
    assert(pte.present);
    pte.accessed = 1;
    pte.dirty = iswrite ? 1 : pte.dirty;

    return PTE_ADDR(pte.val) | OFF(addr);
  }
  return addr;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if ((addr & PAGE_MASK) != ((addr + len - 1) & PAGE_MASK)) {
    // Concatenate the data if it cross the page boundary
    uint32_t data = 0x0;
    printf("FUCK I AM OKAY!\n");
    for (int i = 0; i < len; i++) {
      paddr_t paddr = page_translate(addr, false);
      data += (paddr_read(paddr, 1)) << 8 * i;
    }
    return data;
  } else {
    printf("SHIT IT IS SHIT!\n");
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
