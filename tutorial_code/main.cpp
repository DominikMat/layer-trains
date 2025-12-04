#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include "Shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}  

void processInput(GLFWwindow *window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    glfwInit();

    // select OpenGL version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // select OpenGL Core Profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    
    // Create window object
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // Initalize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }   
    
    Shader shader1(
        "C:/Media/Projects/OpenGL/Layer_Trains/src/shaders/vertexWithColourInfo.vs", 
        "C:/Media/Projects/OpenGL/Layer_Trains/src/shaders/fragmentSineColour.fs"
    );
    Shader shader2(
        "C:/Media/Projects/OpenGL/Layer_Trains/src/shaders/vertexOutputPos.vs", 
        "C:/Media/Projects/OpenGL/Layer_Trains/src/shaders/fragmentInputPos.fs"
    );

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // first triangle
        0.6f, -0.4f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
        0.6f, 0.6f, 0.0f,  0.0f, 1.0f, 0.0f,   // top right
        -0.4f,  0.6f, 0.0f,  0.0f, 0.0f, 1.0f    // top left 
    };

    float vertices2[] = {
        // second triangle
        0.4f, -0.6f, 0.0f,  // bottom right
        -0.6f, -0.6f, 0.0f,  // bottom left
        -0.6f,  0.4f, 0.0f   // top left
    }; 

    unsigned int VAO, VBO, VAO2, VBO2;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float))); // col
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    // ===========
    // triangle 2
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);

    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    // render loop
    while(!glfwWindowShouldClose(window))
    {
        // input handling
        processInput(window);

        // rendering
            // clear buffer with grey colour
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // wireframe
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            
            // draw triangle
            float timeValue = (sin(glfwGetTime())/2.0f) +.5f;
            shader1.use();
            shader1.setFloat("timeMult", timeValue);
            glBindVertexArray(VAO); 
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            shader2.use();
            glBindVertexArray(VAO2); 
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

        // swap buffer and event poll
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // clean up after application close
    glfwTerminate();

    return 0;
}