#version 450 core

layout(location=0) in vec3 vertices;
layout(location = 1)in vec3 normals;

uniform mat4 mat_model;
uniform mat4 mat_view;
uniform mat4 mat_projection;
uniform mat4 mat_center;
uniform mat4 mat_scale;
uniform mat4 mat_center_reverse;

out VS_OUT{
    vec3 norm;
    vec3 fragpos; 
}vs_out;

void main(){
    //gl_Position=mat_projection*mat_view*mat_model*vec4(vertices,1);
    gl_Position=mat_projection*mat_view*mat_model*mat_scale*mat_center*vec4(vertices,1);
    //gl_Position=mat_projection*mat_view*mat_center_reverse*mat_model*mat_center*vec4(vertices,1);
    vs_out.fragpos=mat3(mat_model)*vertices;
    vs_out.norm=normals;
}
