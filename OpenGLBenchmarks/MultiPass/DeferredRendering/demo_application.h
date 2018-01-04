#ifndef DISSERTATION_DEMO_APPLICATION_H_
#define DISSERTATION_DEMO_APPLICATION_H_

#include "demo_scene.h"
#include "gl_application.h"

class DemoApplication final : public GLApplication {
private:
	DemoScene m_DemoScene;

public:
	explicit DemoApplication(const ApplicationSettings& settings);

	bool Initialize() noexcept override;

	void Update() noexcept override;

	void Draw() noexcept override;

	void OnResize(const Vec2i& size) noexcept override;
};

#endif // DISSERTATION_DEMO_APPLICATION_H_
