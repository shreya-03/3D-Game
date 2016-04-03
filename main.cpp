#include <iostream>
#include <cmath>
#include<stdio.h>
#include <fstream>
#include <vector>
#include<stdlib.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include<unistd.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include<time.h>
#include<math.h>
using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;
typedef struct pit
{
	float x;
	float y;
	float z;
}pit;
typedef struct obstacle
{
	float x;
	float y;
	float z;
}obstacle;
pit pits[10];
obstacle obstacles[8];
VAO * objects[512];
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

// Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
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
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);
    glBindVertexArray (vao->VertexArrayID);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}
float end_t=0;
float zoom=1;
int score=0;
float player_init_x=-3.7;
float player_init_z=3.7;
float player_init_down_y=4.3;
float player_init_up_y=4.8;
float camera_angle=45;
float tile_x=-3.5,tile_y=3.5,tile_z=-2.5;
void keyboardDown(unsigned char key, int x, int y)
{
    switch (key) {
        case 'Q':
        case 'q':
        case 27: //ESC
            exit (0);
        default:
            break;
    }
}
VAO *ball;
//VAO *objects[];
bool flag=false;
bool zoom_flag=false;
bool pan_flag=false;
float time_flight=0;
bool advcamera_flag=false,followcamera_flag=false;
float camera_x=0,camera_y=15,camera_z=6;
float target_x=0,target_y=0,target_z=0.1;
int counter=0;
int level=1,life=3;
float player_speed=1;
bool flag_jump_left=false,flag_life=true;
bool flag_jump_right=false,flag_jump_up=false,flag_jump_down=false,flag_up=false;
float counter_right=0,counter_left=0,counter_up=0,counter_down=0;
float obstacle_x,obstacle_y,obstacle_z;
//float angle=0.0,lx=0.0f,lz=-1.0f,x=0.0f,z=5.0f;
/* Executed when a regular key is released */
bool check_pit(float player_x,float player_z);
bool check_obstacle(float player_x,float player_y,float player_z);
void set_target(float x,float y,float z)
{
	target_x=x;
	target_y=y;
	target_z=z;
}
void keyboardUp (unsigned char key, int x, int y)
{
    flag=false;
    switch (key) {
        case 'a':
	case 'A':
    	    if(glutGetModifiers() & GLUT_ACTIVE_SHIFT)
	    {	
//		    cout << "entered" << endl;
		    flag_jump_left=true;
		    flag_jump_right=false;
		    flag_jump_up=false;
		    flag_jump_down=false;
		    counter_left=0;
	    }
//		    cout << "entered" << endl;
	    if(abs(player_init_x-tile_x)<=0.8 && abs(player_init_down_y-tile_y)<=0.8 && abs(player_init_z-tile_z)<=0.8)
	    	player_init_x+=0.1;
	    if(flag_up==true && abs(player_init_x-obstacle_x)<0.5)
	    {
		    player_init_x-=player_speed*0.1;
		    player_init_down_y=5.3;
		    player_init_up_y=5.8;
	    }
	    else if(flag_up==true && abs(player_init_x-obstacle_x)>=0.5)
	    {
		    flag_up=false;
		    player_init_x-=0.3;
		    player_init_down_y=4.3;
		    player_init_up_y=4.8;
	    }
	    else if(player_init_down_y==5.3 && flag_up==false && abs(player_init_x-obstacle_x)<0.5) 
	    {
		    if(counter%25<=23)
		    {
			    player_init_x-=player_speed*0.1;
		    	    player_init_down_y=5.3;
		            player_init_up_y=5.8;
			    flag_up=true;
		    }
		    else
		    {
			    player_init_x-=0.4;
			    player_init_down_y=4.3;
			    player_init_up_y=4.8;
		    }
	    }
	    else if(player_init_down_y==5.3 && flag_up==false && abs(player_init_x-obstacle_x)>=0.5)
	    {
		    player_init_x-=0.3;
		    player_init_down_y=4.3;
		    player_init_up_y=4.8;
	    }
	    else if(!check_obstacle(player_init_x,player_init_down_y,player_init_z) && !check_pit(player_init_x,player_init_z))
	    {	    
	//	    cout << "not obstacle" << endl;
		    player_init_x-=player_speed*0.1;
		    cout << player_init_x << player_init_z << endl;
		    if(player_init_x>=3.7 && player_init_z<=-3.7 && flag_life==true)
		    {
			    level++;
			    cout << "reached level " << level << endl;
			    cout << "score of prev level " << score <<endl;
			    player_init_x=-3.7;
			    player_init_down_y=4.3;
			    player_init_up_y=4.8;
			    player_init_z=3.7;
			    counter=23;
		    }

	    }
	    else if(check_obstacle(player_init_x,player_init_down_y,player_init_z))
	    {	    
		    score-=5;
		    player_init_x+=0.1;
	    }
	    else if(check_pit(player_init_x,player_init_z) && !check_obstacle(player_init_x,player_init_down_y,player_init_z))
	    {
		cout << "entered" << endl;
		if(player_init_down_y==4.3 && flag_life==true)
		{
			score-=10;
			player_init_x-=0.4;
    			player_init_down_y-=1;
			player_init_up_y-=1;
			life-=1;
			if(life==0)
				flag_life=false;
		}
		else if(player_init_down_y<4.3 && flag_life==true)
		{
			player_init_x=-3.7;
			player_init_down_y=4.3;
			player_init_up_y=4.8;
			player_init_z=3.7;
		}
	    }
	     if(advcamera_flag==true)
	    {
		camera_x=player_init_x-0.2;
		camera_y=player_init_up_y;
		camera_z=player_init_z;
		set_target(-6,4,player_init_z-2);
	    }
	    else if(followcamera_flag==true)
	   {
		camera_x=player_init_x+2.5;
		camera_y=player_init_up_y+2;
		camera_z=player_init_z;
		set_target(-6,4,player_init_z-2);
	   }
	   counter++;
	    break;
	case 'd':
	case 'D':
	    if(glutGetModifiers() & GLUT_ACTIVE_SHIFT)
	    {	    
	//	    cout << "entered" << endl;
		    flag_jump_right=true;
		    flag_jump_down=false;
		    flag_jump_up=false;
		    flag_jump_left=false;
		    counter_right=0;
	    }
	    if(flag_up==true && abs(player_init_x-obstacle_x)<0.5)
	    {
		    player_init_x+=player_speed*0.1;
		    player_init_down_y=5.3;
		    player_init_up_y=5.8;
	    }
	    else if(flag_up==true && abs(player_init_x-obstacle_x)>=0.5)
	    {
		    flag_up=false;
		    player_init_down_y=4.3;
		    player_init_up_y=4.8;
		    player_init_x+=0.3;
	    }
	    else if(flag_up==false && player_init_down_y==5.3 && abs(player_init_x-obstacle_x)<0.5)
	    {
	    	   if(counter%25<=23)
		   {
		    	player_init_x+=player_speed*0.1;
		   	player_init_down_y=5.3;
		   	player_init_up_y=5.8;
		   }
		   else
		   {
			   player_init_x+=0.4;
			   player_init_down_y=4.3;
			   player_init_up_y=4.8;
		   }
	    }
	    else if(flag_up==false && player_init_down_y==5.3 && abs(player_init_x-obstacle_x)>=0.5)
	    {
		    player_init_x+=0.3;
		    player_init_down_y=4.3;
		    player_init_up_y=4.8;
	    }
	    else if(!check_obstacle(player_init_x,player_init_down_y,player_init_z) && !check_pit(player_init_x,player_init_z))
	    {	    
	//	    co	ut << "not obstacle" << endl;
		    player_init_x+=player_speed*0.1;
		    cout << player_init_x << player_init_z << endl;
		    if(player_init_x>=3.7 && player_init_z<=-3.7 && flag_life==true)
		    {
			    level++;
			    player_init_x=-3.7;
			    player_init_down_y=4.3;
			    player_init_up_y=4.8;
			    player_init_z=3.7;
			    cout << "reached level " << level<< endl;
			    cout << "score of prev level " << score << endl;
			    counter=23;
		    }
	    }
	    else if(check_obstacle(player_init_x,player_init_down_y,player_init_z))
	    {	    
//		    cout << "obstacle" << endl;
		    score-=5;
		    player_init_x-=0.1;
	    }
	    else if(check_pit(player_init_x,player_init_z) && !check_obstacle(player_init_x,player_init_down_y,player_init_z))
	    {
		    cout << "entered" << endl;
		    if(player_init_down_y==4.3 && flag_life==true)
		    {
			score-=10;
		    	player_init_x+=0.4;
		    	player_init_down_y-=1;
		    	player_init_up_y-=1;
			life-=1;
			if(life==0)
				flag_life=false;
		    }
		    else if(player_init_down_y<4.3 && flag_life==true)
		    {
			    player_init_x=-3.7;
			    player_init_down_y=4.3;
			    player_init_up_y=4.8;
			    player_init_z=3.7;
		    }
	     }
	     if(advcamera_flag==true)
	     {
		camera_x=player_init_x+0.2;
		camera_y=player_init_up_y;
		camera_z=player_init_z;
		set_target(6,4,player_init_z-2);
	     }
	     else if(followcamera_flag==true)
	    {
		camera_x=player_init_x-2.5;
	        camera_y=player_init_up_y+2;
		camera_z=player_init_z;
	        set_target(6,4,player_init_z-2);
	    }
	    counter++;
	    break;
	case 'w':
	case 'W':
	    if(glutGetModifiers() & GLUT_ACTIVE_SHIFT)
	    {	    
		    flag_jump_up=true;
		    flag_jump_right=false;
		    flag_jump_left=false;
		    flag_jump_down=false;
		    counter_up=0;
	    }
	    if(abs(player_init_x-tile_x)<=0.8 && abs(player_init_down_y-tile_y)<=0.8 && abs(player_init_z-tile_z)<=0.8)
	    	player_init_z+=0.1;
	    if(flag_up==true && abs(player_init_z-obstacle_z)<0.5)
	    {
		    player_init_z-=player_speed*0.1;
		    player_init_down_y=5.3;
		    player_init_up_y=5.8;
	    }
	    else if(flag_up==true && abs(player_init_z-obstacle_z)>=0.5)
	    {
		    flag_up=false;
		    player_init_z-=0.3;
		    player_init_down_y=4.3;
		    player_init_up_y=4.8;
	    }
	    else if(flag_up==false && player_init_down_y==5.3 && abs(player_init_z-obstacle_z)<0.5) 
	    {
	    	   if(counter%25<=23)
		   {
		    	player_init_z-=player_speed*0.1;
		   	player_init_down_y=5.3;
		   	player_init_up_y=5.8;
		   }
		   else
		   {
			   player_init_z-=0.4;
			   player_init_down_y=4.3;
			   player_init_up_y=4.8;
		   }
	    }
	    else if(flag_up==false && player_init_down_y==5.3 && abs(player_init_z-obstacle_z)>=0.5)
	    {
		    player_init_z-=0.3;
		    player_init_down_y=4.3;
		    player_init_up_y=4.8;
	    }
	    else if(!check_obstacle(player_init_x,player_init_down_y,player_init_z) && !check_pit(player_init_x,player_init_z))
	    {	    
//		    cout << "not obstacle in up" << endl;
		    player_init_z-=player_speed*0.1;
		    cout << player_init_x << player_init_z << endl;
		    if(player_init_x>=3.7 && player_init_z<=-3.7 && flag_life==true)
		    {
			    level++;
			    cout << "reached level " << level << endl;
			    cout << "score of prev level " << score << endl;
			    player_init_x=-3.7;
			    player_init_z=3.7;
			    player_init_down_y=4.3;
			    player_init_up_y=4.8;
			    counter=23;
		    }
	    }
	    else if(check_obstacle(player_init_x,player_init_down_y,player_init_z))
	    {	    
//		    cout << "obstacle in up" << endl;
		    score-=5;
		    player_init_z+=0.1;
	    }
	    else if(check_pit(player_init_x,player_init_z) && !check_obstacle(player_init_x,player_init_down_y,player_init_z))
	    {
		    cout << "entered" << endl;
		    if(flag_life==true && player_init_down_y==4.3)
		    {
		    	player_init_z-=0.4;
		    	player_init_down_y-=1;
		    	player_init_up_y-=1;
			score-=10;
			life-=1;
			if(life==0)
				flag_life=false;
		    }
		    else if(flag_life==true && player_init_down_y<4.3)
		    {
			    player_init_x=-3.7;
			    player_init_down_y=4.3;
			    player_init_up_y=4.8;
			    player_init_z=3.7;
		    }
	    }
	     if(advcamera_flag==true)
	     {
		camera_x=player_init_x;
		camera_y=player_init_up_y;
		camera_z=player_init_z-0.2;
		set_target(player_init_x-2,4,-6);
	     }
	   else if(followcamera_flag==true)
	   {
		camera_x=player_init_x;
		camera_y=player_init_up_y+2;
	        camera_z=player_init_z+2.5;
		set_target(player_init_x-2,4,-6);
           }
	    counter++;
	    break;
	case 's':
	case 'S':
	    if(glutGetModifiers() & GLUT_ACTIVE_SHIFT)
	    {	    
		    flag_jump_down=true;
		    flag_jump_left=false;
		    flag_jump_right=false;
		    flag_jump_up=false;
		    counter_down=0;
	    }
	    if(abs(player_init_x-tile_x)<=0.8 && abs(player_init_down_y-tile_y)<=0.8 && abs(player_init_z-tile_z)<=0.8)
		    player_init_z-=0.1;
	    if(flag_up==true && abs(player_init_z-obstacle_z)<0.5)
	    {
		    player_init_z+=player_speed*0.1;
		    player_init_down_y=5.3;
		    player_init_up_y=5.8;
	    }
	    else if(flag_up==true && abs(player_init_z-obstacle_z)>=0.5)
	    {
		    flag_up=false;
		    player_init_z+=0.3;
		    player_init_down_y=4.3;
		    player_init_up_y=4.8;
	    }
	    else if(flag_up==false && player_init_down_y==5.3 && abs(player_init_z-obstacle_z)<0.5)
	    {
	    	    if(counter%25<=23)
		    {
			player_init_z+=player_speed*0.1;
		    	player_init_down_y=5.3;
		    	player_init_up_y=5.8;
		    }
		    else
		    {
			    player_init_z+=0.4;
			    player_init_down_y=4.3;
			    player_init_up_y=4.8;
		    }
	    }
	    else if(flag_up==false && player_init_down_y==5.3 && abs(player_init_z-obstacle_z)>=0.5)
	    {
		    player_init_z+=0.3;
		    player_init_down_y=4.3;
		    player_init_up_y=4.8;
	    }
	    if(!check_obstacle(player_init_x,player_init_down_y,player_init_z) && !check_pit(player_init_x,player_init_z))
	    {	    
//		    cout << "no obstacle in down" << endl;
		    player_init_z+=player_speed*0.1;
	//	    cout << player_init_x << player_init_z << endl;
		    if(player_init_x>=3.7 && player_init_z<=-3.7 && flag_life==true)
		    {
			    level++;
			    player_init_x=-3.7;
			    player_init_down_y=4.3;
			    player_init_down_y=4.8;
			    player_init_z=3.7;
			    cout << "reached level " << level << endl;
			    cout << "score of prev level " << score << endl;
			    counter=23;
		    }
	    }
	    else if(check_obstacle(player_init_x,player_init_down_y,player_init_z))
	    {	    
//		    cout << "obstacle in down" << endl;
		    score-=5;
		    player_init_z-=0.1;
	    }
	    else if(check_pit(player_init_x,player_init_z) && !check_obstacle(player_init_x,player_init_down_y,player_init_z))
	    {
		    cout << "entered" << endl;
		    if(flag_life==true && player_init_down_y==4.3)
		    {
		    	player_init_z+=0.4;
		    	player_init_down_y-=1;
		   	player_init_up_y-=1;
			score-=10;
			life-=1;
			if(life==0)
				flag_life=false;
		    }
		    else if(flag_life==true && player_init_down_y<4.3)
		    {
			    player_init_x=-3.7;
			    player_init_down_y=4.3;
			    player_init_up_y=4.8;
			    player_init_z=3.7;
		    }
	    }
 		if(advcamera_flag==true)
		    {
			    camera_x=player_init_x;
			    camera_y=player_init_up_y+0.2;
			    camera_z=player_init_z+0.2;
			    set_target(player_init_x-2,4,6);
		    }
		    else if(followcamera_flag==true)
		    {
			    camera_x=player_init_x;
			    camera_y=player_init_up_y+2;
			    camera_z=player_init_z-2.5;
			    set_target(player_init_x-2,4,6);
		    }
	    counter++;
	    break;
	case 'e':
	    player_speed-=0.2;
	    break;
	case 'f':
	    player_speed+=0.2;
	    break;
//        case 
	default:
            break;
    }
}
float pan=0;
float eye_x=0,eye_y=0,eye_z=5;
float new_eye_x=0,new_eye_y=0,new_eye_z=5;
/* Executed when a special key is pressed */
void keyboardSpecialDown (int key, int x, int y)
{
	float fraction=0.1f;
	switch(key){
		case GLUT_KEY_LEFT:
			camera_x=player_init_x;
			camera_y=player_init_up_y;
			camera_z=player_init_z;
			camera_angle=45;
			advcamera_flag=true;
			break;
		case GLUT_KEY_RIGHT:
			camera_x=player_init_x;
			camera_y=player_init_up_y;
			camera_z=player_init_z;
			followcamera_flag=true;
			camera_angle=45;
			break;
		case GLUT_KEY_UP:
			camera_x=0;
			camera_y=15;
			camera_z=0;
			camera_angle=45;
			advcamera_flag=false;
			followcamera_flag=false;
			target_x=0;
			target_y=0;
			target_z=0.1;
			break;
		case GLUT_KEY_DOWN:
			camera_x=0;
			camera_y=12;
			camera_z=6;
			camera_angle=45;
			advcamera_flag=false;
			followcamera_flag=false;
			target_x=0;
			target_y=0;
			target_z=0.1;
			break;
	}

//	glutPostRedisplay();
}
//bool isDragging=0;
//float deltaAngle,xDragStart;
//char s[10];
//float speed_x;
//const int font=((int)GLUT_BITMAP_9_BY_15;
/* Executed when a special key is released */
/*void mouseMove (int x, int y)
{
	if(isDragging)
	{
		deltaAngle=(x-xDragStart)*0.0005;
		pan+=sin(deltaAngle);		
	}
}
void scrollMouse(int x,int y)
{
	if(isDragging)
		zoom+=float(y)/4;
	if(zoom<0)
		zoom=0;
}*/
/* Executed when a mouse button 'button' is put into state 'state'
 at screen position ('x', 'y')
 */
