/*
 *  Copyright : Copyright 2012 by UTD. All rights reserved. This material may
 be freely copied and distributed subject to inclusion of this
 copyright notice and our World Wide Web URL http://www.utdallas.edu
 */

#include <xed-interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

//#define DEBUG_MODHASH
xed_decoded_inst_t xedd_g;

/* Variables to keep disassembler state */
xed_state_t dstate;
xed_decoded_inst_t xedd;



range ranges[0x10000];
unsigned range_index = 0;

node *offsets[1 << 20];
unsigned ptov[1 << 20];
unsigned page_end[1 << 20];
int is_dism[1 << 20];

int newstart = 1;

void offset_insert(int index, unsigned offset)
{
    node *newnode = (node *) malloc(sizeof(node));
    newnode->offset = offset;
    newnode->next = NULL;

    if (offsets[index] == NULL)
        offsets[index] = newnode;
    else {
        node *temp = offsets[index];
        node *prev = NULL;
        while (temp != NULL && temp->offset < offset) {
            prev = temp;
            temp = temp->next;
        }
        if (prev == NULL) {
            offsets[index] = newnode;
            newnode->next = temp;
        } else {
            prev->next = newnode;
            newnode->next = temp;
        }
    }
}

/* XED2 initialization */
void xed2_init()
{
    unsigned i;

    xed_decoded_inst_set_mode(&xedd_g, XED_MACHINE_MODE_LEGACY_32,
                              XED_ADDRESS_WIDTH_32b);
    xed_tables_init();
    xed_state_zero(&dstate);

    xed_state_init(&dstate, XED_MACHINE_MODE_LEGACY_32,
                   XED_ADDRESS_WIDTH_32b, XED_ADDRESS_WIDTH_32b);

    for (i = 0; i < (1 << 20); i++) {
        offsets[i] = NULL;
        page_end[0] = 0;
    }

}

void set_page(unsigned vaddr, int pageIndex, unsigned pageSize)
{
    ptov[pageIndex] = vaddr & (~(pageSize - 1));
}

int operand_is_imm(const xed_operand_enum_t op_name)
{
    switch (op_name) {
        /* Immediate */
    case XED_OPERAND_IMM0:
        return 1;
        /* Special immediate only used in ENTER instruction */
    case XED_OPERAND_IMM1:
        return 1;
    default:
        return 0;
    }

}

int operand_is_relbr(const xed_operand_enum_t op_name, uint32_t * branch)
{
    switch (op_name) {
        /* Jumps */
    case XED_OPERAND_PTR:      // pointer (always in conjunction with a IMM0)
    case XED_OPERAND_RELBR:{
            // branch displacements

            xed_uint_t disp =
                xed_decoded_inst_get_branch_displacement(&xedd_g);
            *branch = disp;
            return 1;

        }
    default:
        return 0;
    }

}

extern FILE *out_code;
int canDisasn(Mem * mem, unsigned char *data, unsigned offset,
              unsigned len, unsigned vaddr)
{
    unsigned size;
    int success = -1;

    while (offset < len && success != 0) {

        xed_decoded_inst_zero_set_mode(&xedd_g, &dstate);
        xed_error_enum_t xed_error = xed_decode(&xedd_g,
                                                STATIC_CAST(const
                                                            xed_uint8_t *,
                                                            data + offset),
                                                15);
        if (xed_error == XED_ERROR_NONE) {
            const xed_inst_t *xi = xed_decoded_inst_inst(&xedd_g);
            size = xed_decoded_inst_get_length(&xedd_g);
            xed_iclass_enum_t opcode =
                xed_decoded_inst_get_iclass(&xedd_g);

            if (opcode == XED_ICLASS_RET_NEAR || opcode == XED_ICLASS_NOP)
                success = 0;
            if (opcode == XED_ICLASS_JMP && size >= 5)
                success = 0;
            if (size == 3 && data[offset] == 0x8d
                && data[offset + 1] == 0x49 && data[offset + 2] == 0x00)
                success = 0;
//                      if(opcode == XED_ICLASS_JMP && size ==5&&((offset+size)&0x3)==0){
//                              unsigned pc = *(unsigned *)(data+offset+size);
//                              if(pc>=0x80000000)
//                                      if(vtop(mem->mem, mem->mem_size, mem->pgd, pc)!=-1)
//                                              success=1;
//                      }
        } else
            break;

        offset = offset + size;
    }

    return success;
}

