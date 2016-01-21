#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

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

struct BIRD {
    GLfloat xi,yi,xspeed,yspeed,time;
    VAO *birdshape,*mouth,*lefteye,*righteye;
    bool flag,turn,has_collided;
};
typedef struct BIRD BIRD;

struct POWERBAR {
    GLfloat angle , length;
    VAO *bar;
};
typedef struct POWERBAR POWERBAR;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
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

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
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

/**************************
 * Customizable functions *
 **************************/

BIRD birds[6];
POWERBAR powerbar;
bool is_it_time = true;
BIRD create_angrybirds( BIRD bird,float initx,float inity)
{
  
  bird.flag = false;
  bird.time=0;
  bird.turn = false;
  bird.has_collided = false;
  bird.xi =initx;
  bird.yi =inity;
  bird.xspeed =0;
  bird.yspeed =0;
  int num_segments = 360 , k=0,j=0;
  float r = 0.24;
  float theta = 2 * 3.1415926 / float(num_segments); 
  float c = cosf(theta);
  float s = sinf(theta);
  float t;
  float cx =0,cy=0;

  float x = r;
  float y = 0; 
  GLfloat vertex_buffer_data1 [1080]; 
  GLfloat color_buffer_data1 [1080]; 
  for(int i = 0; i < num_segments; i++) 
  { 
    float yo = x+cx;
    float oy = y+cy;
    vertex_buffer_data1[k++] = yo;
    vertex_buffer_data1[k++] = oy;
    vertex_buffer_data1[k++] = 0;
    color_buffer_data1[j++]=1;
    color_buffer_data1[j++]=0;
    color_buffer_data1[j++]=0;

    t = x;
    x = c * x - s * y;
    y = s * t + c * y;
  }

  bird.birdshape = create3DObject(GL_TRIANGLE_FAN,360,vertex_buffer_data1,color_buffer_data1,GL_FILL) ;
  
  GLfloat vertex_buffer_data2 [6]; 
  GLfloat color_buffer_data2 [6];
  vertex_buffer_data2[0] = 0.05; 
  vertex_buffer_data2[1] = -0.1;
  vertex_buffer_data2[2] = 0;
  vertex_buffer_data2[3] = -0.05;
  vertex_buffer_data2[4] = -0.1;
  vertex_buffer_data2[5] = 0;
  color_buffer_data2 [0] =0; 
  color_buffer_data2 [1] =0; 
  color_buffer_data2 [2] =0; 
  color_buffer_data2 [3] =0;
  color_buffer_data2 [4] =0;
  color_buffer_data2 [5] =0;
  bird.mouth =  create3DObject(GL_LINES,2,vertex_buffer_data2,color_buffer_data2,GL_FILL) ;

  GLfloat vertex_buffer_data3 [9]; 
  GLfloat color_buffer_data3 [9];
  vertex_buffer_data3[0] = -0.15; 
  vertex_buffer_data3[1] = 0.1;
  vertex_buffer_data3[2] = 0;
  vertex_buffer_data3[3] = 0;
  vertex_buffer_data3[4] = 0.1;
  vertex_buffer_data3[5] = 0;
  vertex_buffer_data3[6] = -0.075;
  vertex_buffer_data3[7] = 0;
  vertex_buffer_data3[8] = 0;
  color_buffer_data3 [0] =0; 
  color_buffer_data3 [1] =0; 
  color_buffer_data3 [2] =0; 
  color_buffer_data3 [3] =0;
  color_buffer_data3 [4] =0;
  color_buffer_data3 [5] =0;
  color_buffer_data3 [6] =0;
  color_buffer_data3 [7] =0;
  color_buffer_data3 [8] =0;
  bird.lefteye =  create3DObject(GL_TRIANGLES,3,vertex_buffer_data3,color_buffer_data3,GL_FILL) ;

  GLfloat vertex_buffer_data4 [9]; 
  GLfloat color_buffer_data4 [9];
  vertex_buffer_data4[0] = 0.15; 
  vertex_buffer_data4[1] = 0.1;
  vertex_buffer_data4[2] = 0;
  vertex_buffer_data4[3] = 0;
  vertex_buffer_data4[4] = 0.1;
  vertex_buffer_data4[5] = 0;
  vertex_buffer_data4[6] = 0.075;
  vertex_buffer_data4[7] = 0;
  vertex_buffer_data4[8] = 0;
  color_buffer_data4 [0] =0; 
  color_buffer_data4 [1] =0; 
  color_buffer_data4 [2] =0; 
  color_buffer_data4 [3] =0;
  color_buffer_data4 [4] =0;
  color_buffer_data4 [5] =0;
  color_buffer_data4 [6] =0;
  color_buffer_data4 [7] =0;
  color_buffer_data4 [8] =0;
  bird.righteye =  create3DObject(GL_TRIANGLES,3,vertex_buffer_data4,color_buffer_data4,GL_FILL) ;
  return bird;
}
void draw_angrybird(BIRD bird,glm::mat4 MVP,glm::mat4 VP)
{
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatebird = glm::translate (glm::vec3(bird.xi, bird.yi, 0));
  //glm::mat4 scalebird = glm::scale (glm::vec3(1.2,1.2,0));
  Matrices.model *= translatebird;
  //Matrices.model *= scalebird; 
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(bird.birdshape);
  Matrices.model = glm::mat4(1.0f);
  draw3DObject(bird.mouth);
  Matrices.model = glm::mat4(1.0f);
  draw3DObject(bird.lefteye);
  Matrices.model = glm::mat4(1.0f);
  draw3DObject(bird.righteye);
} 


