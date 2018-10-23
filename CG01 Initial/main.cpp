
//Some Windows Headers (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "maths_funcs.h" //Anton's math class
#include "teapot.h" // teapot mesh
#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>


//typedef double DWORD;


// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;
GLuint shaderProgramID;

unsigned int teapot_vao = 0;
int width = 800.0;
int height = 600.0;
GLuint loc1;
GLuint loc2;
double animationFactor, flipAnimationFactor, translateAnimX, translateAnimY = 0.0;
GLfloat rotatez = 0.0f;
bool animToggle = true;


// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS

std::string readShaderSource(const std::string& fileName)
{
	std::ifstream file(fileName.c_str()); 
	if(file.fail()) {
		cout << "error loading shader called " << fileName;
		cin.get();
		exit (1); 
	} 
	
	std::stringstream stream;
	stream << file.rdbuf();
	file.close();

	return stream.str();
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		cin.get();
        exit(0);
    }
	std::string outShader = readShaderSource(pShaderText);
	const char* pShaderSource = outShader.c_str();

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
    glCompileShader(ShaderObj);
    GLint success;
	// check for shader related errors using glGetShaderiv
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		cin.get();
        exit(1);
    }
	// Attach the compiled shader object to the program object
    glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
    shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

	// Create two shader objects, one for the vertex, and one for the fragment shader
    AddShader(shaderProgramID, "Shaders/simpleVertexShader.txt", GL_VERTEX_SHADER);
    AddShader(shaderProgramID, "Shaders/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };


	// After compiling all shader objects and attaching them to the program, we can finally link it
    glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
    glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
    glUseProgram(shaderProgramID);

	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

//Creates the matrix for orthogonal projection
//This uses column major
mat4 ortho(float left, float right, float bottom, float top, float nearp, float farp) {

	float rl = 2.0f / (right - left);
	float tb = 2.0f / (top - bottom);
	float fn = -2.0f / (farp - nearp);

	float xlr = -1 * ((left + right) / (right - left));
	float ytb = -1 * ((top + bottom) / (top - bottom));
	float zfn = -1 * ((farp + nearp) / (farp - nearp));

	mat4 m = identity_mat4();
	m.m[0] = rl;
	m.m[5] = tb;
	m.m[10] = fn;

	m.m[12] = xlr;
	m.m[13] = ytb;
	m.m[14] = zfn;
	return m;
}

void keyPressed(unsigned char key, int x, int y)
{
	printf("keyyy"); // Set the state of the current key to pressed
	if (key == 'a') {
		translateAnimX -= 0.5;
	}

	if (key == 'd') {
		translateAnimX += 0.5;
	}

	if (key == 's') {
		translateAnimY -= 0.5;
	}

	if (key == 'w') {
		translateAnimY += 0.5;
	}
}

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS





void generateObjectBufferTeapot () {
	GLuint vp_vbo = 0;

	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normals");
	
	glGenBuffers (1, &vp_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof (float), teapot_vertex_points, GL_STATIC_DRAW);
	GLuint vn_vbo = 0;
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof (float), teapot_normals, GL_STATIC_DRAW);
  
	glGenVertexArrays (1, &teapot_vao);
	glBindVertexArray (teapot_vao);

	glEnableVertexAttribArray (loc1);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (loc2);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer (loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}


#pragma endregion VBO_FUNCTIONS


void display(){

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	glutKeyboardFunc(keyPressed);

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");

	if (animationFactor >= 360.0)
		animationFactor = 0;
	animationFactor += 0.08;

	if (flipAnimationFactor >= 40.0 || flipAnimationFactor < -40.0)
		animToggle = !animToggle;

		
	if(animToggle)
		flipAnimationFactor += 0.08;
	else
		flipAnimationFactor -= 0.08;
	
	

	// Hierarchy of Teapots

	// Root of the Hierarchy
	mat4 view = identity_mat4();
	mat4 persp_proj = perspective(45.0, (float)width / (float)height, 0.1, 180.0);
	mat4 local1 = identity_mat4();
	local1 = rotate_y_deg(local1, animationFactor);
	local1 = translate(local1, vec3(translateAnimX, translateAnimY, -130));

	// for the root, we orient it in global space
	mat4 global1 = local1;
	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global1.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	// child of hierarchy
	mat4 local2 = identity_mat4();
	local2 = rotate_y_deg(local2, rotatez);
	// translation is 15 units in the y direction from the parents coordinate system
	local2 = translate(local2, vec3(0.0, 15.0, 0.0));
	// global of the child is got by pre-multiplying the local of the child by the global of the parent
	mat4 global2 = global1 * local2;
	// update uniform & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global2.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	// child of hierarchy
	mat4 local3 = identity_mat4();
	local3 = rotate_x_deg(local3, flipAnimationFactor);
	// translation is 15 units in the y direction from the parents coordinate system
	local3 = translate(local3, vec3(0.0, 15.0, 0.0));
	// global of the child is got by pre-multiplying the local of the child by the global of the parent
	mat4 global3 = global2 * local3;
	// update uniform & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global3.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);


	//////////////////////////////
	// LEFT GROUP STARTS
	//////////////////////////////
	/*
	mat4 local1_L = identity_mat4();
	local1_L = rotate_y_deg(local1_L, 0.0);
	// translation is 15 units in the y direction from the parents coordinate system
	local1_L = translate(local1_L, vec3(0.0, 15.0, 0.0));
	// global of the child is got by pre-multiplying the local of the child by the global of the parent
	mat4 global1_L = global3 * local1_L;
	// update uniform & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global1_L.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	*/
	// Root of the Hierarchy
	
	mat4 local1_L = identity_mat4();
	local1_L = rotate_z_deg(local1_L, 55.0f);
	local1_L = translate(local1_L, vec3(-10.0, 15.0, 0.0));

	// for the root, we orient it in global space
	mat4 global1_L = global3*local1_L;
	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global1_L.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	
	// child of hierarchy
	mat4 local2_L = identity_mat4();
	local2_L = rotate_y_deg(local2_L, 6* animationFactor);
	// translation is 15 units in the y direction from the parents coordinate system
	local2_L = translate(local2_L, vec3(0.0, 15.0, 0.0));
	// global of the child is got by pre-multiplying the local of the child by the global of the parent
	mat4 global2_L = global1_L * local2_L;
	// update uniform & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global2_L.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	// child of hierarchy
	mat4 local3_L = identity_mat4();
	local3_L = rotate_y_deg(local3_L, rotatez);
	// translation is 15 units in the y direction from the parents coordinate system
	local3_L = translate(local3_L, vec3(0.0, 15.0, 0.0));
	// global of the child is got by pre-multiplying the local of the child by the global of the parent
	mat4 global3_L = global2_L * local3_L;
	// update uniform & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global3_L.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	//////////////////////////////
	// RIGHT GROUP STARTS
	//////////////////////////////

	// Root of the Hierarchy

	mat4 local1_R = identity_mat4();
	local1_R = rotate_z_deg(local1_R, -55.0f);
	local1_R = translate(local1_R, vec3(10.0, 15.0, 0.0));

	// for the root, we orient it in global space
	mat4 global1_R = global3 * local1_R;
	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global1_R.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	// child of hierarchy
	mat4 local2_R = identity_mat4();
	local2_R = rotate_y_deg(local2_R, rotatez);
	// translation is 15 units in the y direction from the parents coordinate system
	local2_R = translate(local2_R, vec3(0.0, 15.0, 0.0));
	// global of the child is got by pre-multiplying the local of the child by the global of the parent
	mat4 global2_R = global1_R * local2_R;
	// update uniform & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global2_R.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	// child of hierarchy
	mat4 local3_R = identity_mat4();
	local3_R = rotate_y_deg(local3_R, rotatez);
	// translation is 15 units in the y direction from the parents coordinate system
	local3_R = translate(local3_R, vec3(0.0, 15.0, 0.0));
	// global of the child is got by pre-multiplying the local of the child by the global of the parent
	mat4 global3_R = global2_R * local3_R;
	// update uniform & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global3_R.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	
	glutSwapBuffers();

	/*
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable (GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram (shaderProgramID);


	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation (shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation (shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation (shaderProgramID, "proj");
	

	//Here is where the code for the viewport lab will go, to get you started I have drawn a t-pot in the bottom left
	//The model transform rotates the object by 45 degrees, the view transform sets the camera at -40 on the z-axis, and the perspective projection is setup using Antons method

	// bottom-left
	mat4 view = translate (identity_mat4 (), vec3 (0.0, 0.0, -40.0));
	mat4 persp_proj = perspective(45.0, (float)width/(float)height, 0.1, 100.0);
	mat4 model = rotate_z_deg (identity_mat4 (), 0);

	glViewport (0, 0, width / 2, height / 2);
	glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv (matrix_location, 1, GL_FALSE, model.m);
	glDrawArrays (GL_TRIANGLES, 0, teapot_vertex_count);

	if (cameraAnimationFactor > 75.0)
		cameraAnimationFactor = 0;
	cameraAnimationFactor += 0.01;

	// bottom-right
	 view = look_at(vec3(0.0, cameraAnimationFactor, -25.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
	 persp_proj = ortho(-(float)width / 20, (float)width / 20, -(float)height / 20, (float)height / 20, 0.1f, 100.0f);
	 //verify orthographic projection with glm function.
	 //glm::mat4 projMat = glm::ortho(-(float)width/20, (float)width / 20, -(float)height / 20, (float)height / 20, 0.1f, 100.0f);
	 model = rotate_y_deg(identity_mat4(), cameraAnimationFactor);

	glViewport(width / 2, 0, width / 2, height / 2);
	//glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(projMat));
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
		

		
	
    glutSwapBuffers();
	*/
}


void updateScene() {	

		// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	float  delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Create 3 vertices that make up a triangle that fits on the viewport 
	GLfloat vertices[] = {-1.0f, -1.0f, 0.0f, 1.0,
			1.0f, -1.0f, 0.0f, 1.0, 
			0.0f, 1.0f, 0.0f, 1.0};
	// Create a color array that identfies the colors of each vertex (format R, G, B, A)
	GLfloat colors[] = {0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f};
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();

	// load teapot mesh into a vertex buffer array
	generateObjectBufferTeapot ();
	
}

int main(int argc, char** argv){

	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Viewport Teapots");
	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);

	 // A call to glewInit() must be done after glut is initialized!
	glewExperimental = GL_TRUE; //for non-lab machines, this line gives better modern GL support
    GLenum res = glewInit();
	// Check for any errors
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
    return 0;
}











