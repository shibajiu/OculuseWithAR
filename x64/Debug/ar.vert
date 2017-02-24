#version 450 core
layout(location = 0) in vec2 vertices;
layout(location = 1) in vec2 texvertices;

uniform float shift;
out vec2 texcoords;

void main(){
    gl_Position = vec4(vertices.x+shift,vertices.y,0,1);
    texcoords = vec2(texvertices.x*0.5+0.5,texvertices.y*0.5+0.5);
}
