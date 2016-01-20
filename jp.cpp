#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include "SOIL.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)

using namespace std;

struct VAO {
  GLuint VertexArrayID;
  GLuint VertexBuffer;
  GLuint ColorBuffer;

  GLenum PrimitiveMode;
  GLenum FillMode;
  int NumVertices;
};
typedef struct VAO VAO;

struct bb{
  float x;
  float y;
  float vx;
  float vy;
  float speedY;
  float speedX;
  float radius;
  float angle;
  float fixed;
  float dead;
};

typedef struct bb bird;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
} Matrices;

GLuint programID;

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
void createBall(float rad) ;
void quit(GLFWwindow *window);

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
  }

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
  struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
  {
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
      color_buffer_data [3*i] = red;
      color_buffer_data [3*i + 1] = green;
      color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
  }

/* Render the VBOs handled by VAO */
  void draw3DObject (struct VAO* vao);


  float rectangle_rot_dir = 1;
  bool rectangle_rot_status = true;

  float grass_rot_dir = 1;
  bool grass_rot_status = false;


void reshapeWindow (GLFWwindow* window, int width, int height)
{
  int fbwidth=width, fbheight=height;
  glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = 90.0f;

  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

  Matrices.projection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.1f, 500.0f);
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



VAO *rectangle, *Circle, *grass, *xAxis, *yAxis, *cannon, *cannonBase;

bird birds[4];

int current;

float cannon_angle = 0.0f;

float control_angle = 0.0f;

float gravity=0.98;

float grass_width = 100, grass_x = -70, grass_y = -70;

float camera_rotation_angle = 90, rectangle_rotation = 0, currtime,mass_circle=0.7;

void updateAngle(bird *newBird, int dir);
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
  void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
  {
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
      switch (key) {
        case GLFW_KEY_C:
        rectangle_rot_status = !rectangle_rot_status;
        break;
        case GLFW_KEY_A:
                          break;
        default:
                          break;
      }
    }

    if(action == GLFW_REPEAT)
    {
     switch (key) {
      case GLFW_KEY_A:
                      break;
      case GLFW_KEY_UP: cannon_angle+=1;
      control_angle+=1.5;
      updateAngle(&birds[current],1);
                      break;
      case GLFW_KEY_DOWN: cannon_angle-=1;
      control_angle-=1.5;
      updateAngle(&birds[current],1);
                      break;
      default:                
                      break;
    }
  }

  else if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE: quit(window);
                            break;
      case GLFW_KEY_A: birds[current].fixed = 1; 
                      break;
      case GLFW_KEY_UP: cannon_angle+=1;
                      control_angle+=1.5;
                      updateAngle(&birds[current],1);
                      break;
      case GLFW_KEY_DOWN: cannon_angle-=1;
                      control_angle-=1.5;
                      updateAngle(&birds[current],-1);
                      break;
      default:
      break;
    }
  }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
  switch (key) {
    case 'Q':
    case 'q':
    quit(window);
    break;
    default:
    break;
  }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  switch (button) {
    case GLFW_MOUSE_BUTTON_RIGHT:
    if (action == GLFW_RELEASE) {
      rectangle_rot_dir *= -1;
    }
    break;
    default:
    break;
  }
}

// Function to initialize individial birds
void initializeBirds(bird *newBird, float x, float y, float vx, float vy)
{
  newBird->x = x;
  newBird->y = y;
  newBird->vx = vx;
  newBird->vy = vy;
  newBird->speedX = 0.3;
  newBird->speedY = 0.7;
  newBird->radius = 5;
  newBird->angle = 45;
  newBird->fixed = 0;
  newBird->dead = 0;
}

void updateAngle(bird *newBird, int dir)
{
    newBird->speedY = (1)*sin(DEG2RAD(control_angle));
    newBird->speedX = (1)*cos(DEG2RAD(control_angle));
    cout << control_angle << endl;
    cout << newBird->speedX << endl;
    cout << newBird->speedY << endl;
}

