LIBS=-L./xed2/xed2-ia32/lib -lxed
CFLAGS=-I./xed2/xed2-ia32/include 
all:
	gcc -o signa  code_init.c  getPgd.c md5c.c vtop.c pageMd5Comparison.c main.c mddriver.c memload.c memory.c determineOsVersion.c $(LIBS) $(CFLAGS) -g
clean:
	rm -f signa
