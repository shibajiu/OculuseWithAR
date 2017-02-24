#version 450 core
out vec4 color;

uniform vec3 light_dir;
uniform vec3 view_pos;

in VS_OUT{
    vec3 norm;
    vec3 fragpos; 
}fs_in;

void main(){
    vec3 _color=vec3(0.3,0.5,0.5);

    vec3 ambient = 0.04*_color;

    vec3 norm = normalize(fs_in.norm);
    vec3 diffuse = max(dot(norm,light_dir),0.0)*_color;

    vec3 view_dir = view_pos - fs_in.fragpos;
    vec3 halfvec = normalize(-light_dir+view_pos);
    vec3 specular = pow(max(dot(halfvec,norm),0.0),32)*vec3(0.4);

    color = vec4(ambient + diffuse + specular,1.0);
}