int disasn_replace(Mem * mem, unsigned char *data, unsigned *poffset,
                   unsigned len, int is_print, unsigned vaddr,
                   unsigned calledpage[], int is_replace)
{
    unsigned size;
    unsigned offset = *poffset;
    int success = 0;
    char str[128];

    while (offset < len && !success) {

        xed_decoded_inst_zero_set_mode(&xedd_g, &dstate);
        xed_error_enum_t xed_error = xed_decode(&xedd_g,
                                                STATIC_CAST(const
                                                            xed_uint8_t *,
                                                            data + offset),
                                                15);
        if (xed_error == XED_ERROR_NONE) {
            const xed_inst_t *xi = xed_decoded_inst_inst(&xedd_g);
            size = xed_decoded_inst_get_length(&xedd_g);
            xed_iclass_enum_t opcode =
                xed_decoded_inst_get_iclass(&xedd_g);

            if (opcode == XED_ICLASS_RET_NEAR
                || opcode == XED_ICLASS_RET_FAR
                || opcode == XED_ICLASS_INT3)
                success = 1;
            if (opcode == XED_ICLASS_JMP)
                success = 1;
            if (size == 3 && data[offset] == 0x8d
                && data[offset + 1] == 0x49 && data[offset + 2] == 0x00)
                success = 1;
//                      if(opcode == XED_ICLASS_JMP && size ==5&&((offset+size)&0x3)==0){
//                              unsigned pc = *(unsigned *)(data+offset+size);
//                              if(pc>=0x80000000)
//                                      if(vtop(mem->mem, mem->mem_size, mem->pgd, pc)!=-1)
//                                              success=1;
//                      }
        } else
            break;

        offset = offset + size;
    }

    if (success == 0)
        return -1;

    if (newstart == 1) {
        newstart = 0;
        ranges[++range_index].start = vaddr;
        ranges[range_index].end = vaddr + 4095;
        ranges[range_index].disasBytes = 0;
    }

    offset = *poffset;
    while (offset < len) {

        xed_decoded_inst_zero_set_mode(&xedd_g, &dstate);
        xed_error_enum_t xed_error = xed_decode(&xedd_g,
                                                STATIC_CAST(const
                                                            xed_uint8_t *,
                                                            data + offset),
                                                15);
        if (xed_error == XED_ERROR_NONE) {
            const xed_inst_t *xi = xed_decoded_inst_inst(&xedd_g);
            xed_decoded_inst_dump_intel_format(&xedd_g, str, sizeof(str),
                                               0);
            size = xed_decoded_inst_get_length(&xedd_g);
            xed_iclass_enum_t opcode =
                xed_decoded_inst_get_iclass(&xedd_g);

            if (is_replace)
                is_dism[offset / 4096] = 1;

            if (is_print) {
                unsigned int i;
                fprintf(out_code, "0x%08x: ", offset + vaddr);
                for (i = 0; i < size; i++)
                    fprintf(out_code, "%02x ", data[offset + i]);
                for (; i < 8; i++)
                    fprintf(out_code, "   ");
                const xed_operand_t *op0 = xed_inst_operand(xi, 0);
                xed_operand_enum_t op_name0 = xed_operand_name(op0);
                uint32_t branch;
                if (operand_is_relbr(op_name0, &branch)) {
                    char opname[32];
                    strcpy(opname, xed_iclass_enum_t2str(opcode));
                    for (i = 0; i < strlen(opname); i++)
                        opname[i] = opname[i] - ('A' - 'a');
                    opname[4] = '\0';
                    uint32_t pc = vaddr + offset + size + branch;
                    sprintf(str, "%s 0x%08x", opname, pc);

                }
                fprintf(out_code, "%s\n", str);
            }

            if (!is_replace && opcode == XED_ICLASS_CALL_NEAR
                && data[offset] == 0xe8) {
                unsigned pc =
                    vaddr + offset + 5 + *(unsigned *) (data + offset + 1);
                unsigned paddr =
                    vtop(mem->mem, mem->mem_size, mem->pgd, pc);
                if (paddr != -1) {
                    //                      printf("call (%x, %x  %x)\n", vaddr+offset, pc,*(unsigned *) (data + offset + 1));
                    if (ranges[range_index].end < pc) {
                        //                      printf("dis call (%x, %x  %x)\n", vaddr+offset, pc,*(unsigned *) (data + offset + 1));
                        int pageSize = 4096;
                        int pageIndex = paddr / pageSize;
                        char *page =
                            (char *) ((unsigned) mem->mem +
                                      pageIndex * pageSize);
//                                              unsigned  i=pc&0xfff;
//                                              if ((i<=4093&&page[i] == 0x55 && page[i + 1] == 0x8b && page[i + 2] == 0xec)
//                                                                              || (i<=4093&&page[i] == 0x55 && page[i + 1] == 0x89
//                                                                                              && page[i + 2] == 0xe5)) {
                        if (canDisasn
                            (mem, page, paddr - pageIndex * pageSize,
                             pageSize, pc) == 0) {
//                                              if(calledpage[pageIndex]==1){
                            ranges[range_index].end =
                                pc & 0xfffff000 + pageSize - 1;
                            ranges[range_index].len =
                                ranges[range_index].end -
                                ranges[range_index].start + 1;
                        }
                    }
                }
            }

            if (size == 3 && data[offset] == 0x8d
                && data[offset + 1] == 0x49 && data[offset + 2] == 0x00) {
                *poffset = offset + 3;
                return 0;
            }
            /*
               if (size >= 2) {
               unsigned i;
               for (i = 1; i < size; i++)
               data[offset + i] = '\0';
               }
             */
//                      if (opcode == XED_ICLASS_JMP && size >= 5) {
//                              *poffset = offset + 5;
//                              return 0;
//                      }
            if (is_replace && size >= 5) {
                data[offset + size - 1] = '\0';
                data[offset + size - 2] = '\0';
                data[offset + size - 3] = '\0';
                data[offset + size - 4] = '\0';
                if (size >= 9) {
                    data[offset + size - 5] = '\0';
                    data[offset + size - 6] = '\0';
                    data[offset + size - 7] = '\0';
                    data[offset + size - 8] = '\0';
                }
            }
            if (opcode == XED_ICLASS_INT3 || opcode == XED_ICLASS_RET_NEAR
                || opcode == XED_ICLASS_RET_FAR
                || (opcode == XED_ICLASS_JMP)) {
                *poffset = offset + size;
                if (is_print)
                    fprintf(out_code, "\n\n");

                return 0;
            }

        }

        offset = offset + size;
    }

    return 0;
}

