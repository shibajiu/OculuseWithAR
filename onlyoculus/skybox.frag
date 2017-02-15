#version 450 core
uniform samplerCube texSkyBox;
in vec3 texCoords;
out vec4 color;

void main(){
    color = texture(texSkyBox,texCoords);
}