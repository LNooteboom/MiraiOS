all:	out.img
	mkdir temp
	mount out.img temp/
	cp BTST2.BIN temp/
	
	umount temp/

out.img: src/bootsector.asm
	nasm -f bin -o out.img src/bootsector.asm

clean:
	rm -r temp