unsigned total = 0;
unsigned page_init2(Mem * mem, char *page, int pageIndex, int pageSize,
                    unsigned vaddr, int is_replace);
//pageSize can be 0x400000 or 0x1000
void code_preprocess(Mem * mem, unsigned char *page, unsigned size,
                     unsigned pageSize, range * c, int *dsmPage)
{
    unsigned int i = 0, offset;
    unsigned char *inst;
    unsigned start, end;
    unsigned vaddr;

    start = c->start;
    end = c->end;

    printf("preprocess %x %x %x\n", start, end, size);
    for (; i < size - 5; i++) {
        if (page[i] == 0xe8 && (page[i + 4] == 0xff || page[i + 4] == 0)) {
            //get the call pc
            vaddr = start + i;
            unsigned pc1 = vaddr + 5;
            unsigned pc = pc1 + *(unsigned *) (page + i + 1);


            if (pc < start || pc > end)
                continue;

            if ((vaddr & (~(pageSize - 1))) != (pc & (~(pageSize - 1))))
                continue;

            unsigned phaddr = vtop(mem->mem, mem->mem_size, mem->pgd, pc);
            if (phaddr == -1 || phaddr >= mem->mem_size)
                continue;

            inst = (unsigned char *) ((unsigned) mem->mem + phaddr);

            //check wether is "push ebp; mov esp, ebp" instruction  //linux
            //windows: mov %edi,%edi
            if ((*inst == 0x8b && *(inst + 1) == 0xff
                 && *(inst + 2) == 0x55 && *(inst + 3) == 0x8b
                 && *(inst + 4) == 0xec)
                || (*inst == 0x55 && *(inst + 1) == 0x89
                    && *(inst + 2) == 0xe5)
                || (*inst == 0x55 && *(inst + 1) == 0x8b
                    && *(inst + 2) == 0xec)
                ) {
                offset_insert(1 << 20 - 1, pc - start);
                //                              printf("new pc is %x\n", pc);

            }
        }
    }
    c->disasBytes = page_init2(mem, page, 1 << 20 - 1, size, start, 1);

    printf("dissam rate is %x %f\n", c->disasBytes,
           ((float) c->disasBytes) / size);

    memcpy(dsmPage, is_dism, size / 0x1000 * sizeof(int));
}

