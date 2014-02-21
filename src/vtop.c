/*
 Siggraph

 Coverting between virtual and physical addresses

 Zhiqiang Lin

 Copyright   : Copyright 2012 by UTD. all rights reserved. This material may
 be freely copied and distributed subject to inclusion of this
 copyright notice and our World Wide Web URL http://www.utdallas.edu
 */

#include <stdio.h>
#include <string.h>

#define TARGET_PAGE_SIZE (1 << 12)
#define TARGET_PAGE_MASK ~(TARGET_PAGE_SIZE -1)

#define PG_PRESENT_BIT  0
#define PG_RW_BIT   1 
#define PG_USER_BIT 2
#define PG_PWT_BIT  3
#define PG_PCD_BIT  4
#define PG_ACCESSED_BIT 5
#define PG_DIRTY_BIT    6
#define PG_PSE_BIT  7
#define PG_GLOBAL_BIT   8
#define PG_NX_BIT   63

#define PG_PRESENT_MASK  (1 << PG_PRESENT_BIT)                                                                                                               
#define PG_RW_MASK   (1 << PG_RW_BIT)
#define PG_USER_MASK     (1 << PG_USER_BIT)
#define PG_PWT_MASK  (1 << PG_PWT_BIT)
#define PG_PCD_MASK  (1 << PG_PCD_BIT)
#define PG_ACCESSED_MASK (1 << PG_ACCESSED_BIT)
#define PG_DIRTY_MASK    (1 << PG_DIRTY_BIT)
#define PG_PSE_MASK  (1 << PG_PSE_BIT)
#define PG_GLOBAL_MASK   (1 << PG_GLOBAL_BIT)
#define PG_NX_MASK   (1LL << PG_NX_BIT)

#define PG_ERROR_W_BIT     1

#define PG_ERROR_P_MASK    0x01
#define PG_ERROR_W_MASK    (1 << PG_ERROR_W_BIT)
#define PG_ERROR_U_MASK    0x04
#define PG_ERROR_RSVD_MASK 0x08
#define PG_ERROR_I_D_MASK  0x10

unsigned vtop(char *mem, unsigned size, unsigned pgd, unsigned addr) {
	//get pde
	unsigned pde_addr = (pgd & ~0xfff) + ((addr >> 20) & ~3);
	if (pde_addr > size) {
		printf("ERROR 1 addr %x pde %x\n", addr, pde_addr);
		return -1;
	}
	unsigned pde = *(unsigned*) ((unsigned) mem + pde_addr);

	//get pte
	unsigned pte;
	unsigned pte_addr;
	unsigned page_size;

	if (!(pde & PG_PRESENT_MASK)) {
		return -1;
	}

	if (pde & PG_PSE_MASK)
	{
		pte = pde & ~0x003ff000;
		page_size = 4096 * 1024;
	} else {
		pte_addr = ((pde & ~0xfff) + ((addr >> 10) & 0xffc));
		if (pte_addr > size) {
			// printf("ERROR 2 addr %x pte %x\n", addr, pte_addr);
			return -1;
		}
		pte = *(unsigned*) ((unsigned) mem + pte_addr);
		if (!(pte & PG_PRESENT_MASK)) {
			return -1;
		}
		page_size = 4096;
	}

	unsigned page_offset = addr & (page_size - 1);
	unsigned paddr = (pte & TARGET_PAGE_MASK) + page_offset;

	if (paddr >= size)
		return -1;
//	printf("paddr %x pte %x offset %x psize %x\n", paddr, pte,
//			page_offset, page_size);

	return paddr;
}

/*return  physical address, and page properties(readonly/writable,user/system,global or not, page size 4M/4k)
 *if rw is 0, then the page is read only; if rw is 2, then the page is writable*/
unsigned vtopPageProperty(char *mem, unsigned size, unsigned pgd, unsigned addr,
		int *rw, int *us, int *g, int *ps) {
	//get pde
	unsigned pde_addr = (pgd & ~0xfff) + ((addr >> 20) & ~3);
	if (pde_addr > size) {
		printf("ERROR 1 addr %x pde %x\n", addr, pde_addr);
		return -1;
	}
	unsigned pde = *(unsigned*) ((unsigned) mem + pde_addr);

	//get pte
	unsigned pte;
	unsigned pte_addr;
	unsigned page_size;

	if (!(pde & PG_PRESENT_MASK)) {
		return -1;
	}

	if (pde & PG_PSE_MASK)
	{
		pte = pde & ~0x003ff000;
		page_size = 4096 * 1024;
	} else {
		pte_addr = ((pde & ~0xfff) + ((addr >> 10) & 0xffc));
		if (pte_addr > size) {
			// printf("ERROR 2 addr %x pte %x\n", addr, pte_addr);
			return -1;
		}
		pte = *(unsigned*) ((unsigned) mem + pte_addr);
		if (!(pte & PG_PRESENT_MASK)) {
			return -1;
		}
		page_size = 4096;
	}

//	printf("vaddr:%x, pde:%x, pte:%x size:%x\n",addr,pde,pte, page_size);
	unsigned page_offset = addr & (page_size - 1);
	unsigned paddr = (pte & TARGET_PAGE_MASK) + page_offset;
	*rw = pte & PG_RW_MASK;
	*us = pte & PG_USER_MASK;
	*g = pte & PG_GLOBAL_MASK;

	if(page_size==0x400000)
		*ps = 1;
	else
		*ps = 0;


//	printf("paddr %x pte %x offset %x psize %x,R or W:%x, us:%d,global:%d\n",
//			paddr, pte, page_offset, page_size, *rw, *us, *g);

	return paddr;
}