BIRD collisionground(BIRD bird)
{
  glm::vec2 center(bird.xi,bird.yi);
  glm::vec2 particle_half_extents(16 / 2, 1 / 2);
  glm::vec2 particle_center(0,-2.6);
    // Get difference vector between both centers
  glm::vec2 difference = center - particle_center;
  glm::vec2 clamped = glm::clamp(difference, -particle_half_extents, particle_half_extents);
    // Add clamped value to AABB_center and we get the value of box closest to circle
  glm::vec2 closest = particle_center + clamped;
    // Retrieve vector between center circle and closest point AABB and check if length <= radius
  difference = closest - center;
  if( glm::length(difference) < 0.24)
  {
    bird.yi = -2.46;
    bird.yspeed = -1*(bird.yspeed/2);
    if(bird.yspeed <= 0)
      bird.yspeed=0;
    bird.xspeed = bird.xspeed/2;
    bird.has_collided = true;
    //cout << "collided land" << endl;
  }

  else
    bird.has_collided = false;  
  return bird;
}

BIRD flight(BIRD bird)
{
   
  bird.time += 0.005f;
  bird.yspeed = bird.yspeed - (bird.time)*(0.08);
  bird.xi += bird.xspeed;
  bird.yi += bird.yspeed ;

  return bird ;
 
}

BIRD changeangle(BIRD bird,float power=0.3)
{
    bird.yspeed = (power/9)*sin(DEG2RAD(powerbar.angle));
    bird.xspeed = (power/9)*cos(DEG2RAD(powerbar.angle));
    return bird;
}

BIRD move_next_bird(BIRD bird)
{
    if(bird.xi <= -5.3)
      bird.xi += 0.1;
    if(bird.xi >= -5.3 && bird.yi <= -1.2)
      bird.yi += 0.1;
    if (bird.xi >= -5.3 && bird.yi >= -1.2)
    {
      is_it_time = true;
    }
    return bird;
}
  

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float triangle_x = 0,triangle_y = 0,triangle_z = 0;



/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_D:
                powerbar.angle-=1;
                break;
            case GLFW_KEY_A:
                powerbar.angle+=1;
                break;
            case GLFW_KEY_W:
                powerbar.length+=0.1;
                break;
            case GLFW_KEY_S:
                powerbar.length-=0.1;
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_SPACE:
                if(is_it_time == true)
                {
                  for(int i=0;i<6;i++)
                  {
                    if (birds[i].turn == true)
                    {
                      is_it_time = false;
                      birds[i]=changeangle(birds[i],powerbar.length);
                      birds[i].flag = true;
                      birds[i].turn = false;
                      if(i!=5)
                      {
                        birds[i+1].turn=true;
                        
                      }
                      break;
                    }
                  }
                }
                break;
            
            default:
                break;
        }
    }

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_D:
                powerbar.angle-=1;
                break;
            case GLFW_KEY_A:
                powerbar.angle+=1;
                break;
            case GLFW_KEY_W:
                powerbar.length+=0.1;
                break;
            case GLFW_KEY_S:
                powerbar.length-=0.1;
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
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-8.0f, 8.0f, -3.5f, 3.5f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle, *circle ,*ground ,*platform ,*catapult1,*catapult2,*catapult3;



void createPowerbar()
{
  powerbar.angle = 45;
  powerbar.length = 1;
  GLfloat vertex_buffer_data[]={
    0,0,0,
    1,0,0
  };
  GLfloat color_buffer_data[]={
    0,0,0,
    0,0,0
  };
  powerbar.bar = create3DObject(GL_LINES,2,vertex_buffer_data,color_buffer_data,GL_FILL);

}

