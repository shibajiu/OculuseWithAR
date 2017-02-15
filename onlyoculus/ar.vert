#version 450 core
layout(location = 0) in vec2 vertices;

out vec2 texcoords;

void main(){
    gl_Position = vec4(vertices,0,1);
    texcoords = vec2(vertices.x*0.5+0.5,vertices.y*0.5+0.5);
}