#include "Renderer.hpp"

int main()
{
    std::vector<std::unique_ptr<Scene>> scenes;

    // Add scenes to the vector using make_unique
    scenes.push_back(std::make_unique<Scene>("scene0.crtscene"));

    // Loop over the scenes and render them
    for (auto& scene : scenes) {
        Renderer renderer(*scene);  // Pass the dereferenced unique_ptr to the renderer
        renderer.RenderImage();
    }

	return 0;
}