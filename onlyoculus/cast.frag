#version 450 core
out vec4 color;

uniform vec3 light_dir;
uniform vec3 view_pos;
uniform vec3 model_color;

in VS_OUT{
    vec3 norm;
    vec3 fragpos; 
}fs_in;

void main(){
    vec3 _color = model_color;

    vec3 ambient = 0.1*_color;

    vec3 norm = normalize(fs_in.norm);
    vec3 diffuse = max(dot(norm,light_dir),0.0)*_color;

    vec3 view_dir = normalize(view_pos - fs_in.fragpos);
    vec3 halfvec = normalize(-light_dir+view_dir);
    vec3 specular = pow(max(dot(halfvec,norm),0.0),32)*_color;

    color = vec4(ambient + diffuse + specular,1.0);
}
