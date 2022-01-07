#include "App.h"

int main(int _argc, char *_argv[])
{
	App<renderer::Deferred>().run();

	return EXIT_SUCCESS;
}