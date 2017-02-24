#version 450 core

layout(location=0) in vec3 vertices;
layout(location = 1)in vec3 normals;

uniform mat4 mat_model;
uniform mat4 mat_view;
uniform mat4 mat_projection;

out VS_OUT{
    vec3 norm;
    vec3 fragpos; 
}vs_out;

void main(){
    gl_Position=mat_projection*mat_view*mat_model*vec4(vertices,1);
    vs_out.fragpos=vertices;
    vs_out.norm=normals;
}
