#include "simulation.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

enum {simulation_state_not_allocated,simulation_state_simulate,simulation_state_measurement_animation,simulation_state_created_and_wait_for_start,simulation_state_wait_for_restart};
int MeasColorBySim;
int simulation_state=simulation_state_not_allocated;
int simulation_paused;
//typedef double fftw_complex[2];

//fftw_complex *psi;
fftw_complex *psi_transform;
fftw_complex *prop;
fftw_complex *animation_start;
fftw_complex *animation_end;
fftw_plan fft;
fftw_plan ifft;

double* potential;

void simulation_pause(){
    simulation_paused=1;
}
void simulation_unpause(){
    simulation_paused=0;
}


void simulation_alloc(){
    sim_res_x=512;
    sim_res_y=512;
    sim_res_total=sim_res_x*sim_res_y;

    psi = fftw_alloc_complex(sim_res_total);
    psi_transform = (fftw_complex*) fftw_alloc_complex(sim_res_total);
    prop = (fftw_complex*) fftw_alloc_complex(sim_res_total);

    animation_start = (fftw_complex*) fftw_alloc_complex(sim_res_total);
    animation_end = (fftw_complex*) fftw_alloc_complex(sim_res_total);

    fft = fftw_plan_dft_2d(sim_res_x, sim_res_y, psi, psi_transform, FFTW_FORWARD, FFTW_MEASURE);
    ifft = fftw_plan_dft_2d(sim_res_x, sim_res_y, psi_transform, psi, FFTW_BACKWARD, FFTW_MEASURE);

    potential = (double*) malloc(sim_res_total* sizeof(double));
    simulation_state=simulation_state_created_and_wait_for_start;
}

void simulation_dealloc(){
    fftw_destroy_plan(fft);
    fftw_destroy_plan(ifft);

    fftw_free(prop);
    fftw_free(psi_transform);
    fftw_free(psi);

    fftw_free(animation_start);
    fftw_free(animation_end);

    free(potential);
    simulation_state=simulation_state_not_allocated;
}

int simulation_redraw_wave(int offset_x,int offset_y,float angle,float momentum,float gauss_width){
    if(simulation_state==simulation_state_measurement_animation){
        printf("Warn: Measurement animation still running, can't redraw wave!\n");
        return 1;
    }else if(simulation_state==simulation_state_simulate){
        printf("Warn: Simulation still running, can't redraw wave!\n");
        return 1;
    }else if(simulation_state==simulation_state_not_allocated){
        printf("Error: FFTW arrays not allocated!\n");
        return 2;
    }/*else if(simulation_state==simulation_state_wait_for_restart){
        printf("Error: not restarted yet!\n");
        return 3;
    }*/
    MeasColorBySim=Meas_yellow;

    //for gauss function will be cut off to increase performance at redraw
    int cutSquareHalf=(int)(gauss_width*gauss_width*10.f);
    memset(&(psi[0][0]),0,sim_res_total*4*sizeof(float));
    for(int j = 0; j < sim_res_y; j++) {
        for(int i = 0; i < sim_res_x; i++) {
            int gauss_x_squared=(i-offset_x)*(i-offset_x);
            int gauss_y_squared=(j-offset_y)*(j-offset_y);
            int gauss_r_squared=gauss_x_squared+gauss_y_squared;
            if(gauss_r_squared<cutSquareHalf){
                psi[i+j*sim_res_x][0]=exp(-gauss_r_squared/(2*gauss_width*gauss_width)) * cos(((i - sim_res_x / 2.0f) * cos(angle) + ((j - sim_res_y / 2.0f) * sin(angle))) * momentum);
                psi[i+j*sim_res_x][1]=exp(-gauss_r_squared/(2*gauss_width*gauss_width)) * sin(((i - sim_res_x / 2.0f) * cos(angle) + ((j - sim_res_y / 2.0f) * sin(angle))) * momentum);
            }
        }
    }
    simulation_state=simulation_state_created_and_wait_for_start;
    simulation_paused=1;
    return 0;
}

