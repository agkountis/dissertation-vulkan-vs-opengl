#include <iostream>
#include "vulkan_single_threaded_application.h"

int main(int argc, char* argv[])
{
	VulkanSingleThreadedApplication app;

	if (!app.Initialize()) {
		return 1;
	}

	return app.Run();
}