void createCatapult()
{
  static const GLfloat vertex_buffer_data1 [] = {
    -0.05,-1,0, // vertex 1
    0.05,-1,0, // vertex 2
    0.05, 1,0, // vertex 3

    0.05, 1,0, // vertex 3
    -0.05, 1,0, // vertex 4
    -0.05,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data1 [] = {
    0.42,0.28,0.11, // color 1
    0.42,0.28,0.11, // color 2
    0.42,0.28,0.11, // color 3

    0.42,0.28,0.11, // color 3
    0.42,0.28,0.11, // color 4
    0.42,0.28,0.11,  // color 1
  };
  catapult1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data1, color_buffer_data1, GL_FILL);

  static const GLfloat vertex_buffer_data2 [] = {
    -0.05,-0.5,0, // vertex 1
    0.05,-0.5,0, // vertex 2
    0.05, 0.5,0, // vertex 3

    0.05, 0.5,0, // vertex 3
    -0.05, 0.5,0, // vertex 4
    -0.05,-0.5,0  // vertex 1
  };

  static const GLfloat color_buffer_data2 [] = {
    0.42,0.28,0.11, // color 1
    0.42,0.28,0.11, // color 2
    0.42,0.28,0.11, // color 3

    0.42,0.28,0.11, // color 3
    0.42,0.28,0.11, // color 4
    0.42,0.28,0.11,  // color 1
  };

  catapult2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data2, color_buffer_data2, GL_FILL);
  
  static const GLfloat vertex_buffer_data3 [] = {
    -0.05,-0.5,0, // vertex 1
    0.05,-0.5,0, // vertex 2
    0.05, 0.5,0, // vertex 3

    0.05, 0.5,0, // vertex 3
    -0.05, 0.5,0, // vertex 4
    -0.05,-0.5,0  // vertex 1
  };

  static const GLfloat color_buffer_data3 [] = {
    0.42,0.28,0.11, // color 1
    0.42,0.28,0.11, // color 2
    0.42,0.28,0.11, // color 3

    0.42,0.28,0.11, // color 3
    0.42,0.28,0.11, // color 4
    0.42,0.28,0.11,  // color 1
  };

  catapult3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data3, color_buffer_data3, GL_FILL);
  
}

void createGround ()
{
  static const GLfloat vertex_buffer_data [] = {
    -8,-0.5,0, // vertex 1
    8,-0.5,0, // vertex 2
    8, 0.5,0, // vertex 3

    8, 0.5,0, // vertex 3
    -8, 0.5,0, // vertex 4
    -8,-0.5,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.196078,0.5,0.196078, 
    0.196078,0.5,0.196078, 
    0.196078,0.5,0.196078, 
    0.196078,0.5,0.196078, 
    0.196078,0.5,0.196078, 
    0.196078,0.5,0.196078, 
  };

  
  // create3DObject creates and returns a handle to a VAO that can be used later
  ground = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}


// Creates the triangle object used in this sample code
void createTriangle ()
{
  
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

   triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  
  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;



/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  

  /* Render your scene */


  
  /* Rendering the Catapult*/
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatecatapult1 = glm::translate (glm::vec3(-5.22,-2.1,0));
  glm::mat4 scalecatapult1 = glm::scale (glm::vec3(1,0.6,0));
  glm::mat4 transformcatapult1 = translatecatapult1*scalecatapult1;
  Matrices.model *= transformcatapult1;
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(catapult1);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatecatapult2 = glm::translate (glm::vec3(-5.47,-1.25,0));
  glm::mat4 scalecatapult2 = glm::scale (glm::vec3(1,0.8,0));
  glm::mat4 rotatecatapult2 = glm::rotate((float)(DEG2RAD(45)), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 transformcatapult2 = translatecatapult2*rotatecatapult2*scalecatapult2;
  Matrices.model *= transformcatapult2;
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(catapult2); 

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatecatapult3 = glm::translate (glm::vec3(-4.99,-1.25,0));
  glm::mat4 scalecatapult3 = glm::scale (glm::vec3(1,0.8,0));
  glm::mat4 rotatecatapult3 = glm::rotate((float)(DEG2RAD(-45)), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 transformcatapult3 = translatecatapult3*rotatecatapult3*scalecatapult3;
  Matrices.model *= transformcatapult3;
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(catapult3); 


  /* Rendering the ground */
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateground = glm::translate (glm::vec3(0, -3.2, 0)); // glTranslatef
  glm::mat4 groundTransform = translateground;
  Matrices.model *= groundTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(ground); 
  
  /* Rendering the powerbar */
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatebar = glm::translate (glm::vec3(-5, -1, 0));
  glm::mat4 rotatebar = glm::rotate((float)(powerbar.angle*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 scalebar = glm::scale (glm::vec3(powerbar.length,1, 0)); 
  glm::mat4 transformbar = translatebar*rotatebar*scalebar;
  Matrices.model *= transformbar;
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(powerbar.bar); 

  /* Rendering angrybirds */

  int lolo;
  
  for(lolo=0;lolo<6;lolo++)
  {
      if (birds[lolo].flag==true)
      {
        is_it_time = false;
        birds[lolo] = collisionground(birds[lolo]);
        birds[lolo] = flight(birds[lolo]);
        birds[lolo+1] = move_next_bird(birds[lolo+1]);
      }
     draw_angrybird(birds[lolo],MVP,VP);
  }
  
  

   /* Rendering triangle 
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateTriangle = glm::translate (glm::vec3(triangle_x, triangle_y, triangle_z)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/50.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  //draw3DObject(triangle);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  
  glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  //draw3DObject(rectangle);
  */

  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
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

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	
  //Create the models
  int i;
  float xpos=-5.6,ypos=-2.5;
  birds[0] = create_angrybirds(birds[0],-5.2,-1.1);
  birds[0].turn=true;
  for(i=1;i<6;i++)
  {
    birds[i] = create_angrybirds(birds[i],xpos,ypos);
    xpos -=0.5;
  }
  createPowerbar();
  createCatapult();
	createGround();
  createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.74902,0.847059, 1.947059,0); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1600;
	int height = 700;

  GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
       
        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
} 