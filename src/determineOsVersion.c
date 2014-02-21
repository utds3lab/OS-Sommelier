/*
 * determineOsVersion.c
 *
 *  Created on: Dec 20, 2011
 *      Author: yufei gu
 *  Copyright : Copyright 2012 by UTD. All rights reserved. This material may
 be freely copied and distributed subject to inclusion of this
 copyright notice and our World Wide Web URL http://www.utdallas.edu
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include "memory.h"
#include "mddriver.c"

//#define FINGERPRINT_NO 50

fingerprint fingerprints[FINGERPRINT_NO];
extern unsigned out_pc;

extern range ranges[];
extern unsigned range_index;
extern int newstart;
cluster clusters[1 << 16];

//convert char to hex
void initMd5Array(char *md5input, unsigned char md5[16])
{
    int i;
    for (i = 0; i < 16; i++) {
        unsigned int u;
        sscanf(md5input, "%2x", &u);
        md5input += 2;
        md5[i] = (unsigned char) u;
    }

}


void loadSignature(char *filename, fingerprint * fingerprint)
{
    //read file
    struct stat fstat;
    if (stat(filename, &fstat) != 0) {
        printf("No signature file : %s\n", filename);
        exit(1);
    }

    FILE *file = fopen(filename, "r");

    if (file != NULL) {
        //get ride of "../md5/"
        strcpy((*fingerprint).osVersion, filename + 7);

        char line[100];
        while (fgets(line, sizeof line, file) != NULL) {        /* read a line */

            //get rid of empty line
            if ((int) line[0] == 10)
                continue;

            if (strlen(line) >= 43 && strlen(line) < 48 && line[41] == ' '
                && line[32] == ' ') {

                //get md5
                char md5[32];
                int k;
                for (k = 0; k < 32; k++)
                    md5[k] = line[k];

                //get index
                int md5_index = atoi(line + 42);
//                              printf("md5 is %s,index is %d\n", md5, md5_index);
                initMd5Array(md5, (*fingerprint).md5[md5_index]);
            }
        }
        fclose(file);
//              printf("Signature loaded with %s\n", (*fingerprint).osVersion);
    } else {
        perror(filename); 
        return;
    }
//      int i ;
//      for(i=0;i<2000;i++){
//              if(isMd5ArrayEmpty((*fingerprint).md5[i])==0){
//                      printf("Not empty index is %d\n", i );
//              }
//      }

}

//empty,return 0; not empty return -1
int isMd5ArrayEmpty(unsigned char md5[])
{
    int k;
    for (k = 0; k < 16; k++) {
        if ((unsigned char) md5[k] != 0) {
            return -1;
        }
    }
    return 0;
}

//initiate fingerprint db, return the number of os version
int initDb()
{
    int fp_index = 0;
    DIR *d;
    struct dirent *dir;
    char *path = "../md5/";
    d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") == 0
                || strcmp(dir->d_name, "..") == 0)
                continue;
            char filename[4096];
            strcpy(filename, path);
            strcat(filename, dir->d_name);
            //printf("filename is %s\n", filename);
            loadSignature(filename, &fingerprints[fp_index++]);
            //printf("load success %s\n", filename);
        }

        closedir(d);
    }

    printf("Signature load success! Number is %d\n", fp_index);
    return fp_index;
}


//is match success,
//only one match, return 1
//at least two match, return 0
//Gap, return 2;
//match failed, return -1
int matchByIndex(int osNumber, unsigned char md5digest[16], int offset,
                 int availableOs[], int matchCounts[])
{
    //search hash table
    int i;
    int matchCount = 0;
    int matchIndex = -1;
    int gapNo = 0;
    int availableOsCand[FINGERPRINT_NO];
    for (i = 0; i < FINGERPRINT_NO; i++) {
        availableOsCand[i] = availableOs[i];
    }

    for (i = 0; i < osNumber; i++) {
        //if not available, continue;
        if (availableOs[i] == 0)
            continue;

        //gap in md5, vaddr is not continuous
        if (isMd5ArrayEmpty(fingerprints[i].md5[offset]) == 0) {
//                      printf("%d is gap\n", i);
            gapNo++;
            continue;
        } else if (memcmp(md5digest, fingerprints[i].md5[offset], 16) == 0) {
            matchIndex = i;
            printf("Match %s\n", fingerprints[matchIndex].osVersion);
            matchCounts[i]++;
            matchCount++;
        } else {
            //kick out a os signature
            availableOsCand[i] = 0;
        }
    }

    //if at least one match, update outside availableOs array
    if (matchCount > 0) {
        for (i = 0; i < FINGERPRINT_NO; i++) {
            availableOs[i] = availableOsCand[i];
        }
    }
    //only one match
    if (matchCount == 1) {
        printf("Only match is %s\n", fingerprints[matchIndex].osVersion);
        return 1;
    }
    //at least one match
    if (matchCount > 0) {
        return 0;
    } else if (gapNo > 0) {
        return 2;
    }

    return -1;
}

