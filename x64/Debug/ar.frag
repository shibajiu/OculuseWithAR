#version 450 core
uniform sampler2D artex;

in vec2 texcoords;

out vec4 color;

void main(){
	color = vec4(1,0,0.3,0);
    color = texture(artex,texcoords);
}