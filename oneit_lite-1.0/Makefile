all:
	gcc -Os -s -o oneit_lite oneit_lite.c


static:
	gcc -Os -s -static -o oneit_lite oneit_lite.c


386:
	gcc -Os -s -march=i386 -mtune=i386 -m32 -o oneit_lite oneit_lite.c


386-static:
	gcc -Os -s -march=i386 -mtune=i386 -m32 -static -o oneit_lite oneit_lite.c


clean:
	rm -f oneit_lite
