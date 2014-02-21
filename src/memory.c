/*
 * memory.c
 *
 *  Created on: Dec 16, 2011
 *      Author: yufei gu
 *  Copyright : Copyright 2012 by UTD. all rights reserved. This material may
 	 	 	   be freely copied and distributed subject to inclusion of this
 	 	 	   copyright notice and our World Wide Web URL http://www.utdallas.edu
 */

#include "memory.h"

extern void free(void *);
/*return 1,success; return 0, failed*/

int free_mem(Mem * mem){
	if (mem) {
		if (mem->mem){
			free(mem->mem);
		}
		free(mem);
		return 1;
	}
	return 0;
}
