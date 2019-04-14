#version 430 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv_in;
uniform sampler2D texture;
uniform mat4 MVPmatrix;
out vec2 UV;
void main(){
    UV=uv_in;
    gl_Position = MVPmatrix*vec4(position,0.0f,1.0f);
}