/*
unsigned total = 0;
int code_preprocess(Mem *mem, unsigned vaddr, unsigned pageSize, cluster c) {
	int i, offset;
	unsigned char * inst;

	unsigned paddr = vtop(mem->mem, mem->mem_size, mem->pgd, vaddr);
	if (paddr == -1)
		return -1;

	int pageIndex1 = paddr / pageSize;
	int pageIndex2;
	unsigned char * page = (char *) ((unsigned) mem->mem + pageIndex1 * pageSize);

	for (i = 0; i < pageSize - 5; i++) {
		if (page[i] == 0xe8 && (page[i + 4] == 0xff || page[i + 4] == 0)) {
			//get the call pc
			unsigned pc2 = (vaddr & (~(pageSize - 1)));
			unsigned pc1 = pc2 + i + 5;
			unsigned pc = pc1 + *(unsigned *) (page + i + 1);

			if(pc<c.start || pc >c.end)
				continue;
			unsigned phaddr = vtop(mem->mem, mem->mem_size, mem->pgd, pc);
			if (phaddr == -1 || phaddr >= mem->mem_size)
				continue;

			inst = (unsigned char *) ((unsigned) mem->mem + phaddr);

			//check wether is "push ebp; mov esp, ebp" instruction  //linux
			//windows: mov esi, esi  or mov %edi,%edi
			if (
					(*inst == 0x8b && *(inst + 1) == 0xff && *(inst + 2) == 0x55 && *(inst + 3) == 0x8b
					&& *(inst + 4) == 0xec)
					|| (*inst == 0x55 && *(inst + 1) == 0x89 && *(inst + 2) == 0xe5)
				    || 	(*inst == 0x55 && *(inst + 1) == 0x8b && *(inst + 2) == 0xec)
				)
				{
					
				return 0;
//				//	sameSharp[pageIndex1]=1;
//				if (canDisasn(mem, inst - (phaddr & 0xfff), phaddr & 0xfff, pageSize, pc) == 0) {
//					pageIndex2 = phaddr / pageSize;
//					sameSharp[pageIndex2] = 1;
//				}
//				//	printf("call (%x, %x)\n", vaddr+i, pc);
			}
		}
	}
	return -1;
}
*/

