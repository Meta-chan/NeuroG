#include "neurog.h"

int main()
{
	//simple net trying to learn xor operation

	float input[2];
	float output, goal;

	bool ok;
	unsigned int lays[2] = { 2, 1 };
	NeuroG net(2, lays, &ok);
	if (!ok) return 1;
	net.set_koef(0.001f);

	for (unsigned int i = 0; i < 100000; i++)
	{
		char b[2];
		b[0] = rand() % 2;
		b[1] = rand() % 2;
		input[0] = (float)(2 * b[0] - 1);
		input[1] = (float)(2 * b[1] - 1);
		goal = (float)(2 * (b[0] & b[1]) - 1);
		
		net.set_input(input);
		net.forward();
		net.get_output(&output);
		printf("%i %i -> %f\n", b[0], b[1], output);
	
		net.set_goal(&goal);
		net.backward();
	}

	getchar();
};