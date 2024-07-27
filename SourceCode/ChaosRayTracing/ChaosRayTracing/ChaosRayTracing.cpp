#include <iostream>

#include "Renderer.hpp"

int main()
{
    std::vector<std::unique_ptr<Scene>> scenes;

    // Add scenes to the vector using make_unique
    scenes.push_back(std::make_unique<Scene>("scene0.crtscene"));
    scenes.push_back(std::make_unique<Scene>("scene1.crtscene"));

    // Loop over the scenes and render them
    for (auto& scene : scenes) {
        Renderer renderer(*scene);  // Pass the dereferenced unique_ptr to the renderer

        // Start time
        auto start = std::chrono::high_resolution_clock::now();

        // Render the image
        renderer.RenderImage();

        // End time
        auto end = std::chrono::high_resolution_clock::now();

        // Calculate duration
        std::chrono::duration<double> duration = end - start;
        std::cout << scene->settings.sceneName + " rendering time: " << duration.count() << " seconds" << std::endl;
    }

	return 0;
}