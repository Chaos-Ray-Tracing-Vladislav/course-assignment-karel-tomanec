#include "Renderer.hpp"

int main()
{
	std::vector<Scene> scenes{
		Scene{ "scene0.crtscene" },
		//Scene{ "scene1.crtscene" },
		//Scene{ "scene2.crtscene" },
		//Scene{ "scene3.crtscene" },
		//Scene{ "scene4.crtscene" },
	};

	for (auto& scene : scenes)
	{
		Renderer renderer(scene);
		renderer.RenderImage();
	}

	return 0;
}