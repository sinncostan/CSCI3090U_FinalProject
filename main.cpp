#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "shader.hpp"
#include "objectModel.cpp"

using namespace glm;

GLuint textureId;

int height = 480;
int width = 640;
bool swapper = false;
int rot = 10;
bool rotation = false;

glm::mat4 modelMatrix;
glm::mat4 modelMatrix2;
glm::mat4 viewMatrix;
glm::mat4 viewMatrix2;

glm::vec3 eyePosition(-150, 85, 150);
glm::mat4 projectionMatrix;

float yoffset = 1.0f;
float xoffset = 0.0f;
float zoffset = 0.0f;

float xrot = 0.0f;
float yrot = 0.0f;
float zrot = 0.0f;



// Model related stuff
GLint vertex_attribute;

GLuint positions_vbo = 0;
GLuint textureCoords_vbo = 0;
GLuint normals_vbo = 0;

GLfloat vertices[] = {-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
                      -1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  1.0f,
                      -1.0f, 1.0f,  1.0f, 1.0f, -1.0f, 1.0f};

static void loadTexture(char const * path){
    int imageWidth, imageHeight;
   int numComponents;

   // load the image data into a bitmap
   unsigned char *bitmap = stbi_load(path,
                                     &imageWidth,
                                     &imageHeight,
                                     &numComponents, 4);

   // generate a texture name
   glGenTextures(1, &textureId);

   // make the texture active
   glBindTexture(GL_TEXTURE_2D, textureId);

   // make a texture mip map
   glGenerateTextureMipmap(textureId);
   glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

   // specify the functions to use when shrinking/enlarging the texture image
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

   // specify the tiling parameters
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   // send the data to OpenGL
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imageWidth, imageHeight,
                0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);

   // bind the texture to unit 0
   glBindTexture(GL_TEXTURE_2D, textureId);
   glActiveTexture(GL_TEXTURE0);

   // free the bitmap data
   stbi_image_free(bitmap);
}


static void createObject(std::string objectFile) {


    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    objectModel objload(objectFile); 

    // VERTEX BUFFER OBJECTS (vbo)
    glGenBuffers(1, &positions_vbo);  // Create a buffer
    // Send the buffer to the GPU and make it active
    glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
    // Upload the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, objload.vertices.size() * sizeof(vec3), &objload.vertices[0], GL_STATIC_DRAW);


        // VERTEX BUFFER OBJECTS (vbo)
    glGenBuffers(1, &textureCoords_vbo);  // Create a buffer
    // Send the buffer to the GPU and make it active
    glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo);
    // Upload the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, objload.uvs.size() * sizeof(vec3), &objload.uvs[0] , GL_STATIC_DRAW);

        // VERTEX BUFFER OBJECTS (vbo)
    glGenBuffers(1, &normals_vbo);  // Create a buffer
    // Send the buffer to the GPU and make it active
    glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
    // Upload the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, objload.normals.size() * sizeof(vec3), &objload.normals[0], GL_STATIC_DRAW);


}

float light_y = 0.0f;
bool light_up = false;

bool animation = false;
bool updated = false;

static void drawObject(GLuint programID, GLuint objectVbo, glm::mat4 mvp, glm::mat4 mv,
                       glm::vec4 colour, glm::vec4 diffuse) {
    glUseProgram(programID);

    GLuint gpuMvp = glGetUniformLocation(programID, "u_MVP_Array");
    glUniformMatrix4fv(gpuMvp, 1, GL_FALSE, &mvp[0][0]);

    GLuint gpuColour = glGetUniformLocation(programID, "u_Colour");
    glUniform4fv(gpuColour, 1, (GLfloat *)&colour[0]);

    GLuint gpuMV = glGetUniformLocation(programID, "u_MV");
    glUniformMatrix4fv(gpuMV, 1, GL_FALSE, &mv[0][0]);


    // attribute vec4 position;
    // attribute vec2 textureCoords;
    // attribute vec3 normal;

    GLint positionAttribId = glGetAttribLocation(programID, "position");
	GLint textureCoordsAttribId = glGetAttribLocation(programID, "textureCoords");
	GLint normalAttribId = glGetAttribLocation(programID, "normal");

	// the position of our light
	GLuint lightPosId = glGetUniformLocation(programID, "u_LightPos");
	glUniform3f(lightPosId, 150 ,light_y , 90);
	//gg

         // the position of our camera/eye
    GLuint eyePosId = glGetUniformLocation(programID, "u_EyePosition");
    glUniform3f(eyePosId, eyePosition.x, eyePosition.y, eyePosition.z);

	// the colour of our object
	GLuint diffuseColourId = glGetUniformLocation(programID, "u_DiffuseColour");
	glUniform4f(diffuseColourId, diffuse.x, diffuse.y, diffuse.z, diffuse.w);
	
	
	   // the shininess of the object's surface
   GLuint shininessId = glGetUniformLocation(programID, "u_Shininess");
   glUniform1f(shininessId, 25);

//     //Colour of Light
//     GLuint ambientColourId = glGetUniformLocation(programID, "u_AmbientColour");
//    glUniform4f(ambientColourId, colour.x, colour.y, colour.z, colour.w);

   GLuint swapLightandTexture = glGetUniformLocation(programID, "u_ShaderTextureSwap");
    glUniform1f(swapLightandTexture, swapper);

	// provide the vertex positions to the shaders
	glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
	glEnableVertexAttribArray(positionAttribId);
	glVertexAttribPointer(positionAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// provide the vertex texture coordinates to the shaders
	glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo);
	glEnableVertexAttribArray(textureCoordsAttribId);
	glVertexAttribPointer(textureCoordsAttribId, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// provide the vertex normals to the shaders
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glEnableVertexAttribArray(normalAttribId);
	glVertexAttribPointer(normalAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, objectVbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(vertices[0]));
    glDisableVertexAttribArray(0);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        if(swapper == true) {
            swapper = false;
        } 
        else {
            swapper = true;
        }
    }
    else if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
	   if (rotation) {
		   rotation = false;
	   }
	   else {
		   rotation = true;
	   }
	  
    }
	else if (key = GLFW_KEY_A && action == GLFW_PRESS) {
	   xoffset = 7.0f;
	   xrot = 0.0f;
	   animation = true;
   }

}