/* if this page is kernel code,than return 0, otherwise return -1
*/
int find_kernel(Mem * mem, unsigned vaddr, unsigned pageSize)
{
    int i, offset;
    unsigned char *inst;
    int iret, sysexit, lgdt = 0;

    unsigned paddr = vtop(mem->mem, mem->mem_size, mem->pgd, vaddr);
    if (paddr == -1)
        return -1;

    int pageIndex = paddr / pageSize;
    unsigned char *page =
        (char *) ((unsigned) mem->mem + pageIndex * pageSize);

    iret = 0;
    sysexit = 0;

    int sys_instr_cnt = 0;
    int cr3 =0;
    int clts =0;
    int rdmsr =0;
    int rdtsc =0;
    int wrmsr =0;
    int wbinvd =0;
    for (i = 0; i < pageSize - 6; i++) {

        //mov EAX,CR3;
        //mov CR3,EAX;

        if (page[i] == 0x0f && page[i + 1] == 0x20 && page[i + 2] == 0xd8
            && page[i + 3] == 0x0f && page[i + 4] == 0x22
            && page[i + 5] == 0xd8) {
            puts("cr3");
            //sys_instr_cnt++;
	    cr3 =1;
            return 0;
        }
        //hlt,  problem: too short
        //if(page[i]== 0xf4)
        //  return 0;


        //mwait, problem: too short, windows have no mwait

        //if(page[i]== 0x0f && page[i+1]== 0x01 && page[i+2]==0xc9){
        //  puts("mwait");
        //  return 0;
        //}

        //clts
        if (page[i] == 0x0f && page[i + 1] == 0x06 && page[i+2]!= 0x00) {
            //puts("clts");
	    //            sys_instr_cnt++;
	    clts =1;
            //            return 0;
        }
        //wbinvd
        if (page[i] == 0x0f && page[i + 1] == 0x09 && page[i+2]!= 0x00) {
            //puts("wbinvd");
	    //            sys_instr_cnt++;
	    wbinvd =1;
            //            return 0;
        }
        //rdmsr
        if (page[i] == 0x0f && page[i + 1] == 0x32 && page[i+2]!= 0x00) {
            //puts("rdmsr");
	    //            sys_instr_cnt++;
	    rdmsr =1;
            //            return 0;            return 0;
        }
        //wrmsr
        if (page[i] == 0x0f && page[i + 1] == 0x30 && page[i+2]!= 0x00) {
            //puts("wrmsr");
	    //            sys_instr_cnt++;
	    wrmsr =1;
            //            return 0;

        }
        //rdtsc
        if (page[i] == 0x0f && page[i + 1] == 0x31 && page[i+2]!= 0x00) {
            //puts("rdtsc");
            //sys_instr_cnt++;
	    rdtsc =1;
            //            return 0;
        }
	//if(cr3 == 1)
	//return 0;
	//	if(rdtsc + wrmsr + rdmsr + clts+ wbinvd +cr3 >= 2)
	//return 0;

    }
    //    if (sys_instr_cnt > 1)
    //  return 0;

    return -1;

    //problem, freebsd not work
    int gdt = 0, idt = 0;
    for (i = 0; i < pageSize - 7; i++) {
        //LGDTL and lidtl

        if (page[i] == 0x0f && page[i + 1] == 0x01 && page[i + 2] == 0x15) {
            gdt = 1;
            //    puts("gdt");
        }
        if (page[i] == 0x0f && page[i + 1] == 0x01 && page[i + 2] == 0x1d) {
            idt = 1;
            // puts("idt");
        }
        //if (gdt ==1 ){
        if (gdt == 1 && idt == 1) {
            puts("LGDTL and LIDTL");
            return 0;
        }
    }
    return -1;

    for (i = 0; i < pageSize - 1; i++)
        if (page[i] == 0xcf) {
            iret = 1;
            break;
        }
    if (iret != 1)
        return -1;

//      for( i = 0 ;i <pageSize-4; i++)
//              if(page[i]==0x83&&page[i+1]==0xc4&&page[i+3]==0xcf){
//                      return 0;
//              }
//
//      return -1;
    for (i = 0; i < pageSize - 3; i++)
        if (page[i] == 0xfb && page[i + 1] == 0x0f && page[i + 2] == 0x35) {
            return 0;
        }

    return -1;
    for (i = 0; i < pageSize - 5; i++)
        if (page[i] == 0xea) {
            unsigned pc = *(unsigned *) (page + i + 1);
            unsigned phaddr = vtop(mem->mem, mem->mem_size, mem->pgd, pc);
            if (phaddr != -1)
                return 0;
        }

    return -1;
    for (i = 0; i < pageSize - 3; i++)
        if (page[i] == 0x0f && page[i + 1] == 0x01 && page[i + 2] == 0x15) {
            lgdt = 1;
            break;
//                                      return 0;
        }
    if (lgdt == 0)
        return -1;
    for (i = 0; i < pageSize - 3; i++)
//              if(page[i]==0xfb&&page[i+1]==0x0f&&page[i+2]==0x35){
        if (page[i] == 0x0f && page[i + 1] == 0x01 && page[i + 2] == 0x1d) {
            return 0;
        }

    return -1;

}

//int code_preprocess(Mem *mem, unsigned vaddr, unsigned pageSize,
//              int sameSharp[], unsigned virtualAddrs[]) {
//      int i, offset;
//      unsigned char * inst;
//      int res = -1;
//
//      unsigned paddr = vtop(mem->mem, mem->mem_size, mem->pgd, vaddr);
//      if (paddr == -1)
//              return -1;
//
//      int pageIndex1 = paddr / pageSize;
//      int pageIndex2;
//      unsigned char * page = (char *) ((unsigned) mem->mem
//                      + pageIndex1 * pageSize);
//
//      for (i = 0; i < pageSize - 5; i++) {
//              if ((page[i] == 0x8b && page[i + 1] == 0xff && page[i + 2] == 0x55
//                              && page[i + 3] == 0x8b && page[i + 4] == 0xec)
//                              || (page[i] == 0x55 && page[i + 1] == 0x89
//                                              && page[i + 2] == 0xe5)) {
//
//                      res = 0;
//                      set_page(vaddr, pageIndex1, pageSize);
//                      sameSharp[pageIndex1] = 1;
//                      virtualAddrs[pageIndex1] = vaddr;
//                      offset_insert(pageIndex1, i);
//              }
//      }
//
//      return res;
//
//}

