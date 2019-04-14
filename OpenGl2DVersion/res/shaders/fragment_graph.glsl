#version 430 core
in vec2 UV;
in float potential_true_frag;
//uniform sampler2D texture0;
out vec4 frag_color;
uniform sampler2D texture0;
varying float zheight;
void main(){
    float re=(texture(texture0,UV).b)*2-1;
    float im=(texture(texture0,UV).g)*2-1;
    //float re=(texture(texture0,UV).b)*2-1;
    //float im=(texture(texture0,UV).g)*2-1;
    float pot=texture(texture0,UV).r;
    float potential_false_frag=(1-potential_true_frag);
    //float intens=(re*re+im*im);
    if(zheight>0.001f){
        frag_color=vec4(potential_false_frag*sqrt(re*re)+zheight*10.0*potential_true_frag,zheight*10.0*potential_true_frag,potential_false_frag*sqrt(im*im)+zheight*10.0*potential_true_frag,1.0f*potential_true_frag+0.25f*potential_false_frag);
    }else{
        frag_color=vec4(potential_false_frag*sqrt(re*re)+pot*potential_true_frag,pot*potential_true_frag,potential_false_frag*sqrt(im*im)+pot*potential_true_frag,0.0f);
    }

}
