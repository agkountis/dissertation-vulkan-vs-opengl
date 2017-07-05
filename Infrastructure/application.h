#ifndef APPLICATION_H_
#define APPLICATION_H_
#include "types.h"
#include "timer.h"
#include <string>

struct ApplicationSettings {
	std::string name;
	Vec2i windowResolution;
	Vec2i windowPosition;
	bool fullscreen;
	bool vsync;
};

class Application {
private:
	bool m_Terminate{ false };

	Timer m_Timer;

	ApplicationSettings m_Settings;

public:
	Application(const ApplicationSettings& settings);

	Application(const Application& application) = delete;

	Application& operator=(const Application& application) = delete;

	virtual ~Application() = default;

	void SetTermination(bool state) noexcept;

	bool ShouldTerminate() const noexcept;

	const Timer& GetTimer() const noexcept;

	const ApplicationSettings& GetSettings() const noexcept;

	void SetSettings(const ApplicationSettings& settings) noexcept;

	virtual bool Initialize() noexcept = 0;

	virtual i32 Run() const noexcept = 0;

	virtual void Draw() const noexcept = 0;
};

#endif // APPLICATION_H_
