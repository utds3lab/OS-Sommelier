/*
 * memory.h
 *
 *  Created on: Dec 16, 2011
 *      Author: yufei gu
 *  Copyright : Copyright 2012 by UTD. all rights reserved. This material may
 	 	 	   be freely copied and distributed subject to inclusion of this
 	 	 	   copyright notice and our World Wide Web URL http://www.utdallas.edu
 */

#ifndef MEMORY_
#define MEMORY_
#define FINGERPRINT_NO 100   //100 signatures, each one present one
                             //specific OS kernel version.
typedef struct mem{
	char * mem;
	int mem_size;
	unsigned pgd;
}Mem;
typedef struct node_ {
	unsigned offset;
	struct node_ * next;
} node;

typedef struct range{
	unsigned start;
	unsigned end;
	unsigned len;
	int disasBytes;
	node * offset;
} range;

typedef struct cluster_{
	unsigned start;
	unsigned end;
	unsigned pageSize;
}cluster;


typedef struct fingerprint {
	char osVersion[100];
	unsigned char md5[4000][16];
} fingerprint;

int free_mem(Mem *mem);

#endif /* MEMORY_ */
