#include "demo_application.h"

int main(int argc, char* argv[])
{
	ApplicationSettings applicationSettings;
	applicationSettings.name = "OpenGL - Driver Overhead - Draw Call Count";
	applicationSettings.windowResolution = Vec2i{ 1920, 1080 };
	applicationSettings.windowPosition = Vec2i{ 10, 50 };
	applicationSettings.vsync = false;

	DemoApplication app{ applicationSettings };

	if (!app.Initialize()) {
		return 1;
	}

	return app.Run();
}