void mouseClick (int button, int state, int x, int y)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN)
	    {    
//		    pan+=0.1f;
//		    isDragging=1;
//		    xDragStart=x;
		    camera_y-=0.5;
		    camera_z-=0.5;
	    }
//	    else
//		    isDragging=0;
	break;
        case GLUT_RIGHT_BUTTON:
            if (state == GLUT_DOWN) {
//		pan-=0.1f;
//		isDragging=1;
//		xDragStart=x;
		camera_y+=0.5;
		camera_z+=0.5;
            }
//	    else
//		    isDragging=0;
            break;
        default:
            break;
    }
}

// Executed when the mouse moves to position ('x', 'y') 
float camera_rotation_angle=90;
void mouseMotion (int x, int y)
{
	//	if(isDragging!=1)
		camera_rotation_angle=atan((y-12)/(x-0))*180.0f/M_PI;
}

void reshapeWindow (int width, int height)
{
	GLfloat fov = 90.0f;
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);
//    Matrices.projection = glm::ortho(-6.0f, 6.0f, -6.0f, 6.0f, 0.1f, 500.0f);
}
VAO *Obstacle1,*Obstacle2,*Obstacle3,*Obstacle4,*player_down,*player_up,*Obstacles[8];
bool check_pit(float player_x,float player_z)
{
	bool flag=false;
	for(int i=0;i<10;i++)
	{
//		if(sqrt(((player_x-pits[i].x)*(player_x-pits[i].x))+((player_z-pits[i].z)*(player_z-pits[i].z)))<=0.53851648071)
		if(abs(player_x-pits[i].x)<=0.5 &&  abs(player_z-pits[i].z)<=0.5)
		{
			flag=true;
			break;
		}
	}
	if(flag==true)
		return true;
	else
		return false;
}
bool check_obstacle(float player_x,float player_y,float player_z)
{
	bool flag=false;
	for(int i=0;i<8;i++)
	{
//		cout << player_x-obstacles[i].x << endl;
		if(abs(player_x-obstacles[i].x)<=0.8 && abs(player_y-obstacles[i].y)<=0.8 && abs(player_z-obstacles[i].z)<=0.8)
		{
	//		cout << "entered func" << endl;
			flag=true;
			obstacle_x=obstacles[i].x;
			obstacle_y=obstacles[i].y;
			obstacle_z=obstacles[i].z;
			break;
		}
	}
	if(flag==true)
		return true;
	else
		return false;
}
void create3DWorld()
{
	for(int i=0;i<512;i++)
	{
		const GLfloat vertex_buffer_data[]={
			-0.5,-0.5,0.5,
			0.5,-0.5,0.5,
			0.5,0.5,0.5,

			0.5,0.5,0.5,
			-0.5,0.5,0.5,
			-0.5,-0.5,0.5,

			0.5,0.5,0.5,
			0.5,-0.5,0.5,
			0.5,-0.5,-0.5,

			0.5,-0.5,-0.5,
			0.5,0.5,-0.5,
			0.5,0.5,0.5,

			-0.5,-0.5,-0.5,
			0.5,-0.5,-0.5,
			0.5,0.5,-0.5,

			0.5,0.5,-0.5,
			-0.5,0.5,-0.5,
			-0.5,-0.5,-0.5,

			-0.5,0.5,-0.5,
			-0.5,-0.5,-0.5,
			-0.5,-0.5,0.5,

			-0.5,-0.5,0.5,
			-0.5,0.5,0.5,
			-0.5,0.5,-0.5,

			-0.5,0.5,-0.5,
			-0.5,0.5,0.5,
			0.5,0.5,0.5,

			0.5,0.5,0.5,
			0.5,0.5,-0.5,
			-0.5,0.5,-0.5,

			-0.5,-0.5,-0.5,
			-0.5,-0.5,0.5,
			0.5,-0.5,0.5,

			0.5,-0.5,0.5,
			0.5,-0.5,-0.5,
			-0.5,-0.5,-0.5,
		};
		const GLfloat color_buffer_data[]={
			0.69803921568,0.13333333333,0.13333333333,
			0.69803921568,0.13333333333,0.13333333333,
			0.69803921568,0.13333333333,0.13333333333,

			0.69803921568,0.13333333333,0.13333333333,
			0.69803921568,0.13333333333,0.13333333333,
			0.69803921568,0.13333333333,0.13333333333,

			1,0.38823529411,0.27843137254,
			1,0.38823529411,0.27843137254,
			1,0.38823529411,0.27843137254,

			1,0.38823529411,0.27843137254,
			1,0.38823529411,0.27843137254,
			1,0.38823529411,0.27843137254,

			0.69803921568,0.13333333333,0.13333333333,
			0.69803921568,0.13333333333,0.13333333333,
			0.69803921568,0.13333333333,0.13333333333,

			0.69803921568,0.13333333333,0.13333333333,
			0.69803921568,0.13333333333,0.13333333333,
			0.69803921568,0.13333333333,0.13333333333,

			1,0.38823529411,0.27843137254,
			1,0.38823529411,0.27843137254,
			1,0.38823529411,0.27843137254,

			1,0.38823529411,0.27843137254,
			1,0.38823529411,0.27843137254,
			1,0.38823529411,0.27843137254,

			1,0.49803921568,0.31372549019,
			1,0.49803921568,0.31372549019,
			1,0.49803921568,0.31372549019,

			1,0.49803921568,0.31372549019,
			1,0.49803921568,0.31372549019,
			1,0.49803921568,0.31372549019,

			1,0.49803921568,0.31372549019,
			1,0.49803921568,0.31372549019,
			1,0.49803921568,0.31372549019,

			1,0.49803921568,0.31372549019,
			1,0.49803921568,0.31372549019,
			1,0.49803921568,0.31372549019,

	};	
	objects[i]=create3DObject(GL_TRIANGLES,36,vertex_buffer_data,color_buffer_data,GL_FILL);
	}
}
VAO* createObstacle(float a,float red,float green,float blue)
{
	const GLfloat vertex_buffer_data[]={
			-a,-a,a,
			a,-a,a,
			a,a,a,

			a,a,a,
			-a,a,a,
			-a,-a,a,

			a,a,a,
			a,-a,a,
			a,-a,-a,

			a,-a,-a,
			a,a,-a,
			a,a,a,

			-a,-a,-a,
			a,-a,-a,
			a,a,-a,

			a,a,-a,
			-a,a,-a,
			-a,-a,-a,

			-a,a,-a,
			-a,-a,-a,
			-a,-a,a,

			-a,-a,a,
			-a,a,a,
			-a,a,-a,

			-a,a,-a,
			-a,a,a,
			a,a,a,

			a,a,a,
			a,a,-a,
			-a,a,-a,

			-a,-a,-a,
			-a,-a,a,
			a,-a,a,

			a,-a,a,
			a,-a,-a,
			-a,-a,-a,
		};
	return create3DObject(GL_TRIANGLES,36,vertex_buffer_data,red,green,blue,GL_FILL);
}	
//float camera_rotation_angle=90;
int x=0,y=0;
bool obstacle_flag=false;
bool flag_tile=false;
void draw3DWorld()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programID);
	Matrices.projection=glm::perspective(camera_angle,1.0f*800/600,1.0f,500.0f);
