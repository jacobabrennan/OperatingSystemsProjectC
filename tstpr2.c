main()
{
	syscall(0,"tstpr2 is working!\r\n");
	syscall(5);
	syscall(0,"tstpr2 still executing\r\n");
	while(1);
}
