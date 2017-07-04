#ifndef APPLICATION_H_
#define APPLICATION_H_
#include "types.h"
#include "timer.h"

class Application {
private:
	bool m_Terminate{ false };

	Timer m_Timer;

public:
	Application() = default;

	Application(const Application& application) = delete;

	Application& operator=(const Application& application) = delete;

	virtual ~Application() = default;

	void SetTermination(bool state) noexcept;

	bool ShouldTerminate() const noexcept;

	const Timer& GetTimer() const noexcept;

	virtual bool Initialize() noexcept = 0;

	virtual i32 Run() const noexcept = 0;

	virtual void Draw() const noexcept = 0;
};

#endif // APPLICATION_H_