//is match success
int match(int osNumber, unsigned char md5digest[16], int *md5Index,
          int availableOs[])
{
    //search hash table
    int i;
    int matchCount = 0;
    int matchIndex = -1;
    int gapNo = 0;
    int availableOsCand[FINGERPRINT_NO];
    for (i = 0; i < FINGERPRINT_NO; i++) {
        availableOsCand[i] = availableOs[i];
    }
    printf("osNumber is %d,md5index is %d\n", osNumber, *md5Index);
    for (i = 0; i < osNumber; i++) {
        if (availableOs[i] == 0)
            continue;

        //gap in md5, vaddr is not continuous
        if (isMd5ArrayEmpty(fingerprints[i].md5[*md5Index]) == 0) {
            printf("%d is gap\n", i);
            gapNo++;
            continue;
        } else if (memcmp(md5digest, fingerprints[i].md5[*md5Index], 16) ==
                   0) {
            matchIndex = i;
            matchCount++;
        } else {
            //kick out a os signature
            availableOsCand[i] = 0;
        }
    }

    //if at least one match, update outside availableOs array
    if (matchCount > 0) {
        for (i = 0; i < FINGERPRINT_NO; i++) {
            availableOs[i] = availableOsCand[i];
        }
    }
    //only one match
    if (matchCount == 1) {
        printf("OS is %s\n", fingerprints[matchIndex].osVersion);
        return 1;
    }
    //at least one match
    if (matchCount > 0) {
//              printf("OS is %s\n", fingerprints[matchIndex].osVersion);
        return 0;
    } else if (gapNo > 0) {
        return 2;
    }

    return -1;
}

/* determine version of OS by mem
 * 1.To get signature of multiply versions of os, which is the md5 of kernel code
 * 2.Compared by a decision tree.
 * 3.Done!*/