//	glm::vec3 eye (5*cos(camera_rotation_angle*M_PI/180.0f),0,5*sin(camera_rotation_angle*M_PI/180.0f));
	glm::vec3 eye(new_eye_x,new_eye_y,new_eye_z);
	glm::vec3 target(0,0,0);
	glm::vec3 up(0,1,0);
	Matrices.view=glm::lookAt(glm::vec3(camera_x,camera_y,camera_z),glm::vec3(target_x,target_y,target_z),glm::vec3(0,1,0));
	glm::mat4 VP=Matrices.projection*Matrices.view;
	glm::mat4 MVP;
	x=0,y=0;
	for(float i=-3.5;i<=3.5;i++)
	{
		for(float j=-3.5;j<=3.5;j++)
		{
			for(float k=-3.5;k<=3.5;k++)
			{
	//			cout<<i<<" "<<j<<" "<<k<<endl;
			//	cout<<"x: "<<x<<endl;
				if(x!=59 && x!=126 && x!=186 && x!=315 && x!=316 && x!=381 && x!=254 && x!=383 && x!=253 && x!=509)
				{
			//		cout << x << endl;
					if(x!=57)
					{
					Matrices.model=glm::mat4(1.0f);
					glm::mat4 translateCube=glm::translate(glm::vec3(i,j,k));
					Matrices.model*=translateCube;
					MVP=VP*Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
					draw3DObject(objects[x]);
					}
//					else
//						flag_tile=true;
				}
				else
				{
					pits[y].x=i;
					pits[y].y=j;
					pits[y].z=k;
					y++;
				}
				x++;
//				glutSwapBuffers();
			}
		}
	}
