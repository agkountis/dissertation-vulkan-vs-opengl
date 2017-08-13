#include "static_scene_low_poly_application.h"

int main(int argc, char* argv[])
{
	ApplicationSettings applicationSettings;
	applicationSettings.name = "Vulkan Single Threaded Benchmark";
	applicationSettings.windowResolution = Vec2i{ 1024, 768 };
	applicationSettings.vsync = false;

	StaticSceneLowPolyApplication app{ applicationSettings };

	if (!app.Initialize()) {
		return 1;
	}

	return app.Run();
}
