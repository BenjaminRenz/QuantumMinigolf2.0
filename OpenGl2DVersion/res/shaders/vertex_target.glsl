#version 430 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv_in;
out vec2 uv_pass_to_frag;
uniform int colorMeas;
uniform mat4 MVPmatrix;
void main(){
    uv_pass_to_frag=uv_in;
    gl_Position = MVPmatrix*vec4(position,-1.0f,1.0f);
}
