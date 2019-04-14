#ifndef SIMULATION_H_INCLUDED
#define SIMULATION_H_INCLUDED
#include "fftw3.h"
#include <stdint.h>
#include <linmath.h>

#define Meas_hole_rad 0.075f
#define Meas_hole_x 0.5f
#define Meas_hole_y 0.8f

//TODO change on simulation_load_potential depening on image reaolution
int sim_res_x;
int sim_res_y;
int sim_res_total;

int MeasColorBySim;
enum{Meas_green=0,Meas_red=1,Meas_yellow=2};
fftw_complex *psi;

void simulation_alloc(); //Needs to be called before any aanimation can start.
void simulation_dealloc();

void simulation_pause();
void simulation_unpause();

int simulation_redraw_wave(int offset_x,int offset_y,float angle,float momentum,float gauss_width);
int simulation_run(float dt);

enum {meas_hit,meas_no_hit,meas_blocked};
int simulation_measurement(double glfwTime,vec2 meas_pos_in_grid_coordinates);
void simulation_load_potential(uint8_t* pot);

#endif
