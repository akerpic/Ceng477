#include "helper.h"
#include <glm/ext.hpp>
#include <glm/gtx/rotate_vector.hpp>

static GLFWwindow * win = NULL;

// Shaders
GLuint idProgramShader;
GLuint idFragmentShader;
GLuint idVertexShader;
GLuint idJpegTexture;
GLuint idMVPMatrix;

int widthTexture, heightTexture;

struct cam
{
    glm::vec3 camera_gaze = glm::vec3(0, 0, 1);
    glm::vec3 camera_up = glm::vec3(0, 1, 0);
    glm::vec3 camera_left = glm::cross(camera_up, camera_gaze);
    glm::vec3 camera_position;
    GLfloat camera_speed = 0.0f;
};

struct my_window{
    cam the_camera;
    bool is_fs = false;
    bool is_update = false;
    int fs_width;
    int fs_height;

    int normal_width = 600;
    int normal_height = 600;

    GLdouble pp_angle = 45;
    GLdouble aspect_ratio = 1;
    GLdouble near_distance = 0.1;
    GLdouble far_distance = 1000;
    GLfloat height_factor = 10.0f;

    GLFWmonitor *monitor;
    const GLFWvidmode *vidmode;

    glm::vec3 *vertex_array;

};
my_window the_window;

static void errorCallback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void check_directions(GLFWwindow* window,int key, int scancode, int action, int mods)
{

    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
    {
        the_window.the_camera.camera_up = glm::rotate(the_window.the_camera.camera_up, -0.01f, the_window.the_camera.camera_left);
        the_window.the_camera.camera_gaze = glm::rotate(the_window.the_camera.camera_gaze, -0.01f, the_window.the_camera.camera_left);
    }
    
    else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        the_window.the_camera.camera_up = glm::rotate(the_window.the_camera.camera_up, 0.01f,the_window.the_camera.camera_left);
        the_window.the_camera.camera_gaze = glm::rotate(the_window.the_camera.camera_gaze, 0.01f, the_window.the_camera.camera_left);
    }

    else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        the_window.the_camera.camera_left = glm::rotate(the_window.the_camera.camera_left, 0.01f, the_window.the_camera.camera_up);
        the_window.the_camera.camera_gaze = glm::rotate(the_window.the_camera.camera_gaze, 0.01f, the_window.the_camera.camera_up);
    }

    else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        the_window.the_camera.camera_left = glm::rotate(the_window.the_camera.camera_left, -0.01f, the_window.the_camera.camera_up);
        the_window.the_camera.camera_gaze = glm::rotate(the_window.the_camera.camera_gaze, -0.01f, the_window.the_camera.camera_up);
    }

}

static void check_camera_speed(GLFWwindow* window,int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_U && action == GLFW_PRESS) 
        the_window.the_camera.camera_speed += 0.25;
    
    else if (key == GLFW_KEY_J && action == GLFW_PRESS) 
        the_window.the_camera.camera_speed -= 0.25;

}

static void check_height_factor(GLFWwindow* window,int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_O && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
    {
        the_window.height_factor += 0.5;
        glUniform1f(glGetUniformLocation(idProgramShader, "heightFactor"), the_window.height_factor);
    }

    else if (key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
    {
        the_window.height_factor -= 0.5;
        glUniform1f(glGetUniformLocation(idProgramShader, "heightFactor"), the_window.height_factor);
    }
}

static void check_fullscreen(GLFWwindow* window,int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        if (the_window.is_fs)
            glfwSetWindowMonitor(window, nullptr, 0, 0, the_window.normal_width, the_window.normal_height, 0);
        else
            glfwSetWindowMonitor(window, the_window.monitor,0,0,the_window.vidmode->width, the_window.vidmode->height,the_window.vidmode->refreshRate);

        the_window.is_fs = !the_window.is_fs;
        the_window.is_update = true;
    } 
}

static void keyCallback_operations(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);    
   
    check_directions(window, key, scancode, action, mods);
    check_camera_speed(window, key, scancode, action, mods);
    check_height_factor(window, key, scancode, action, mods);
    check_fullscreen(window, key, scancode, action, mods);
}

