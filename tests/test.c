int main()
{
	volatile int *mem = (int *)0x200;
	*mem = 24;
	for (int i = 0; i < 8; ++i)
		*mem += 2;
	return *mem;
}
