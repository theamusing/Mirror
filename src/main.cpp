#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <opengl/shader.hpp>
#include <opengl/camera.hpp>
#include <opengl/model.hpp>
#include <opengl/light.hpp>
#include <opengl/skyBox.hpp>
#include <opengl/reflectPlane.hpp>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(-0.5f, 0.0f, 0.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mirror", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // set camera view info
    // --------------------
    camera.aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    camera.near = 0.1f;
    camera.far = 100.0f;
    camera.resolution = glm::vec2(SCR_WIDTH, SCR_HEIGHT);

    // build and compile shaders
    // -------------------------
    Shader ourShader("../resources/shaders/model_lighting.vs", "../resources/shaders/model_lighting.fs");
    Shader reflectShader("../resources/shaders/mirror.vs", "../resources/shaders/mirror.fs");
    Shader skyboxShader("../resources/shaders/skybox.vs", "../resources/shaders/skybox.fs");

    // load models
    // -----------
    Model ourModel("../resources/models/wooden-stylised-carriage/040404.fbx", false);
    ourModel.scale = glm::vec3(0.01,0.01,0.01);
    ourModel.position = glm::vec3(0.0f, -1.5f, 2.0f);
    ourModel.rotateAxisAngle(glm::vec3(0.0f, 1.0f, 0.0f), 75.0f);

    Model frame("../resources/models/mirror/classical-mirror/source/frame.fbx", false);
    frame.position = glm::vec3(-5 * sin(glm::radians(0.0f)), 0.1f, -5 * cos(glm::radians(0.0f)));
    frame.rotateAxisAngle(glm::vec3(0.0f, 1.0f, 0.0f), 0);
    frame.scale = glm::vec3(0.05f, 0.05f, 0.05f);

    vector<Model> modelList;
    modelList.push_back(ourModel);
    modelList.push_back(frame);

    // generate mirrors
    // ---------------
    ReflectPlaneManager ourReflectPlaneManager;
    for(int i = 0; i < 4; i++)
    {
        ReflectPlane mirror("../resources/models/mirror/classical-mirror/source/mirror.fbx", glm::vec3(0.0f, 0.0f, 1.0f), false);
        mirror.model.position = glm::vec3(i * 1.0f - 2.0f, -0.1f, -2.0f);
        mirror.model.scale = glm::vec3(0.02f, 0.02f, 0.02f);
        mirror.color = glm::vec3(i*0.25 + 0.75);
        // mirror.color = glm::vec3(1.0,0.5,0.5);
        mirror.blurLevel = i * 0.7f;
        ourReflectPlaneManager.addReflectPlane(mirror);

        Model frame("../resources/models/mirror/classical-mirror/source/frame.fbx", false);
        frame.position = glm::vec3(i * 1.0f - 2.0f, -0.1f, -2.0f);
        frame.scale = glm::vec3(0.02f, 0.02f, 0.02f);
        modelList.push_back(frame);
    }
    // generate a light source
    LightManager ourLightManager;
    // ourLightManager.addPointLight(glm::vec3(2.0f, 0.0f, 2.0f), glm::vec3(1.0f, 0.0f, 0.0f), 1.0f);
    // ourLightManager.addSpotLight(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.6f, 5.0f, 15.0f);

    // generate skybox
    SkyBox ourSkyBox;
    ourSkyBox.loadTexture({
        "../resources/textures/skybox/right.jpg",
        "../resources/textures/skybox/left.jpg",
        "../resources/textures/skybox/top.jpg",
        "../resources/textures/skybox/bottom.jpg",
        "../resources/textures/skybox/front.jpg",
        "../resources/textures/skybox/back.jpg"
    }, false);

    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // render the model
        ourShader.use();
        ourLightManager.Attach(ourShader);
        ourShader.setCamera(camera);
        ourModel.Draw(ourShader);

        // render mirror
        ourReflectPlaneManager.generateReflection(camera, ourLightManager, modelList);
        reflectShader.use();
        ourLightManager.Attach(reflectShader);
        ourSkyBox.Attach(reflectShader, 0);
        reflectShader.setCamera(camera);
        ourReflectPlaneManager.Draw(reflectShader, 1);

        // render skybox
        skyboxShader.use();
        skyboxShader.setCamera(camera);
        ourSkyBox.Draw(skyboxShader);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}