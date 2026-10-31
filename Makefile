# Attribution: Jacob A Brennan, based on assignments from Dr. Margaret Black

compile:
	# Create disk image and load compiled bootloader
	nasm bootload.asm
	rm -f diskc.img
	dd if=/dev/zero of=diskc.img bs=512 count=1000
	dd if=bootload of=diskc.img bs=512 count=1 conv=notrunc
	# Compile Utility Programs
	gcc -o loadFile loadFile.c
	# Compile and Load Kernel
	as86 kernel.asm -o kernel_asm.o
	bcc -ansi -c -o kernel_c.o kernel.c && ld86 -o kernel -d kernel_c.o kernel_asm.o
	./loadFile kernel
	# Compile and Load Test Program(s)
	as86 -o userlib.o userlib.asm
	bcc -ansi -c -o tstpr1.o tstpr1.c && ld86 -d -o tstpr1 tstpr1.o userlib.o
	bcc -ansi -c -o tstpr2.o tstpr2.c && ld86 -d -o tstpr2 tstpr2.o userlib.o
	./loadFile tstpr1
	./loadFile tstpr2
	# Compile and Load Shell
	bcc -ansi -c -o shell_c.o shell.c && ld86 -s -o shell -d shell_c.o userlib.o
	./loadFile shell
	# Load test message
	./loadFile message.txt

compress:
	git archive --format=tar -o jacobabrennan.tar HEAD
