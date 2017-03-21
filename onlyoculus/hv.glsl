#version 450 core
layout(location=0) in vec3 vertices;
uniform mat4 mat_model;
uniform mat4 mat_view;
uniform mat4 mat_projection;

void main(){
	gl_Position=mat_projection*mat_view*mat_model*vec4(-vertices.x,vertices.y,vertices.z,1);
}