void init(char * fname)
{
    the_window.monitor = glfwGetPrimaryMonitor();
    the_window.vidmode = (glfwGetVideoMode(the_window.monitor));

    the_window.fs_width = the_window.vidmode->width;
    the_window.fs_height = the_window.vidmode->height;

    glfwSetKeyCallback(win, keyCallback_operations);

    initShaders();
    glUseProgram(idProgramShader);
    initTexture(fname, &widthTexture, &heightTexture);

    the_window.vertex_array = new glm::vec3[6 * widthTexture * heightTexture]; 

    the_window.the_camera.camera_position = glm::vec3(widthTexture / 2, widthTexture / 10, (-1) * (widthTexture / 4));

    glm::vec3 inp = glm::vec3(the_window.the_camera.camera_position + the_window.the_camera.camera_gaze * the_window.near_distance);

    glm::mat4 projection_matrix = glm::perspective(the_window.pp_angle, the_window.aspect_ratio, the_window.near_distance, the_window.far_distance);

    glm::mat4 mvp = projection_matrix * glm::lookAt(the_window.the_camera.camera_position, inp, the_window.the_camera.camera_up) * glm::mat4(1.0f);
    
    glUniform3fv(glGetUniformLocation(idProgramShader, "cameraPosition"), 1, &the_window.the_camera.camera_position[0]);

    glUniformMatrix4fv(glGetUniformLocation(idProgramShader, "MVP"), 1, GL_FALSE, &mvp[0][0]);

    glUniformMatrix4fv(glGetUniformLocation(idProgramShader, "NM"), 1, GL_FALSE, &glm::inverseTranspose(glm::lookAt(the_window.the_camera.camera_position, inp, the_window.the_camera.camera_up))[0][0]);

    glUniformMatrix4fv(glGetUniformLocation(idProgramShader, "MV"), 1, GL_FALSE, &glm::lookAt(the_window.the_camera.camera_position, inp, the_window.the_camera.camera_up)[0][0]);


    glEnable(GL_DEPTH_TEST);

    glViewport(0,0, the_window.normal_width, the_window.normal_height);

    glUniform1i(glGetUniformLocation(idProgramShader, "widthTexture"), widthTexture);
    glUniform1i(glGetUniformLocation(idProgramShader, "heightTexture"), heightTexture);
    glUniform1f(glGetUniformLocation(idProgramShader, "heightFactor"), the_window.height_factor);
    glUniform1i(glGetUniformLocation(idProgramShader, "rgbTexture"), 0);


    int next = 0;
    
    for (int i = 0; i < widthTexture; i++) {
        for (int j = 0; j < heightTexture; j++) {

            the_window.vertex_array[next] = glm::vec3(i, 0, j);  
            the_window.vertex_array[next + 1] = glm::vec3(i + 1, 0, j + 1);                           
            the_window.vertex_array[next + 2] = glm::vec3(i + 1, 0, j); 
            the_window.vertex_array[next + 3] = glm::vec3(i, 0, j); 
            the_window.vertex_array[next + 4] = glm::vec3(i, 0, j + 1); 
            the_window.vertex_array[next + 5] = glm::vec3(i + 1, 0, j + 1); 

            next = next + 6;
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Please provide only a texture image\n");
        exit(-1);
    }

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    win = glfwCreateWindow(600, 600, "CENG477 - HW3", NULL, NULL);

    if (!win) {
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(win);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

        glfwTerminate();
        exit(-1);
    }

    init(argv[1]);

    while(!glfwWindowShouldClose(win)) 
    {

        if(the_window.is_update)
        {

            if(the_window.is_fs)
                glViewport( 0, 0, the_window.fs_width, the_window.fs_height);
            else
                glViewport( 0, 0, the_window.normal_width, the_window.normal_height);
            
            the_window.is_update = false;

        }

        glClearColor(0, 0, 0, 1);
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glClearDepth(1.0f);

        the_window.the_camera.camera_position += the_window.the_camera.camera_speed * the_window.the_camera.camera_gaze;

        glm::vec3 inp = glm::vec3(the_window.the_camera.camera_position + the_window.the_camera.camera_gaze * the_window.near_distance);

        glm::mat4 projection_matrix = glm::perspective(the_window.pp_angle, the_window.aspect_ratio, the_window.near_distance, the_window.far_distance);

        glm::mat4 mvp = projection_matrix * glm::lookAt(the_window.the_camera.camera_position, inp, the_window.the_camera.camera_up) * glm::mat4(1.0f);
        
        glUniform3fv(glGetUniformLocation(idProgramShader, "cameraPosition"), 1, &the_window.the_camera.camera_position[0]);

        glUniformMatrix4fv(glGetUniformLocation(idProgramShader, "MVP"), 1, GL_FALSE, &mvp[0][0]);

        glUniformMatrix4fv(glGetUniformLocation(idProgramShader, "NM"), 1, GL_FALSE, &glm::inverseTranspose(glm::lookAt(the_window.the_camera.camera_position, inp, the_window.the_camera.camera_up))[0][0]);

        glUniformMatrix4fv(glGetUniformLocation(idProgramShader, "MV"), 1, GL_FALSE, &glm::lookAt(the_window.the_camera.camera_position, inp, the_window.the_camera.camera_up)[0][0]);


        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, the_window.vertex_array);

        glDrawArrays(GL_TRIANGLES, 0, 6 * widthTexture * heightTexture);

        glDisableClientState(GL_VERTEX_ARRAY);
        
        glfwPollEvents();
        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
