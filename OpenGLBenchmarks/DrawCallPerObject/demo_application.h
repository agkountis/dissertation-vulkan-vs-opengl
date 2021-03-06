#ifndef DEMO_APPLICATION_H_
#define DEMO_APPLICATION_H_
#include "demo_scene.h"
#include "gl_application.h"

class DemoApplication final : public GLApplication {
private:
	DemoScene m_DemoScene;

public:
	DemoApplication(const ApplicationSettings& settings) noexcept;

	bool Initialize() noexcept override;

	void Update() noexcept override;

	void Draw() noexcept override;

	void OnResize(const Vec2i& size) noexcept override;
};

#endif //DEMO_APPLICATION_H_
