#version 450 core
uniform sampler2D artex;

in vec2 texcoords;

out vec4 color;

void main(){
    color = texture(artex,texcoords);
}