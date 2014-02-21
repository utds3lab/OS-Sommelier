/*
    memload.c

    Memory image loading module

    Yufei Gu
    Copyright : Copyright 2012 by UTD. All rights reserved. This material may
    be freely copied and distributed subject to inclusion of this
    copyright notice and our World Wide Web URL http://www.utdallas.edu
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

void mem_free(char *mem)
{
    if (mem)
        free(mem);
}

char * mem_load(char *name, unsigned long *isize)
{
    struct stat fstat;
    int ret;
    char *mem = NULL;
    unsigned long size;

    if (stat(name, &fstat) != 0)
    {
        printf("No snapshot : %s\n", name);
        exit(1);
    }

    size = fstat.st_size;
    //printf("snapshot [%s] size %d/%x\n", name, size, size);

    mem = malloc(size);
    if (mem == NULL)
        return NULL;

    FILE *f = fopen(name, "r");
    ret = fread(mem, 1, size, f);
    fclose(f);
    *isize = size;

    if (ret == size)
    {
        //printf("Snapshot is successfully loaded\n");
        //printf("%d bytes read %x\n", ret, mem);
    }
    else
    {
        free(mem);
        return NULL;
    }

    return mem;

}
