#include "demo_application.h"

int main(int argc, char* argv[])
{
	ApplicationSettings applicationSettings;
	applicationSettings.name = "Vulkan - CommandBufferRecording - Multithreaded per frame command buffers.";
	applicationSettings.windowResolution = Vec2i{ 1024, 768 };
	applicationSettings.windowPosition = Vec2i{ 512, 512 };
	applicationSettings.vsync = false;

	DemoApplication app{ applicationSettings };

	if (!app.Initialize()) {
		return 1;
	}

	return app.Run();
}