int simulation_run(float dt){
    if((simulation_state!=simulation_state_created_and_wait_for_start)&&(simulation_state!=simulation_state_simulate)){
        printf("Warn: Wave packet not initialized, won't start simulation!\n");
        return 1;
    }
    //printf("Paused:? %d\n",simulation_paused);
    if(simulation_paused){

        return 2;
    }
    static float dt_old=0.f;
    if(dt_old!=dt){ //Generate new momentum prop
        for(int y = 0; y < sim_res_y / 2; y++) {
            for(int x = 0; x < sim_res_x / 2; x++) { //e^(-*p^2/2m) from Hamilton
                prop[y * sim_res_x + x][0] = cos(dt * (-x * x - y * y));
                prop[y * sim_res_x + x][1] = sin(dt * (-x * x - y * y));
            }
            for(int x = sim_res_x / 2; x < sim_res_x; x++) {
                prop[y * sim_res_x + x][0] = cos(dt * (-(x - sim_res_x) * (x - sim_res_x) - y * y));
                prop[y * sim_res_x + x][1] = sin(dt * (-(x - sim_res_x) * (x - sim_res_x) - y * y));
            }
        }
        for(int y = sim_res_y / 2; y < sim_res_y; y++) {
            for(int x = 0; x < sim_res_x / 2; x++) {
                prop[y * sim_res_x + x][0] = cos(dt * (-x * x - (y - sim_res_y) * (y - sim_res_y)));
                prop[y * sim_res_x + x][1] = sin(dt * (-x * x - (y - sim_res_y) * (y - sim_res_y)));
            }
            for(int x = sim_res_x / 2; x < sim_res_x; x++) {
                prop[y * sim_res_x + x][0] = cos(dt * (-(x - sim_res_x) * (x - sim_res_x) - (y - sim_res_y) * (y - sim_res_y)));
                prop[y * sim_res_x + x][1] = sin(dt * (-(x - sim_res_x) * (x - sim_res_x) - (y - sim_res_y) * (y - sim_res_y)));
            }
        }
        dt_old=dt;
    }
    //Change to momentum space (same as fourier transform)
    fftw_execute(fft);
    //Complex multiplication of the wave function in the momentum space with the squared momentum propagator e^(1/(i*hbar)*)?? TODO
    //Which applies the -p^2/2*m of the Hamilton Operator H=(-p^2/2*m+V(x))
    for(int i = 0; i < sim_res_total; i++) {
        double psi_re_temp = psi_transform[i][0];
        psi_transform[i][0] = psi_re_temp * prop[i][0] - psi_transform[i][1] * prop[i][1];
        psi_transform[i][1] = psi_re_temp * prop[i][1] + psi_transform[i][1] * prop[i][0];
    }
    //Change back to position space
    fftw_execute(ifft);
    //apply the potential part of the Hamilton operator
    //propagator is e^(i*V(x)*t)
    for(int i = 0; i < sim_res_total; i++) {
        double psi_re_temp = psi[i][0];
        psi[i][0] = psi_re_temp * cos(potential[i]) - psi[i][1] * sin(potential[i]);
        psi[i][1] = psi_re_temp * sin(potential[i]) + psi[i][1] * cos(potential[i]);
    }

    int biggest = 0;
    biggest = 0;

    for(int i = 0; i < sim_res_total; i++) {
        if(psi[i][0]*psi[i][0] + psi[i][1]*psi[i][1] > psi[biggest][0]*psi[biggest][0] + psi[biggest][1]*psi[biggest][1]){
            biggest = i;
        }
    }
    float norm=sqrt(psi[biggest][0]*psi[biggest][0]+psi[biggest][1]*psi[biggest][1]);

    for(int i = 0; i < sim_res_total; i++) {
        psi[i][0]/=norm;
        psi[i][1]/=norm;
    }
    //printf("Debug Norming %f %f %f\n",norm,psi[biggest][0],psi[biggest][1]);

    simulation_state=simulation_state_simulate;
    return 0;
}