void determineOsVersion(Mem * mem)
{
    int i;
    int pageSize = 4 * 1024;    //4k
    int totalPageNumber = mem->mem_size / (4 * 1024);   //assume that every page has 4k

    unsigned codePageNo = 0;
    //record when two page have different page index and the same sharp
    int calledPages[totalPageNumber];
    int dsmPages[totalPageNumber];
    //record virtual address
    unsigned virtualAddrs[totalPageNumber];
    for (i = 0; i < totalPageNumber; i++) {
        calledPages[i] = 0;
        dsmPages[i] = 0;
        virtualAddrs[i] = 0;
    }

    //start address
    unsigned startVirtualAddr = 0x80000000;

    //with dissemble or not
    int withDissemble = 1;

    int cr3PageIndex = 0;
    unsigned cr3Pages[20];
    for (i = 0; i < 20; i++) {
        cr3Pages[i] = 0;
    }

    struct timeval earlier;
    struct timeval later;
    if (gettimeofday(&earlier, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }
    //generate step 1 clusters
    clusters[0].end = 0;
    int pre_rw = -1;            //read or write
    int pre_us = -1;            //use or system
    int pre_g = -1;             //global, no move out of TLB
    int pre_ps = -1;            //page size
    unsigned cluster_index = 0;
    newstart = 1;

    unsigned vAddr = startVirtualAddr;
    for (; vAddr > 0x80000000 - 1; vAddr += 0x1000) {
        //printf("startvirtual %x:\n",startVirtualAddr);

        int rw = 0;             //read or write
        int us = 0;             //use or system
        int g = 0;              //global, no move out of TLB
        int ps = 0;             //page size 4M or 4k
        unsigned pAddr =
            vtopPageProperty(mem->mem, mem->mem_size, mem->pgd, vAddr, &rw,
                             &us, &g, &ps);

        //if PHYSICAL ADDRESS is not VALID, then start a new cluster
        if (pAddr < 0 || pAddr > mem->mem_size || us != 0 || g != 256) {
            if (newstart == 0) {
                clusters[cluster_index].end = vAddr - 1;
                //printf("err address end is %x %x\n", vAddr, ranges[range_index].end);
                newstart = 1;
            }
            continue;
        }
        //if any property changes, then start a new cluster
        if (rw != pre_rw || us != pre_us || g != pre_g || ps != pre_ps) {
            if (newstart == 0) {
                clusters[cluster_index].end = vAddr - 1;
                //printf("property change end is %x %x\n", vAddr, ranges[range_index].end);
                newstart = 1;
            }
        }
        //update pre properties
        pre_rw = rw;
        pre_us = us;
        pre_g = g;
        pre_ps = ps;

        //collect pages  with continuous properties;
        if (newstart) {
            clusters[++cluster_index].start = vAddr;
            clusters[cluster_index].end = vAddr + pageSize - 1;
            newstart = 0;
        } else
            clusters[cluster_index].end = vAddr + pageSize - 1;
    }

    if (gettimeofday(&later, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }
    printf("step1, cluster: %d,time cost is %d milliseconds\n",
           cluster_index, timeval_diff(NULL, &later, &earlier) / 1000);

    //step2. kernel code page clusters with cr3
    if (gettimeofday(&earlier, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }

    unsigned startVirtual = startVirtualAddr;
    for (; startVirtual > startVirtualAddr - 1; startVirtual += 0x1000) {
        //      for (; startVirtual < 0x818f0000; startVirtual += 0x1000) {
        unsigned vAddr = startVirtual;

        int rw = 0;             //read or write
        int us = 0;             //use or system
        int g = 0;              //global(no move out of TLB) or not global
        int ps = 0;             //page size
        unsigned pAddr =
            vtopPageProperty(mem->mem, mem->mem_size, mem->pgd, vAddr, &rw,
                             &us, &g, &ps);

        // IS PHYSICAL ADDRESS VALID?
        if (pAddr == -1 || pAddr > mem->mem_size)
            continue;

        //collect pages which are system access, and global pages
        if (us == 0 && g == 256) {
//                      printf("r only page %x\n", vAddr);
            if (find_kernel(mem, vAddr, pageSize) == 0) {
                //record kernel address
                cr3Pages[cr3PageIndex++] = vAddr;
                printf("kernel start at %x\n", vAddr);
            }
        }
    }

    if (gettimeofday(&later, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }
    printf("step2 time cost is %d milliseconds\n",
           timeval_diff(NULL, &later, &earlier) / 1000);

    //step 3. clusters
    if (gettimeofday(&earlier, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }
    int cr3PagesNo = 0;
    ranges[0].end = 0;
    newstart = 1;
    for (i = 1; i <= cluster_index; i++) {
//:w            printf("%x %x\n", clusters[i].start, clusters[i].end);
        if (containKernelAddres(clusters[i], cr3Pages) == -1) {
            continue;
        }
        cr3PagesNo++;
        unsigned vAddr = clusters[i].start;
        //      printf("%x %x\n", clusters[i].start, clusters[i].end);
        newstart = 1;
        for (; vAddr < clusters[i].end; vAddr += 0x1000) {
            unsigned pAddr =
                vtop(mem->mem, mem->mem_size, mem->pgd, vAddr);
            if (vAddr == out_pc)
                code_init(mem, vAddr, pageSize, dsmPages, virtualAddrs, 1,
                          calledPages, &codePageNo);
            else
                code_init(mem, vAddr, pageSize, dsmPages, virtualAddrs, 0,
                          calledPages, &codePageNo);
        }
        ranges[range_index].end = clusters[i].end;
    }
    if (gettimeofday(&later, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }
    printf("step2, cluster: %d\n", cr3PagesNo);

    printf("step3, cluster: %d,time cost is %d milliseconds\n",
           range_index, timeval_diff(NULL, &later, &earlier) / 1000);

    //3.find the kernel core code page cluster, and print it
    int osNumber = initDb();

    if (gettimeofday(&earlier, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }

    int max_len = 0, max_index = 0;
    for (i = 1; i <= range_index; i++) {
//              printf("start:%x, end:%x: len:%x kernel\n", ranges[i].start, ranges[i].end, ranges[i].len);
        if (ranges[i].len > max_len) {
            max_index = i;
            max_len = ranges[i].len;
        }
    }

    //4.print md5 of pages that can be disassembled

    int availableOs[FINGERPRINT_NO], matchCounts[FINGERPRINT_NO];
    for (i = 0; i < FINGERPRINT_NO; i++) {
        availableOs[i] = 1;
        matchCounts[i] = 0;
    }
    startVirtualAddr = ranges[max_index].start;
    unsigned disasPageNo = 0;
    unsigned totalPageNo = 0;
    for (; startVirtualAddr <= ranges[max_index].end;
         startVirtualAddr += 0x1000) {
        totalPageNo++;
        unsigned pAddr =
            vtop(mem->mem, mem->mem_size, mem->pgd, startVirtualAddr);
        if (pAddr == -1 || pAddr > mem->mem_size)
            continue;
        int pageIndex = pAddr / pageSize;
        if (dsmPages[pageIndex] == 1) {
            int offset =
                (startVirtualAddr - ranges[max_index].start) / 4096;
            void *startAdress =
                (void *) ((unsigned) mem->mem + pageIndex * pageSize);
            unsigned char md5digest[16];
            MDMem(startAdress, pageSize, md5digest);
            //      printf("%x ", vaddr); //print vaddr
            MDPrint(md5digest);
            printf("\n");

            //search hash table
            int ret =
                matchByIndex(osNumber, md5digest, offset, availableOs,
                             matchCounts);

//                      genMd5WithOffset(startAdress, pageSize, startVirtualAddr, offset);
            disasPageNo++;
        }
    }

    if (gettimeofday(&later, NULL)) {
        perror("gettimeofday() error");
        exit(1);
    }
    printf("step4.time cost is %d milliseconds\n",
           timeval_diff(NULL, &later, &earlier) / 1000);

    int maxIndex = -1;
    int maxMatch = 0;
    for (i = 0; i < FINGERPRINT_NO; i++) {
        if (matchCounts[i] > maxMatch) {
            maxIndex = i;
            maxMatch = matchCounts[i];
        }
    }
    if (maxMatch > 0)
        printf("Os is %s, match count is %d\n",
               fingerprints[maxIndex].osVersion, maxMatch);
    else
        puts("Unknown OS!");

    return;
}

/* determine version of OS by mem
 * 1.To get signature of multiply versions of os, which is the md5 of kernel code
 * 2.scan all pages of input memory, generate the md5 checksum of one page, compare to all the signature,
 * 	 if they are match, output the version of the os.
 * 3.Done!
 * 4.abandoned*/
void determineOsVersion2(Mem * mem)
{
    //get signature
    int osNumber = initDb();

    int pageSize = 4 * 1024;    //4k
    int totalPageNumber = mem->mem_size / (4 * 1024);   //assume that every page has 4k

    //record when two page have different page index and the same sharp
    int calledPages[totalPageNumber];
    int dsmPages[totalPageNumber];
    //record virtual address
    int i;
    unsigned virtualAddrs[totalPageNumber];
    for (i = 0; i < totalPageNumber; i++) {
        calledPages[i] = 0;
        dsmPages[i] = 0;
        virtualAddrs[i] = 0;
    }

    //start address
    unsigned start_vaddr = 0x80000000;
    unsigned vaddr = start_vaddr;
    int matchCount = 0;
    int matchPageIndex = 0;
    int availableOs[FINGERPRINT_NO];
    for (i = 0; i < FINGERPRINT_NO; i++)
        availableOs[i] = 1;
    for (; vaddr > start_vaddr - 1; vaddr += 0x1000) {
        int rw = 0, us = 0, g = 0, ps = 0;      //page size 4M or 4k
        unsigned pAddr =
            vtopPageProperty(mem->mem, mem->mem_size, mem->pgd, vaddr, &rw,
                             &us, &g, &ps);
        if (pAddr == -1 || pAddr > mem->mem_size)
            continue;
        int pageIndex = pAddr / pageSize;

        if (us == 0 && g == 256
            && is_code_page(mem, vaddr, pageSize, virtualAddrs) == 0) {
            page_init(mem, pageIndex, pageSize, dsmPages, 0, vaddr,
                      calledPages);
            if (dsmPages[pageIndex] != 1)
                continue;

            void *startAdress =
                (void *) ((unsigned) mem->mem + pageIndex * pageSize);
            unsigned char md5digest[16];
            MDMem(startAdress, pageSize, md5digest);
            MDPrint(md5digest);
            printf("\n");

            //search hash table
            int ret =
                match(osNumber, md5digest, &matchPageIndex, availableOs);
            while (ret == 2) {
                matchPageIndex++;
                ret =
                    match(osNumber, md5digest, &matchPageIndex,
                          availableOs);
            }
            if (ret >= 0) {
                matchPageIndex++;
                matchCount++;
                if (ret == 1)
                    break;
            }

        }

    }


    if (matchCount == 0)
        puts("Unknown OS!");
    printf("match Count:%d\n", matchCount);
}
