#version 430 core
out vec4 frag_color;
in vec2 uv_pass_to_frag;
#define Meas_green 0
#define Meas_red 1
#define Meas_yellow 2
uniform int colorMeas;
uniform sampler2D texture1;
void main(){
    if(colorMeas==Meas_green){
        frag_color=vec4(texture(texture1,uv_pass_to_frag)).bgba;
    }else if(colorMeas==Meas_red){
        frag_color=vec4(texture(texture1,uv_pass_to_frag)).rbba;
    }else{ //yellow
        frag_color=vec4(texture(texture1,uv_pass_to_frag));
    }

}