static void drawSqaures(GLuint programID){

    modelMatrix = glm::mat4(1.0f);
	
	
	//     glm::rotate(model1Matrix, glm::radians(0.0f), glm::vec3(1, 0, 0));
    // modelMatrix =
    //     glm::rotate(modelMatrix, glm::radians(0.0f), glm::vec3(0, 1, 0));
    // modelMatrix =
    //     glm::rotate(modelMatrix, glm::radians(0.0f), glm::vec3(0, 0, 1));
    // modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0, 1.0, 1.0));

    // Render the objects


    glm::vec4 red(0.3, 0.0, 1.0, 1.0);
    glm::vec4 diffusetop(0.8, 0.4, 1.0, 1.0);
    glm::vec4 diffuseleft(0.6, 0.0, 1.0, 1.0);
    glm::vec4 diffuseright(0.4, 0.0, 1.0, 1.0);

    //drawObject(programID, vbo, projectionMatrix * viewMatrix * modelMatrix, red);



  //glm::mat4 modelMatrix = glm::mat4(1.0f);
 
  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 2.5f, 0.0f));

  modelMatrix = glm::rotate(modelMatrix,glm::radians(0.0f),glm::vec3(1,0,0));//rotation x = 0.0 degrees
  modelMatrix = glm::rotate(modelMatrix,glm::radians(0.0f),glm::vec3(0,1,0));//rotation y = 0.0 degrees
  modelMatrix = glm::rotate(modelMatrix,glm::radians(0.0f),glm::vec3(0,0,1));

  modelMatrix = glm::scale(modelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
  //modelMatrix = glm::translate(modelMatrix, glm::vec3(0.1, 1.0, 1.0));

  drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffusetop);


  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 2.5f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffusetop);


  modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.5f, 0.0f, -2.5f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffusetop);
  

  modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.5f, 0.0f, 0.0f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffusetop);

  
  modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.25f, 0.0f, 0.0f));

  modelMatrix = glm::rotate(modelMatrix,glm::radians(0.0f),glm::vec3(1,0,0));//rotation x = 0.0 degrees
  modelMatrix = glm::rotate(modelMatrix,glm::radians(0.0f),glm::vec3(0,1,0));//rotation y = 0.0 degrees
  modelMatrix = glm::rotate(modelMatrix,glm::radians(90.0f),glm::vec3(0,0,1));

  modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.25f, 0.0f, 0.0f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffuseleft);

  
  modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.5f, 0.0f, 0.0f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffuseleft);

  
  modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.5f, 0.0f, 0.0f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffuseleft);


  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 2.5f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffuseleft);



  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 2.5f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffuseleft);

  
  // TODO:  Draw the fingers (and thumb)
  //glm::vec3 base(0.0,0.0,0.0);
  


  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 2.25f));

  modelMatrix = glm::rotate(modelMatrix,glm::radians(90.0f),glm::vec3(1,0,0));//rotation x = 0.0 degrees
  modelMatrix = glm::rotate(modelMatrix,glm::radians(0.0f),glm::vec3(0,1,0));//rotation y = 0.0 degrees
  modelMatrix = glm::rotate(modelMatrix,glm::radians(0.0f),glm::vec3(0,0,1));
  
  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 2.25f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffuseright);


  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 2.5f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffuseright);


  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 2.5f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffuseright);
  

  modelMatrix = glm::translate(modelMatrix, glm::vec3(2.5f, 0.0f, 0.0f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffuseright);


  modelMatrix = glm::translate(modelMatrix, glm::vec3(2.5f, 0.0f, 0.0f));

   drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffuseright);


  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -2.5f));

  drawObject(programID, positions_vbo, projectionMatrix * viewMatrix * modelMatrix, projectionMatrix * viewMatrix, red, diffuseright);
    
  modelMatrix2 = modelMatrix;
  viewMatrix2 = viewMatrix;

  
  


  modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(-0.75 + xoffset, 1.0f , -1.95f + zoffset));
  modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(0.0f+xrot), glm::vec3(1, 0, 0));//rotation x = 0.0 degrees

  modelMatrix2 = glm::scale(modelMatrix2, glm::vec3(1.9, 1.9, 1.9));
  drawObject(programID, positions_vbo, projectionMatrix * viewMatrix2 * modelMatrix2, projectionMatrix * viewMatrix2, red, diffuseright);




  modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(0.0f, -2.0f, 0.0));
  modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(90.0f), glm::vec3(1, 0, 0));//rotation x = 0.0 degrees
  drawObject(programID, positions_vbo, projectionMatrix * viewMatrix2 * modelMatrix2, projectionMatrix * viewMatrix2, red, diffusetop);

	


  modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(-90.0f), glm::vec3(1, 0, 0));//rotation x = 0.0 degrees
  drawObject(programID, positions_vbo, projectionMatrix * viewMatrix2 * modelMatrix2, projectionMatrix * viewMatrix2, red, diffuseleft);

  modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(-90.0f), glm::vec3(1, 0, 0));//rotation x = 0.0 degrees
  drawObject(programID, positions_vbo, projectionMatrix * viewMatrix2 * modelMatrix2, projectionMatrix * viewMatrix2, red, diffusetop);

  modelMatrix2 = glm::rotate(modelMatrix2,glm::radians(90.0f),glm::vec3(0,0,1));
  drawObject(programID, positions_vbo, projectionMatrix * viewMatrix2 * modelMatrix2, projectionMatrix * viewMatrix2, red, diffusetop);

  modelMatrix2 = glm::rotate(modelMatrix2, glm::radians(-90.0f), glm::vec3(0, 0, 1));
  drawObject(programID, positions_vbo, projectionMatrix * viewMatrix2 * modelMatrix2, projectionMatrix * viewMatrix2, red, diffusetop);

}