//	if(flag_tile==true)
//	{
		if(flag_tile==false)
		{	
			tile_y+=0.005;
			if(tile_y>=4.5)
				flag_tile=true;
		}
		else
		{	
			tile_y-=0.005;
			if(tile_y<=3.5)
				flag_tile=false;
		}
		Matrices.model=glm::mat4(1.0f);
		glm::mat4 translateCube=glm::translate(glm::vec3(tile_x,tile_y,tile_z));
		Matrices.model*=translateCube;
		MVP=VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(objects[57]);
//	}
	for(int i=0;i<8;i++)
	{
		Matrices.model=glm::mat4(1.0f);
		glm::mat4 translateOb=glm::translate(glm::vec3(obstacles[i].x,obstacles[i].y,obstacles[i].z));
		Matrices.model*=translateOb;
		MVP=VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(Obstacles[i]);
	}
        if(counter%25==24)	
	{
		if(obstacle_flag==false)
		{
			for(int i=0;i<8;i++)
			{	
				obstacles[i].x=i-3.5;
				obstacles[i].y=4.5;
				if(i!=0 && i!=7)
					obstacles[i].z=((64*(i+1))-(rand()%8+1))%8-3.5;
				else
					obstacles[i].z=((64*(i+1))-(rand()%8+1))%5-1.5;
				if(abs(player_init_x-obstacles[i].x)<=0.8 && abs(player_init_down_y-obstacles[i].y)<=0.8 && abs(player_init_z-obstacles[i].z)<=0.8)
				{
					player_init_down_y=5.3;
					player_init_up_y=5.8;
					obstacle_x=obstacles[i].x;
					obstacle_y=obstacles[i].y;
					obstacle_z=obstacles[i].z;
					flag_up=false;
				}
			}
			obstacle_flag=true;
		}
	}
	else
		obstacle_flag=false;

	if(flag_jump_left==true && flag_life==true)
	{
		sleep(0.001);
		if(check_obstacle(player_init_x,player_init_down_y,player_init_z) && !check_pit(player_init_x,player_init_z))
		{
/*			if(abs(player_init_x-obstacle_x)<=0.8)
			{
				player_init_down_y=4.3;
				player_init_up_y=4.8;
				player_init_x+=0.1;
			}*/
			score-=5;
			if(counter_left>=1.5)
			{	
				flag_up=true;
				player_init_down_y=5.3;
				player_init_up_y=5.8;
			}
			else if(counter_left<1.5)
			{
				player_init_down_y=4.3;
				player_init_up_y=4.8;
				player_init_x+=0.1;
			}
			flag_jump_left=false;
			counter_left=0;
		}
		else
		{
			if(counter_left<=1.5)
			{
				player_init_x-=0.1;
				player_init_down_y+=0.1;
				player_init_up_y+=0.1;
				counter_left+=0.1;
			}
			else
			{
//				cout << "entered" << endl;
				player_init_x-=0.1;
				player_init_down_y-=0.1;
				player_init_up_y-=0.1;
			}
			if(counter_left>1.5 && player_init_down_y==4.3 && !check_pit(player_init_x,player_init_z))
			{
				flag_jump_left=false;
				counter_left=0;
			}
			else if(counter_left>1.5 && player_init_down_y==4.3 && check_pit(player_init_x,player_init_z))
			{
				score-=10;
				life-=1;
				player_init_down_y=4.3;
				player_init_up_y=4.8;
				player_init_x=-3.7;
				player_init_z=3.7;
				if(life==0)
					flag_life=false;
				flag_jump_left=false;
				counter_left=0;
			}
		}

	Matrices.model=glm::mat4(1.0f);
	glm::mat4 translateplayerdown=glm::translate(glm::vec3(player_init_x,player_init_down_y,player_init_z));
	Matrices.model*=translateplayerdown;
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	draw3DObject(player_down);

	Matrices.model=glm::mat4(1.0f);
	glm::mat4 translateplayerup=glm::translate(glm::vec3(player_init_x,player_init_up_y,player_init_z));
	Matrices.model*=translateplayerup;
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	draw3DObject(player_up);
	}
	else if(flag_jump_right==true && flag_life==true)
	{
		sleep(0.001);
		if(check_obstacle(player_init_x,player_init_down_y,player_init_z) && !check_pit(player_init_x,player_init_z))
		{
/*			if(abs(player_init_x-obstacle_x)<=0.8)
			{
				player_init_down_y=4.3;
				player_init_up_y=4.8;
				player_init_x-=0.1;
			}*/
			score-=5;
			if(counter_right>=1.5)
			{	
				flag_up=true;
				player_init_down_y=5.3;
				player_init_up_y=5.8;
			}
			else if(counter_right<1.5)
			{
				player_init_down_y=4.3;
				player_init_up_y=4.8;
				player_init_x-=0.1;
			}	
			flag_jump_right=false;
			counter_right=0;
		}
		else
		{
			if(counter_right<=1.5)
			{
				player_init_x+=0.1;
				player_init_down_y+=0.1;
				player_init_up_y+=0.1;
				counter_right+=0.1;
			}
			else
			{
				cout << "entered" << endl;
				player_init_x+=0.1;
				player_init_up_y-=0.1;
				player_init_down_y-=0.1;
			}
		}
			if(counter_right>1.5 && player_init_down_y<=4.3 && !check_pit(player_init_x,player_init_z))
			{	
				flag_jump_right=false;
				counter_right=0;
			}
			else if(counter_right>1.5 && player_init_down_y<=4.3 && check_pit(player_init_x,player_init_z))
			{
				score-=10;
				player_init_x=-3.7;
				player_init_down_y=4.3;
				player_init_up_y=4.8;
				player_init_z=3.7;
				flag_jump_right=false;
				counter_right=0;
				life-=1;
				if(life==0)
					flag_life=false;
			}
	Matrices.model=glm::mat4(1.0f);
	glm::mat4 translateplayerdown=glm::translate(glm::vec3(player_init_x,player_init_down_y,player_init_z));
	Matrices.model*=translateplayerdown;
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	draw3DObject(player_down);

	Matrices.model=glm::mat4(1.0f);
	glm::mat4 translateplayerup=glm::translate(glm::vec3(player_init_x,player_init_up_y,player_init_z));
	Matrices.model*=translateplayerup;
	MVP=VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	draw3DObject(player_up);
	}
	if(flag_jump_up==true && flag_life==true)
	{
		sleep(0.001);
		if(check_obstacle(player_init_x,player_init_down_y,player_init_z) && !check_pit(player_init_x,player_init_z))
		{
			score-=5;
			if(counter_up>=1.5)
			{
				flag_up=true;
				player_init_down_y=5.3;
				player_init_up_y=5.8;
			}
			else if(counter_up<1.5)
			{
				player_init_down_y=4.3;
				player_init_up_y=4.8;
				player_init_z+=0.1;
			}
//			else if(abs(player_init_down_y-obstacle_y)<=0.8 && counter_up>=1.5)
//				flag_up=true;
			flag_jump_up=false;
			counter_up=0;
		}
		else
		{
			if(counter_up<=1.5)
			{
				player_init_z-=0.1;
				player_init_down_y+=0.1;
				player_init_up_y+=0.1;
				counter_up+=0.1;
			}
			else
			{
				player_init_z-=0.1;
				player_init_down_y-=0.1;
				player_init_up_y-=0.1;
			}
		}
		if(counter_up>1.5 && player_init_down_y<=4.3 && !check_pit(player_init_x,player_init_z))
		{
			flag_jump_up=false;
			counter_up=0;
		}
		else if(counter_right>1.5 && player_init_down_y<=4.3 && check_pit(player_init_x,player_init_z))
		{
			player_init_z=3.7;
			player_init_down_y=4.3;
			player_init_up_y=4.8;
			player_init_x=-3.7;
			score-=10;
			life-=1;
			if(life==0)
				flag_life=false;
			flag_jump_up=false;
			counter_up=0;
		}
		Matrices.model=glm::mat4(1.0f);
		glm::mat4 translateplayerdown=glm::translate(glm::vec3(player_init_x,player_init_down_y,player_init_z));
		Matrices.model*=translateplayerdown;
		MVP=VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(player_down);

		Matrices.model=glm::mat4(1.0f);
		glm::mat4 translateplayerup=glm::translate(glm::vec3(player_init_x,player_init_up_y,player_init_z));
		Matrices.model*=translateplayerup;
		MVP=VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(player_up);
	}
	if(flag_jump_down==true && flag_life==true)
	{
		sleep(0.001);
		if(check_obstacle(player_init_x,player_init_down_y,player_init_z) && !check_pit(player_init_x,player_init_z))
		{
			score-=5;
			if(counter_down>=1.5)
			{
				flag_up=true;
				player_init_down_y=5.3;
				player_init_up_y=5.8;
			}
			else if(counter_down<1.5)
			{
				player_init_down_y=4.3;
				player_init_up_y=4.8;
				player_init_z-=0.1;
			}
//			else if(abs(player_init_down_y-obstacle_y)<=0.8 && counter_down>=1.5)
//				flag_up=true;
			flag_jump_down=false;
			counter_down=0;
		}
		else
		{
			if(counter_down<=1.5)
			{
				player_init_z+=0.1;
				player_init_down_y+=0.1;
				player_init_up_y+=0.1;
				counter_down+=0.1;
			}
			else
			{
				player_init_z+=0.1;
				player_init_down_y-=0.1;
				player_init_up_y-=0.1;
			}
		}
		if(counter_down>1.5 && player_init_down_y<=4.3 && !check_pit(player_init_x,player_init_z))
		{
			flag_jump_down=false;
			counter_down=0;
		}
		else if(counter_down>1.5 && player_init_down_y<=4.3 && check_pit(player_init_x,player_init_z))
		{
			player_init_z=3.7;
			player_init_down_y=4.3;
			player_init_up_y=4.8;
			player_init_x=-3.7;
			score-=10;
			life-=1;
			if(life==0)
				flag_life=false;
			flag_jump_down=false;
			counter_down=0;
		}
		Matrices.model=glm::mat4(1.0f);
		glm::mat4 translateplayerdown=glm::translate(glm::vec3(player_init_x,player_init_down_y,player_init_z));
		Matrices.model*=translateplayerdown;
		MVP=VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(player_down);

		Matrices.model=glm::mat4(1.0f);
		glm::mat4 translateplayerup=glm::translate(glm::vec3(player_init_x,player_init_up_y,player_init_z));
		Matrices.model*=translateplayerup;
		MVP=VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(player_up);
	}
		

	if(!flag_jump_down && !flag_jump_up && !flag_jump_left && !flag_jump_right && flag_life)
	{
		Matrices.model=glm::mat4(1.0f);
		glm::mat4 translateplayerdown=glm::translate(glm::vec3(player_init_x,player_init_down_y,player_init_z));
		Matrices.model*=translateplayerdown;
		MVP=VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(player_down);

		Matrices.model=glm::mat4(1.0f);
		glm::mat4 translateplayerup=glm::translate(glm::vec3(player_init_x,player_init_up_y,player_init_z));
		Matrices.model*=translateplayerup;
		MVP=VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(player_up);
	}

	glutSwapBuffers();
}
/* Executed when the program is idle (no I/O activity) */
void idle () {
    draw3DWorld (); // drawing same scene
}


