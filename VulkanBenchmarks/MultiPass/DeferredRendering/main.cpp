#include "demo_application.h"

int main(int argc, char* argv[])
{
	ApplicationSettings applicationSettings;
	applicationSettings.name = "Vulkan - MultiPass - Deferred Shading";
	applicationSettings.windowResolution = Vec2i{ 1920, 1080 };
	applicationSettings.windowPosition = Vec2i{ 512, 512 };
	applicationSettings.vsync = false;

	DemoApplication app{ applicationSettings };

	if (!app.Initialize()) {
		return 1;
	}

	return app.Run();
}
