#version 430 core
layout(location = 0) in vec2 position;
uniform float potential_true;
uniform mat4 MVPmatrix;
out vec2 UV;
out float potential_true_frag;
uniform sampler2D texture0;
varying float zheight;
void main(){
    UV=position.xy+vec2(0.5f,0.5f);
    potential_true_frag=potential_true;
    float re=(texture(texture0,UV).b)*2-1.0f;
    float im=(texture(texture0,UV).g)*2-1.0f;
    float abs_sqr_psi=(re*re+im*im)/10.0f;
    float pot=(texture(texture0,UV).r)/10.0f;
    zheight=(1-potential_true)*abs_sqr_psi+(potential_true)*pot;
    gl_Position = MVPmatrix*vec4(position,zheight,1.0f);//(re*re+im*im)/10.0f,1.0f);
}