static void render(GLFWwindow *window, GLuint programID) {
    // Turn on depth buffering (dont render objects overtop of eachother)
    glEnable(GL_DEPTH_TEST);

    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    GLuint textureUniformId = glGetUniformLocation(programID, "textureSampler");
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, textureId);
   //glUniform1i(textureUniformId, 0);

    // Calculate the model matrix (transformations for the model)
    
    if(light_up){
        light_y += 0.01;
    } else {
        light_y -= 0.01;
    }

    if(light_y > 35){
        light_up = false;
    } else if (light_y < -35) {
        light_up = true;
    }

	
	if (rotation) {
		viewMatrix = glm::rotate(viewMatrix, glm::radians(0.1f), glm::vec3(0, 1, 0));
	}



	//yoffset+= 0.009f;
	if (xoffset > 0.0f) {
		xoffset -= 0.001f;
		xrot += 0.10295f;
	}
	if (xoffset  < 0.45f && xoffset > 0.2f) {
		viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 0.1f, 0.0f));
	}
	else if (xoffset > 0.0f && xoffset < 0.2f) {
		viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, -0.1f, 0.0f));
	}
	
	
	
	


    
    drawSqaures(programID);



}

static GLFWwindow *init_opengl() {
    // The variable to hold the window in
    GLFWwindow *window;

    // Init GLFW
    if (!glfwInit()) exit(EXIT_FAILURE);

    // Create the window
    window = glfwCreateWindow(width, height, "IDK SINTHOO STUFF", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    // Make the window active
    glfwMakeContextCurrent(window);
    glewInit();

    return window;
}

int main(void) {
    // Init the scene
    GLFWwindow *window = init_opengl();
    glfwSetKeyCallback(window, keyboard);

    // Create/Load the objects
    createObject("plane.obj");
    loadTexture("test.png");


    // Load the shaders
    GLuint programID = createShaderProgram("vertex.glsl", "fragment.glsl");


    // Calculate the projection
    projectionMatrix = glm::perspective(glm::radians(45.0f),
                                        (float)width / height, 0.1f, 1000.0f);

    // Calculate the view matrix (where we're looking at)
    viewMatrix =
        glm::lookAt(eyePosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    while (!glfwWindowShouldClose(window)) {
        // Render the scene

        render(window, programID);

        // Display the image
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
