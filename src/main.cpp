#include "App.h"

ASSERT_VK_STATIC_VARS_FUNC();

int main(int _argc, char *_argv[])
{
	App<renderer::Deferred>().run();

	return EXIT_SUCCESS;
}