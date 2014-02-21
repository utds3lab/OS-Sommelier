/*
 ============================================================================
 Name        : main.c
 Author      : Yufei Gu
 Version     :
 Copyright   : Copyright 2012 by UTD. All rights reserved. This material may
 be freely copied and distributed subject to inclusion of this
 copyright notice and our World Wide Web URL http://www.utdallas.edu
 Description : Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "memload.h"
#include "memory.h"


extern long long timeval_diff(struct timeval *difference,
                              struct timeval *end_time,
                              struct timeval *start_time);
unsigned getPgd(char *mem, int mem_size);

void usage(char *prog)
{
    printf("%s snapshot\n", prog);
}

Mem *initMem(char *snapshot)
{
   //load memory file
    struct timeval earlier;
    struct timeval later;
    if (gettimeofday(&earlier, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }
    char *mem;
    unsigned long mem_size;
    mem = mem_load(snapshot, &mem_size);
    if (mem == NULL)
        return NULL;
    else
        printf("mem '%s' load success! size is %ld\n", snapshot, mem_size);

    if (gettimeofday(&later, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }
    int loadTime = timeval_diff(NULL, &later, &earlier) / 1000;
    printf("Load mem time cost is %d milliseconds\n", loadTime);
    FILE *out_data;
    out_data = fopen("loadMemTime", "a+");
    fprintf(out_data, "%d\t%s\n", loadTime, snapshot);
    fclose(out_data);

    //get pgd
    unsigned pgd = getPgd(mem, mem_size);
    printf("pgd is 0x%x\n", pgd);

    //construct a struct Mem
    Mem *mem1 = (Mem *) malloc(sizeof(Mem));
    mem1->mem = mem;
    mem1->mem_size = mem_size;
    mem1->pgd = pgd;

    return mem1;
}

//get signature from a memory snapshot
void genSignature(char *snapshot1)
{
    xed2_init();
    Mem *mem1 = initMem(snapshot1);

    //traverse memory
    if (mem1 != NULL) {
        findReadOnlyPages(mem1);
    }
    //free memory
    free_mem(mem1);
}

/*determine a os version by memory snapshot*/
void determineOsVer(char *snapshot)
{
    xed2_init();
    Mem *mem1 = initMem(snapshot);

    //traverse memory
    if (mem1 != NULL) {
        determineOsVersion(mem1);
    }
    //free memory
    free_mem(mem1);
}

unsigned out_pc;
FILE *out_code;
struct timeval programStart;
char *snapshot;
char *out_sig;

int main(int argc, char *argv[])
{
    if (argc < 1) {
        usage(argv[0]);
        return 1;
    }
    //load memory
    char *argument = argv[1];
    snapshot = argv[2];

    //  out_sig = argv[3];
    char * out_code_file = argv[3];
    sscanf(out_code_file, "%x", &out_pc);
    out_code = fopen(out_code_file, "w");

    int isScan = 0;
    int isGenerator = 0;
    //s is scanning and telling the os version
    extern char *strchr(const char *s, int c);
    char *pch = strchr(argument, 's');
    if (pch != NULL)
        isScan = 1;
    //g is generate signature
    pch = strchr(argument, 'g');
    if (pch != NULL)
        isGenerator = 1;

    struct timeval later;
    if (gettimeofday(&programStart, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }

//    if (isScan == 1)
//        determineOsVer(snapshot);
//    else if (isGenerator == 1)
    genSignature(snapshot);

    if (gettimeofday(&later, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }
    printf("Total time cost is %lld milliseconds\n",
           timeval_diff(NULL, &later, &programStart) / 1000);
    return EXIT_SUCCESS;
}