//determine if the page is code page by opcode "mov..."
int is_code_page(Mem * mem, unsigned vaddr, unsigned pageSize,
                 unsigned virtualAddrs[])
{
    int res = -1;

    unsigned paddr = vtop(mem->mem, mem->mem_size, mem->pgd, vaddr);
    if (paddr == -1)
        return -1;

    int pageIndex = paddr / pageSize;
    unsigned char *page =
        (char *) ((unsigned) mem->mem + pageIndex * pageSize);

    int i;
    for (i = 0; i < pageSize - 5; i++) {
        //      if ((page[i] == 0x8b && page[i + 1] == 0xff && page[i + 2] == 0x55
        if ((page[i] == 0x55 && page[i + 1] == 0x8b && page[i + 2] == 0xec)
            || (page[i] == 0x55 && page[i + 1] == 0x89
                && page[i + 2] == 0xe5)) {
            res = 0;
            set_page(vaddr, pageIndex, pageSize);
            virtualAddrs[pageIndex] = vaddr;
            offset_insert(pageIndex, i);
        }
    }

    return res;
}

unsigned page_init2(Mem * mem, char *page, int pageIndex, int pageSize,
                    unsigned vaddr, int is_replace)
{
    unsigned i;
    unsigned offset;
    unsigned end;
    node *next;
    unsigned disasBytes = 0;


    end = page_end[pageIndex];
    for (next = offsets[pageIndex]; next != NULL; next = next->next) {
        offset = next->offset;
        if (offset > end) {
            for (i = end; i < offset; i++)
                page[i] = '\0';
            end = offset;
            if (disasn_replace
                (mem, page, &end, pageSize, 0, vaddr, NULL,
                 is_replace) == 0) {
#ifdef DEBUG
                printf("start disasn %x  %x %x\n", vaddr + offset,
                       vaddr + end, end - offset);
#endif
                disasBytes += end - offset;
            } else {
                end = offset;
            }
        }
    }

    if (end >= pageSize) {
        page_end[pageIndex + 1] = end - pageSize;
    }
#ifdef DEBUG
    if (sameSharp[pageIndex] == 0)
        printf("can't disassemble %d\n", pageIndex);
#endif
    for (i = end; i < pageSize; i++)
        page[i] = '\0';

    return disasBytes;
}

void page_init(Mem * mem, int pageIndex, int pageSize, int sameSharp[],
               int is_print, unsigned vaddr, unsigned calledpage[],
               int is_replace)
{
    unsigned i;
    char *page = (char *) ((unsigned) mem->mem + pageIndex * pageSize);
    unsigned offset;
    unsigned end;
    node *next;

    sameSharp[pageIndex] = 0;

    end = page_end[pageIndex];
    for (next = offsets[pageIndex]; next != NULL; next = next->next) {
        offset = next->offset;
        if (offset > end) {
            for (i = end; is_replace && i < offset; i++)
                page[i] = '\0';
            end = offset;
            if (disasn_replace
                (mem, page, &end, pageSize, is_print, vaddr, calledpage,
                 is_replace) == 0) {
                ranges[range_index].disasBytes += end - offset;
//                              printf("ranges[range_index].disasBytes:%d, %x\n",ranges[range_index].disasBytes,vaddr);
//                              sameSharp[pageIndex] = 1;
            } else {
                end = offset;
            }
        }
    }
//      if (end > 0)
//              sameSharp[pageIndex] = 1;

    if (end >= pageSize) {
        page_end[pageIndex + 1] = end - pageSize;
    }
#ifdef DEBUG
    if (sameSharp[pageIndex] == 0)
        printf("can't disassemble %d\n", pageIndex);
#endif
    for (i = end; is_replace && i < pageSize; i++)
        page[i] = '\0';
}

void code_init(Mem * mem, unsigned vaddr, unsigned pageSize,
               int sameSharp[], unsigned virtualAddrs[], int is_print,
               unsigned calledpage[], unsigned *codePageNo)
{

    if (newstart == 0 && vaddr > ranges[range_index].end) {
        //printf("end is %x %x\n", vaddr, ranges[range_index].end);
        newstart = 1;
    }

    if (is_code_page(mem, vaddr, pageSize, virtualAddrs) == 0) {
        unsigned paddr = vtop(mem->mem, mem->mem_size, mem->pgd, vaddr);
        int pageIndex = paddr / pageSize;
        page_init(mem, pageIndex, pageSize, sameSharp, is_print, vaddr,
                  calledpage, 0);
        (*codePageNo)++;
    }
}
