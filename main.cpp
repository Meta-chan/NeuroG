#include "neurog.h"

int main()
{
	//simple net trying to learn xor operation

	float input[2];
	float output, goal;

	bool ok;
	unsigned int lays[3] = { 2, 2, 1 };
	NeuroG net(3, lays, 1.0f, &ok);
	if (!ok) return 1;
	net.set_coefficient(0.1f);

	for (unsigned int i = 0; i < 10000; i++)
	{
		char b[2];
		b[0] = rand() % 2;
		b[1] = rand() % 2;
		input[0] = (float)(2 * b[0] - 1);
		input[1] = (float)(2 * b[1] - 1);
		goal = (float)(2 * (b[0] ^ b[1]) - 1);
		
		net.set_input(input);
		net.set_output_pointer(&output);
		net.forward();
		net.get_output();
		printf("%i %i -> %f\n", b[0], b[1], output);
	
		net.set_goal(&goal);
		net.backward();
	}

	getchar();
};