void initGLUT (int& argc, char** argv, int width, int height)
{
    glutInit (&argc, argv);

    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Angry Bird Game");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
        exit (1);
    }

    // register glut callbacks
    glutKeyboardFunc (keyboardDown);
    glutKeyboardUpFunc (keyboardUp);
 
    glutSpecialFunc (keyboardSpecialDown);
//    glutSpecialUpFunc (keyboardSpecialUp);

//    glutMotionFunc(scrollMouse);
    glutMouseFunc (mouseClick);
      glutMotionFunc (mouseMotion);
//    glutMotionFunc(mouseMove);
    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw3DWorld); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)   
    glutIgnoreKeyRepeat (true); // Ignore keys held down
}

/* Process menu option 'op' */
void menu(int op)
{
//    switch(op)
//    {
  //      case 'Q':
//        case 'q':
        if(player_init_x>=3.7 && player_init_z<=-3.7)    
	{	
		cout << score << endl;
		exit(0);
	}
    }


void addGLUTMenus ()
{
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
}


void initGL (int width, int height)
{
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (width, height);

	// Background color of the scene
	glClearColor (0.67843137254f, 1.0f,0.18431372549f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

//	createCube();
//	createObstacle();
//	createSphere(5.0f,0.0f,0.0f,0.0f);
	obstacles[0].x=-3.5;
	obstacles[0].y=4.5;
	obstacles[0].z=0.5;
	obstacles[1].x=-2.5;
	obstacles[1].y=4.5;
	obstacles[1].z=-1.5;
	obstacles[2].x=-1.5;
	obstacles[2].y=4.5;
	obstacles[2].z=1.5;
	obstacles[3].x=-0.5;
	obstacles[3].y=4.5;
	obstacles[3].z=-2.5;
	obstacles[4].x=0.5;
	obstacles[4].y=4.5;
	obstacles[4].z=-1.5;
	obstacles[5].x=1.5;
	obstacles[5].y=4.5;
	obstacles[5].z=1.5;
	obstacles[6].x=2.5;
	obstacles[6].y=4.5;
	obstacles[6].z=-0.5;
	obstacles[7].x=3.5;
	obstacles[7].y=4.5;
	obstacles[7].z=-2.5;
	create3DWorld();
	Obstacles[0]=createObstacle(0.5f,1,0.8431372549,0);
	Obstacles[1]=createObstacle(0.5f,0.82352941176,0.41176470588,0.11764705882);
	Obstacles[2]=createObstacle(0.5f,0.87058823529,0.72156862745,0.5294117647);
	Obstacles[3]=createObstacle(0.5f,0.85490196078,0.43921568627,0.83921568627);
	Obstacles[4]=createObstacle(0.5f,1,0.54901960784,0);
	Obstacles[5]=createObstacle(0.5f,0.50196078431,0.50196078431,0);
	Obstacles[6]=createObstacle(0.5f,0,0.54509803921,0.54509803921);
	Obstacles[7]=createObstacle(0.5f,0.11764705882,0.56470588235,1);
	player_down=createObstacle(0.3f,0,0,1);
	player_up=createObstacle(0.2f,1,0,1);
}

int main (int argc, char** argv)
{
	int width = 800;
	int height = 600;

    initGLUT (argc, argv, width, height);

    addGLUTMenus ();

	initGL (width, height);

	while(level<=3)
	{		
		cout << "level" << level<<endl;
		glutMainLoop ();
	}

    return 0;
}
