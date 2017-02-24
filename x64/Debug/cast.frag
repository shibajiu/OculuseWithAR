#version 450 core
out vec4 color;

void main(){
    color=vec4(gl_FragCoord.x *0.2,gl_FragCoord.y *0.5,gl_FragCoord.z *0.6,1);
}