#ifndef APPLICATION_H_
#define APPLICATION_H_
#include "types.h"

class Application {
private:
	bool m_Terminate{ false };

public:
	Application() = default;

	Application(const Application& application) = delete;

	Application& operator=(const Application& application) = delete;

	virtual ~Application() = default;

	void SetTermination(bool state) noexcept;

	bool ShouldTerminate() const noexcept;

	f64 GetDelta() const noexcept;

	virtual bool Initialize() = 0;

	virtual i32 run() = 0;
};

#endif // APPLICATION_H_