void createGrass ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -100,-100,0, //  1
    100,-100,0, //  2
    -100, -70,0, //  3

    100,-100,0, //  2
    -100, -70,0, //  3
    100, -70, 0     // 4
  };

  static const GLfloat color_buffer_data [] = {
    0,0.7,0, // color 1
    0,0.7,0, // color 2
    0,0.7,0, // color 3

    0,0.8,0, // color 3
    0,0.8,0, // color 4
    0,0.8,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  grass = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createXAxis ()
{

  static const GLfloat vertex_buffer_data [] = {
   -100,0.2,0, // vertex 1
   -100,-0.2,0, // vertex 2
    100,-0.2,0, // vertex 3

    100,-0.2,0, // vertex 3
    100, 0.2,0, // vertex 4
   -100, 0.2,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  xAxis = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createYAxis ()
{

  static const GLfloat vertex_buffer_data [] = {
   0.2,-100,0, // vertex 1
   -0.2,-100,0, // vertex 2
    -0.2,100,0, // vertex 3

   -0.2,100,0, // vertex 3
    0.2,100, 0, // vertex 4
   0.2,-100, 0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  yAxis = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBall(float rad)
{

  rad = 2;

  int i,k=0;

  GLfloat vertex_buffer_data[1090]={};
  GLfloat color_buffer_data[1090]={};
  vertex_buffer_data[k++]=0;
  vertex_buffer_data[k++]=0;
  vertex_buffer_data[k++]=0;

  for (i = 1; i < 361; ++i)
  {
   vertex_buffer_data[k] = rad*cos(DEG2RAD(i));
   color_buffer_data[k] = 1;
       // cout << ((double) rand() / (RAND_MAX)) << endl;

   k++;
   vertex_buffer_data[k] = rad*sin(DEG2RAD(i));
   color_buffer_data[k] = 1;
   k++;
   vertex_buffer_data[k] = 0;
   color_buffer_data[k] = 1;
   k++;
 }

 Circle = create3DObject(GL_TRIANGLE_FAN, 361, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCannonBase()
{
  float rad = 5;

  int i,k=0;

  GLfloat vertex_buffer_data[1090]={};
  GLfloat color_buffer_data[1090]={};
  vertex_buffer_data[k++]=0;
  vertex_buffer_data[k++]=0;
  vertex_buffer_data[k++]=0;

  for (i = 1; i < 361; ++i)
  {
   vertex_buffer_data[k] = rad*cos(DEG2RAD(i));
   color_buffer_data[k] = 1;
       // cout << ((double) rand() / (RAND_MAX)) << endl;

   k++;
   vertex_buffer_data[k] = rad*sin(DEG2RAD(i));
   color_buffer_data[k] = 1;
   k++;
   vertex_buffer_data[k] = 0;
   color_buffer_data[k] = 1;
   k++;
 }

 cannonBase = create3DObject(GL_TRIANGLE_FAN, 361, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCannon ()
{

  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    20,0,0, // vertex 2
    20,-5,0, // vertex 3

    20,-5,0, // vertex 3
    0,-5,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  cannon = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void projectile(bird *newBird)
{
  if(newBird->fixed != 0)
  {
    newBird->dead += 0.05f;
    newBird->vy = newBird->speedY - (newBird->dead)*(0.15);
    newBird->vx = newBird->speedX;
    newBird->x += newBird->vx;
    newBird->y += newBird->vy ;

  }
}



void draw ()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram (programID);


  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  glm::mat4 VP = Matrices.projection * Matrices.view;

  glm::mat4 MVP;  // MVP = Projection * View * Model

  // Load identity to model matrix
  // Matrices.model = glm::mat4(1.0f);


  // glm::mat4 translateTriangle = glm::translate (glm::vec3(x_triangle, 0.0f, 0.0f)); // glTranslatef
  // glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  // glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  // Matrices.model *= triangleTransform; 
  // MVP = VP * Matrices.model; // MVP = p * V * M
  // glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  // draw3DObject(triangle);

  ///////////////////////////GRASS///////////////////////////
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(grass);

  ////////////////////////// X AXIS /////////////////////////
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(xAxis);

  ///////////////////////// Y AXIS //////////////////////////
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(yAxis);

  //////////////////////// CIRCLE  /////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCircle = glm::translate (glm::vec3(birds[0].x, birds[0].y, 0));        // glTranslatef
  Matrices.model *= translateCircle;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(Circle); 
  projectile(&birds[0]);


  ///////////////////// CANNON BODY /////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 rotateCannon = glm::rotate((float)(cannon_angle*M_PI/120.0f), glm::vec3(0,0,1));
  glm::mat4 translateCannon = glm::translate (glm::vec3(-70, -60, 0));        // glTranslatef
  Matrices.model *= (translateCannon * rotateCannon); // Rotate then Translate
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cannon);


  ////////////////////// CANON BASE ////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCannonBase = glm::translate (glm::vec3(-70, -63, 0));        // glTranslatef
  Matrices.model *= translateCannonBase;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cannonBase);

}


GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
      exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    glfwSetWindowCloseCallback(window, quit);

    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
  }

  void initGL (GLFWwindow* window, int width, int height)
  {
  // Generate the VAO, VBOs, vertices data & copy into the array buffer

  createGrass();
  createXAxis();
  createYAxis();
  createCannonBase();
  createCannon();
  createBall(birds[0].radius);


  programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


  reshapeWindow (window, width, height);

  glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
  glClearDepth (1.0f);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

  cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
  cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
  cout << "VERSION: " << glGetString(GL_VERSION) << endl;
  cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void makeBirds()
{
  for(int i=0; i<4; i++)
  {
    initializeBirds(&birds[i], -70, -63, 0, 0);

    current = 0 ; // Insert loop here 

    cout << birds[i].x << endl;
    cout << birds[i].y << endl;
    cout << birds[i].radius << endl;
    cout << birds[i].angle << endl;
  }
}

int main (int argc, char** argv)
{
  int width = 700;
  int height = 700;

  GLFWwindow* window = initGLFW(width, height);

  initGL (window, width, height);

  double last_update_time = glfwGetTime(), current_time;

/* Create And initialize the birds */

  makeBirds();
    /* Draw in loop */
  while (!glfwWindowShouldClose(window)) 
  {

        // OpenGL Draw commands
    draw();

        // Swap Frame Buffer in double buffering
    glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
    glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) 
        { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
          last_update_time = current_time;
        }
      }
      // SOIL_free_image_data( ht_map );

      glfwTerminate();
      exit(EXIT_SUCCESS);
    }



    GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

  // Create the shaders
      GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
      GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
      std::string VertexShaderCode;
      std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
      if(VertexShaderStream.is_open())
      {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
          VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
      }

  // Read the Fragment Shader code from the file
      std::string FragmentShaderCode;
      std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
      if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
          FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
      }

      GLint Result = GL_FALSE;
      int InfoLogLength;

  // Compile Vertex Shader
      printf("Compiling shader : %s\n", vertex_file_path);
      char const * VertexSourcePointer = VertexShaderCode.c_str();
      glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
      glCompileShader(VertexShaderID);

  // Check Vertex Shader
      glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      std::vector<char> VertexShaderErrorMessage(InfoLogLength);
      glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
      fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

  // Compile Fragment Shader
      printf("Compiling shader : %s\n", fragment_file_path);
      char const * FragmentSourcePointer = FragmentShaderCode.c_str();
      glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
      glCompileShader(FragmentShaderID);

  // Check Fragment Shader
      glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
      glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
      fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

  // Link the program
      fprintf(stdout, "Linking program\n");
      GLuint ProgramID = glCreateProgram();
      glAttachShader(ProgramID, VertexShaderID);
      glAttachShader(ProgramID, FragmentShaderID);
      glLinkProgram(ProgramID);

  // Check the program
      glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
      glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
      glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
      fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

      glDeleteShader(VertexShaderID);
      glDeleteShader(FragmentShaderID);

      return ProgramID;
    }
    void quit(GLFWwindow *window)
    {
      glfwDestroyWindow(window);
      glfwTerminate();
      exit(EXIT_SUCCESS);
    }
    void draw3DObject (struct VAO* vao)
    {
    // Change the Fill Mode for this object
      glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
      glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
      glEnableVertexAttribArray(0);
    // Bind the VBO to use
      glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
      glEnableVertexAttribArray(1);
    // Bind the VBO to use
      glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
  }
