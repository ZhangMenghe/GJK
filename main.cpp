#define GL_LITE_IMPLEMENTATION
#include "gl_lite.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

GLFWwindow* window = NULL;
int gl_width = 360;
int gl_height = 240;
float gl_aspect_ratio = (float)gl_width/gl_height;

#include "GameMaths.h"
#include "Input.h"
#include "Camera3D.h"
#include "init_gl.h"
#include "Shader.h"
#include "load_obj.h"
#include "GJK.h"
#include "Player.h"

int main() {
	if(!init_gl(window, "GJK", gl_width, gl_height)){ return 1; }

	//Load cube mesh
	GLuint cube_vao;
	unsigned int cube_num_indices = 0;
	{
		float* vp = NULL;
		uint16_t* indices = NULL;
		unsigned int num_verts = 0;
		load_obj_indexed("cube.obj", &vp, &indices, &num_verts, &cube_num_indices);

		glGenVertexArrays(1, &cube_vao);
		glBindVertexArray(cube_vao);
		
		GLuint points_vbo;
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, num_verts*3*sizeof(float), vp, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VP_ATTRIB_LOC);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glVertexAttribPointer(VP_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		free(vp);

		GLuint index_vbo;
		glGenBuffers(1, &index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_num_indices*sizeof(unsigned short), indices, GL_STATIC_DRAW);
		free(indices);
	}
	//Load sphere mesh
	GLuint sphere_vao;
	unsigned int sphere_num_indices = 0;
	{
		float* vp = NULL;
		uint16_t* indices = NULL;
		unsigned int num_verts = 0;
		load_obj_indexed("sphere.obj", &vp, &indices, &num_verts, &sphere_num_indices);

		glGenVertexArrays(1, &sphere_vao);
		glBindVertexArray(sphere_vao);
		
		GLuint points_vbo;
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, num_verts*3*sizeof(float), vp, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VP_ATTRIB_LOC);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glVertexAttribPointer(VP_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		free(vp);

		GLuint index_vbo;
		glGenBuffers(1, &index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_num_indices*sizeof(unsigned short), indices, GL_STATIC_DRAW);
		free(indices);
	}
	//Load cube mesh
	GLuint cylinder_vao;
	unsigned int cylinder_num_indices = 0;
	{
		float* vp = NULL;
		uint16_t* indices = NULL;
		unsigned int num_verts = 0;
		load_obj_indexed("cylinder.obj", &vp, &indices, &num_verts, &cylinder_num_indices);

		glGenVertexArrays(1, &cylinder_vao);
		glBindVertexArray(cylinder_vao);
		
		GLuint points_vbo;
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, num_verts*3*sizeof(float), vp, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VP_ATTRIB_LOC);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glVertexAttribPointer(VP_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		free(vp);

		GLuint index_vbo;
		glGenBuffers(1, &index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder_num_indices*sizeof(unsigned short), indices, GL_STATIC_DRAW);
		free(indices);
	}

	//Set up level geometry
	#define NUM_BOXES 5
	mat4 box_M[NUM_BOXES];
	vec4 box_colour[NUM_BOXES];
	BBox box_collider[NUM_BOXES];
	{
		const vec3 box_pos[NUM_BOXES] = {
			vec3(-6, 0,-6),
			vec3(-6, 0, 6),
			vec3( 0, 0, 0),
			vec3( 6, 0,-6),
			vec3( 6, 0, 6)
		};

		const vec3 box_scale[NUM_BOXES] = {
			vec3(5.0f, 2.0f, 5.0f),
			vec3(5.0f, 1.0f, 5.0f),
			vec3(5.0f, 1.0f, 5.0f),
			vec3(5.0f, 3.0f, 5.0f),
			vec3(5.0f, 1.0f, 5.0f)
		};

		box_M[0] = translate(rotate_y_deg(scale(identity_mat4(),box_scale[0]), 45), box_pos[0]);
		box_M[1] = translate(scale(identity_mat4(),box_scale[1]), box_pos[1]);
		box_M[2] = translate(scale(identity_mat4(),box_scale[2]), box_pos[2]);
		box_M[3] = translate(rotate_x_deg(scale(identity_mat4(),box_scale[3]), 40), box_pos[3]);
		box_M[4] = translate(rotate_z_deg(scale(identity_mat4(),box_scale[4]), 50), box_pos[4]);
	
		//Set up physics objects
		box_collider[0].pos = box_pos[0];
		box_collider[1].pos = box_pos[1];
		box_collider[2].pos = box_pos[2];
		box_collider[3].pos = box_pos[3];
		box_collider[4].pos = box_pos[4];
		for(int i=0; i<NUM_BOXES; i++)
		{
			box_collider[i].min = vec3(-0.5, 0,-0.5);
			box_collider[i].max = vec3( 0.5, 1, 0.5);
			box_collider[i].matRS = box_M[i];
			box_collider[i].matRS_inverse = inverse(box_M[i]);
			box_colour[i] = vec4(0.8f, 0.1f, 0.1f, 1);
		}
	}
	#define NUM_SPHERES 3
	mat4 sphere_M[NUM_SPHERES];
	vec4 sphere_colour[NUM_SPHERES];
	Sphere sphere_collider[NUM_SPHERES];
	{
		const vec3 sphere_pos[NUM_SPHERES] = {
			vec3(-6, 3,-6),
			vec3(-6, 2, 6),
			vec3( 6, 5, -6)
		};

		sphere_M[0] = translate(identity_mat4(), sphere_pos[0]);
		sphere_M[1] = translate(identity_mat4(), sphere_pos[1]);
		sphere_M[2] = translate(identity_mat4(), sphere_pos[2]);
	
		//Set up physics objects
		sphere_collider[0].pos = sphere_pos[0];
		sphere_collider[1].pos = sphere_pos[1];
		sphere_collider[2].pos = sphere_pos[2];
		for(int i=0; i<NUM_SPHERES; i++)
		{
			sphere_collider[i].r = 1;
			sphere_collider[i].matRS = sphere_M[i];
			sphere_collider[i].matRS_inverse = inverse(sphere_M[i]);
			sphere_colour[i] = vec4(0.1f, 0.8f, 0.1f, 1);
		}
	}
	#define NUM_CYLINDERS 3
	mat4 cylinder_M[NUM_CYLINDERS];
	vec4 cylinder_colour[NUM_CYLINDERS];
	Cylinder cylinder_collider[NUM_CYLINDERS];
	{
		const vec3 cylinder_pos[NUM_CYLINDERS] = {
			vec3( 6, 0, 0),
			vec3(-6, 0, 0),
			vec3( 0, 0,-6)
		};
		const float cylinder_r[NUM_CYLINDERS] = {
			2, 1, 3
		};
		const float cylinder_h[NUM_CYLINDERS] = {
			2, 3, 3
		};

		cylinder_M[0] = translate(scale(identity_mat4(), vec3(cylinder_r[0], cylinder_h[0], cylinder_r[0])), cylinder_pos[0]);
		cylinder_M[1] = translate(scale(identity_mat4(), vec3(cylinder_r[1], cylinder_h[1], cylinder_r[1])), cylinder_pos[1]);
		cylinder_M[2] = translate(scale(identity_mat4(), vec3(cylinder_r[2], cylinder_h[2], cylinder_r[2])), cylinder_pos[2]);
	
		//Set up physics objects
		cylinder_collider[0].pos = cylinder_pos[0];
		cylinder_collider[1].pos = cylinder_pos[1];
		cylinder_collider[2].pos = cylinder_pos[2];
		for(int i=0; i<NUM_CYLINDERS; i++)
		{
			cylinder_collider[i].r = 1;
			cylinder_collider[i].y_base = cylinder_pos[i].y;
			cylinder_collider[i].y_cap = cylinder_pos[i].y + 1;
			cylinder_collider[i].matRS = cylinder_M[i];
			cylinder_collider[i].matRS_inverse = inverse(cylinder_M[i]);
			cylinder_colour[i] = vec4(0.8f, 0.1f, 0.8f, 1);
		}
	}

	//Set up player's physics collider
	BBox player_collider;
	player_collider.pos = player_pos;
	player_collider.min = vec3(-0.5, 0,-0.5);
	player_collider.max = vec3( 0.5, 1, 0.5);
	player_collider.matRS = identity_mat4();
	player_collider.matRS_inverse = identity_mat4();

	//Camera setup
	g_camera.init(vec3(0,3,6), vec3(0,0,0));

	//Load shader
	Shader basic_shader = init_shader("MVP.vert", "uniform_colour.frag");
	GLuint colour_loc = glGetUniformLocation(basic_shader.id, "colour");
	glUseProgram(basic_shader.id);
	glUniformMatrix4fv(basic_shader.V_loc, 1, GL_FALSE, g_camera.V.m);
	glUniformMatrix4fv(basic_shader.P_loc, 1, GL_FALSE, g_camera.P.m);

	check_gl_error();

	double curr_time = glfwGetTime(), prev_time, dt;
	//-------------------------------------------------------------------------------------//
	//-------------------------------------MAIN LOOP---------------------------------------//
	//-------------------------------------------------------------------------------------//
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Get dt
		prev_time = curr_time;
		curr_time = glfwGetTime();
		dt = curr_time - prev_time;
		if(dt > 0.1) dt = 0.1;

		// static float fps_timer = 0.0f;
		// fps_timer+=dt;
		// if(fps_timer>0.2f){
		// 	char title_string[64];
		// 	sprintf(title_string, "GJK - %.2fHz", 1/dt);
		// 	glfwSetWindowTitle(window, title_string);
		// 	fps_timer = 0.0f;
		// }

		//Get Input
		g_mouse.prev_xpos = g_mouse.xpos;
    	g_mouse.prev_ypos = g_mouse.ypos;
		glfwPollEvents();
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
		}

		static bool freecam_mode = false;
		static bool F_was_pressed = false;
		if(glfwGetKey(window, GLFW_KEY_F)) {
			if(!F_was_pressed) { freecam_mode = !freecam_mode; }
			F_was_pressed = true;
		}
		else F_was_pressed = false;

		//Move player
		if(!freecam_mode) player_update(dt);

		//do collision detection
		{
			player_collider.pos = player_pos;
			bool hit_something = false;
			//BOXES
			for(int i=0; i<NUM_BOXES; i++)
			{
				vec3 mtv(0,0,0); //minimum translation vector
				if(gjk(&player_collider, &box_collider[i], &mtv)){
					hit_something = true;
					float ground_slope = RAD2DEG(acos(dot(normalise(mtv), vec3(0,1,0))));
					if(ground_slope<player_max_stand_slope){
						player_vel.y = 0;
						player_is_on_ground = true;
						player_is_jumping = false;
					}
				}
				player_pos += mtv;

				player_M = translate(identity_mat4(), player_pos);
			}
			//SPHERES
			for(int i=0; i<NUM_SPHERES; i++)
			{
				if(gjk(&player_collider, &sphere_collider[i])){
					sphere_colour[i] = vec4(0.8f,0.8f,0.1f,1);
				}
				else sphere_colour[i] = vec4(0.1f,0.8f,0.1f,1);
				player_M = translate(identity_mat4(), player_pos);
			}
			//CYLINDERS
			for(int i=0; i<NUM_CYLINDERS; i++)
			{
				vec3 mtv(0,0,0); //minimum translation vector
				if(gjk(&player_collider, &cylinder_collider[i], &mtv)){
					player_pos += mtv;
					cylinder_colour[i] = vec4(0.8f,0.8f,0.1f,1);

					hit_something = true;
					float ground_slope = RAD2DEG(acos(dot(normalise(mtv), vec3(0,1,0))));
					if(ground_slope<player_max_stand_slope){
						player_vel.y = 0;
						player_is_on_ground = true;
						player_is_jumping = false;
					}
				}
				else cylinder_colour[i] = vec4(0.8f,0.1f,0.8f,1);

				player_M = translate(identity_mat4(), player_pos);
			}
			//Grace Period for jumping when running off platforms
			{
				static float plat_fall_timer = 0;
				const float plat_fall_time = 0.15;
				if(!hit_something && player_pos.y>0)
				{
					plat_fall_timer += dt;
					if(plat_fall_timer>plat_fall_time || length(player_vel)<player_top_speed/2)
					{
						plat_fall_timer = 0;
						player_is_on_ground = false;
					}
				}
			}
		}

		if(freecam_mode) g_camera.update_debug(dt);
		else g_camera.update_player(player_pos, dt);

		static bool draw_wireframe = true;
		static bool slash_was_pressed = false;
		if(glfwGetKey(window, GLFW_KEY_SLASH)) {
			if(!slash_was_pressed) { draw_wireframe = !draw_wireframe; }
			slash_was_pressed = true;
		}
		else slash_was_pressed = false;

		//Rendering
		glUseProgram(basic_shader.id);
		glUniformMatrix4fv(basic_shader.V_loc, 1, GL_FALSE, g_camera.V.m);

		//Cubes
		glBindVertexArray(cube_vao);
		for(int i=0; i<NUM_BOXES; i++){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthFunc(GL_LESS);
			glUniform4fv(colour_loc, 1, box_colour[i].v);
			glUniformMatrix4fv(basic_shader.M_loc, 1, GL_FALSE, box_M[i].m);
			glDrawElements(GL_TRIANGLES, cube_num_indices, GL_UNSIGNED_SHORT, 0);

			if(draw_wireframe){
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDepthFunc(GL_ALWAYS);
				glUniform4fv(colour_loc, 1, vec4(0,0,0,1).v);
				glDrawElements(GL_TRIANGLES, cube_num_indices, GL_UNSIGNED_SHORT, 0);
			}
		}
		//Spheres
		glBindVertexArray(sphere_vao);
		for(int i=0; i<NUM_SPHERES; i++){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthFunc(GL_LESS);
			glUniform4fv(colour_loc, 1, sphere_colour[i].v);
			glUniformMatrix4fv(basic_shader.M_loc, 1, GL_FALSE, sphere_M[i].m);
			glDrawElements(GL_TRIANGLES, sphere_num_indices, GL_UNSIGNED_SHORT, 0);
		}
		//Cylinders
		glBindVertexArray(cylinder_vao);
		for(int i=0; i<NUM_CYLINDERS; i++){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthFunc(GL_LESS);
			glUniform4fv(colour_loc, 1, cylinder_colour[i].v);
			glUniformMatrix4fv(basic_shader.M_loc, 1, GL_FALSE, cylinder_M[i].m);
			glDrawElements(GL_TRIANGLES, cylinder_num_indices, GL_UNSIGNED_SHORT, 0);

			if(draw_wireframe){
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDepthFunc(GL_ALWAYS);
				glUniform4fv(colour_loc, 1, vec4(0,0,0,1).v);
				glDrawElements(GL_TRIANGLES, cylinder_num_indices, GL_UNSIGNED_SHORT, 0);
			}
		}
		//Player
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDepthFunc(GL_LESS);
		glUniformMatrix4fv(basic_shader.M_loc, 1, GL_FALSE, player_M.m);
		glUniform4fv(colour_loc, 1, player_colour.v);
		glBindVertexArray(cube_vao);
		glDrawElements(GL_TRIANGLES, cube_num_indices, GL_UNSIGNED_SHORT, 0);

		check_gl_error();

		glfwSwapBuffers(window);
	}//end main loop
	return 0;
}