int simulation_measurement(double glfwTime,vec2 meas_pos_in_grid_coordinates){
    if(simulation_state!=simulation_state_simulate){
        return meas_blocked;
    }
    simulation_pause();
    simulation_state=simulation_state_wait_for_restart;
    //Internal variable which holds information of what part of the animation will get executed in this frame
    srand((long)(10000.0f * glfwTime));
    double random = (rand() % 1001) / 1000.0f;
    double sum = 0;
    double norm_sum = 0;
    for(int i = 0; i < sim_res_total; i++) {
        norm_sum = norm_sum + (psi[i][0] * psi[i][0] + psi[i][1] * psi[i][1]);
    }
    int meas_pos=0;
    for(; meas_pos < sim_res_total; meas_pos++) {
        sum = sum + ((psi[meas_pos][0] * psi[meas_pos][0] + psi[meas_pos][1] * psi[meas_pos][1]) / norm_sum);
        if(sum > random) {
            printf("Sum:  %f\n",sum);
            printf("Rand: %f\n",random);
            printf("Norm %f\n",norm_sum);
            break;
        }
    }
    memset(&(psi[0][0]),0,sim_res_total*4*sizeof(float));
    //Pixel
    float meas_x=(meas_pos%sim_res_x)/(float)sim_res_x;
    float meas_y=(meas_pos/sim_res_x)/(float)sim_res_y;
    float meas_x_hole_dist_sqr=(meas_x-Meas_hole_x)*(meas_x-Meas_hole_x);
    float meas_y_hole_dist_sqr=(meas_y-Meas_hole_y)*(meas_y-Meas_hole_y);
    printf("Distxsq %f Distysq %f \n",meas_x_hole_dist_sqr,meas_y_hole_dist_sqr);
    if((meas_x_hole_dist_sqr+meas_y_hole_dist_sqr)<Meas_hole_rad*Meas_hole_rad){
        printf("Inside hole.\n");
        MeasColorBySim=Meas_green;
        meas_pos_in_grid_coordinates[0]=(float)(meas_pos%sim_res_x);
        meas_pos_in_grid_coordinates[1]=(float)(meas_pos/sim_res_x);
        psi[meas_pos][0]=1.0f;
        psi[meas_pos+1][0]=1.0f;
        psi[meas_pos+2][0]=1.0f;
        psi[meas_pos+3][0]=1.0f;
        psi[meas_pos+sim_res_x][0]=1.0f;
        psi[meas_pos+sim_res_x+1][0]=1.0f;
        psi[meas_pos+sim_res_x+2][0]=1.0f;
        psi[meas_pos+sim_res_x+3][0]=1.0f;
        psi[meas_pos+sim_res_x*2][0]=1.0f;
        psi[meas_pos+sim_res_x*2+1][0]=1.0f;
        psi[meas_pos+sim_res_x*2+2][0]=1.0f;
        psi[meas_pos+sim_res_x*2+3][0]=1.0f;
        psi[meas_pos+sim_res_x*3][0]=1.0f;
        psi[meas_pos+sim_res_x*3+1][0]=1.0f;
        psi[meas_pos+sim_res_x*3+2][0]=1.0f;
        psi[meas_pos+sim_res_x*3+3][0]=1.0f;
        return meas_hit;
    }else{
        printf("Outside hole.\n");
        MeasColorBySim=Meas_red;
        meas_pos_in_grid_coordinates[0]=(float)(meas_pos%sim_res_x);
        meas_pos_in_grid_coordinates[1]=(float)(meas_pos/sim_res_x);
        psi[meas_pos][1]=1.0f;
        psi[meas_pos+1][1]=1.0f;
        psi[meas_pos+2][1]=1.0f;
        psi[meas_pos+3][1]=1.0f;
        psi[meas_pos+sim_res_x][1]=1.0f;
        psi[meas_pos+sim_res_x+1][1]=1.0f;
        psi[meas_pos+sim_res_x+2][1]=1.0f;
        psi[meas_pos+sim_res_x+3][1]=1.0f;
        psi[meas_pos+sim_res_x*2][1]=1.0f;
        psi[meas_pos+sim_res_x*2+1][1]=1.0f;
        psi[meas_pos+sim_res_x*2+2][1]=1.0f;
        psi[meas_pos+sim_res_x*2+3][1]=1.0f;
        psi[meas_pos+sim_res_x*3][1]=1.0f;
        psi[meas_pos+sim_res_x*3+1][1]=1.0f;
        psi[meas_pos+sim_res_x*3+2][1]=1.0f;
        psi[meas_pos+sim_res_x*3+3][1]=1.0f;
        return meas_no_hit;
    }

}

void simulation_load_potential(uint8_t* pot){
    for(int i = 0; i < sim_res_total; i++) {
        potential[i] = (255 - pot[4 * i + 1]) / 255.0f;
    }
}

