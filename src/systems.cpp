#include "systems.hpp"

static engine::Shader UVMeshShader((unsigned)0); 

void engine::renderUVMeshes(Scene& scene) {

    if (UVMeshShader.ID == 0) {
        UVMeshShader = Shader("src/shaders/UVMesh");
    }

    Components cameras = scene.getComponent<Camera>();
    Components transforms = scene.getComponent<GlobalTransform>();
    
    Entity main_camera;
    for (unsigned i = 0; i < cameras.size(); i++) {
        if (cameras[i] != nullptr && cameras[i]->main) {
            if (transforms[i] != nullptr) {
                main_camera = i;
                break;
            }
            Logger::logError("Main camera has no GlobalTransform component");            
        }
        if (i+1 == cameras.size()) {
            Logger::logError("No main camera");
            return;
        }
    }
    
    Components meshes = scene.getComponent<UVMesh>();
    Components textures = scene.getComponent<Texture>();

    for (unsigned i = 0; i < transforms.size(); i++) {
        if (transforms[i] != nullptr && meshes[i] != nullptr && textures[i] != nullptr) {
            UVMesh* mesh = meshes[i];

            mesh->bind();

            // position attribute
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // texture coord attribute
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);

            UVMeshShader.use();
            UVMeshShader.setInt("texture_image", 0);
            textures[i]->use(0);

            UVMeshShader.setMatrix4f("transform", transforms[i]->getTransformationMatrix());

            IVector2 window_size;
            SDL_GetWindowSize(Engine::getWindow(), &window_size.x, &window_size.y);

            UVMeshShader.setMatrix4f("projection", Camera::getProjectionMatrix(*transforms[main_camera], window_size));

            glDrawElements(GL_TRIANGLES, mesh->indicies.size(), GL_UNSIGNED_INT, 0);
            // Unbind array
            glBindVertexArray(0);
        }
    }
}