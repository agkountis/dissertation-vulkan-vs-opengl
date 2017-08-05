#ifndef APPLICATION_H_
#define APPLICATION_H_
#include "types.h"
#include "timer.h"
#include "resource_manager.h"
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
	explicit Application(const ApplicationSettings& settings);

	Application(const Application& application) = delete;

	Application& operator=(const Application& application) = delete;

	virtual ~Application() = default;

	void SetTermination(bool state) noexcept;

	bool ShouldTerminate() const noexcept;

	Timer& GetTimer() noexcept;

	const ApplicationSettings& GetSettings() const noexcept;

	void SetSettings(const ApplicationSettings& settings) noexcept;

	virtual bool Initialize() noexcept = 0;

	virtual i32 Run() noexcept = 0;

	virtual void Update() noexcept = 0;

	virtual void PreDraw() noexcept = 0;

	virtual void Draw() noexcept = 0;

	virtual void PostDraw() noexcept = 0;

	virtual void OnResize(const Vec2i& size) noexcept = 0;
};

#endif // APPLICATION_H_
