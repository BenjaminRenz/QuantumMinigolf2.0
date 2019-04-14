enum {simulation_state_simulate,simulation_state_measurement_animation,simulation_state_create_and_wait_for_start,simulation_state_wait_for_restart}
int simulation_state;

int simulation_initialize(float angle,int offset_x,int offset_y,float diameter,float momentum){
    if(simulation_state==simulation_state_simulate){
        printf("Debug: Simulation still running, can't redraw.\n");
        return 1;
    }else if(simulation_state==simulation_state_measurement_animation){
        printf("Debug: Measurement animation still running, can't redraw.\n");
        return 1;
    }
     if(draw_new_wave == 1) {
            diameter = guiElementsStorage[GUI_SLIDER_SIZE].position_x * SIZE_MULTI + 10.0f;
            Movement_angle = PI * 2.0f *(guiElementsStorage[GUI_SLIDER_WAVE_ROTATION].position_x+0.25f);
            memset(&(psi[0][0]),0,Resolutionx*Resolutiony*4*sizeof(float));
            for(int j = 0; j < Resolutiony; j++) {
                for(int i = 0; i < Resolutionx; i++) {
                        //TODO radial cutoff for faster initialisation
                    if((abs(i-((int)wave_offset_x)))<(Resolutionx/10.0f)&&(abs(j-((int)wave_offset_y))<(Resolutiony/10.0f))){
                        psi[i + j * Resolutionx][0] = exp(-((i - ((int)wave_offset_x)) * (i - ((int)wave_offset_x)) / wave_proportion + (j - ((int)wave_offset_y)) * (j - ((int)wave_offset_y))) / (diameter)) * cos((i - Resolutionx / 2.0f) * cos(Movement_angle) + ((j - Resolutiony / 2.0f) * sin(Movement_angle)) * momentum_multi);
                        psi[i + j * Resolutionx][1] = exp(-((i - ((int)wave_offset_x)) * (i - ((int)wave_offset_x)) / wave_proportion + (j - ((int)wave_offset_y)) * (j - ((int)wave_offset_y))) / (diameter)) * sin((i - Resolutionx / 2.0f) * cos(Movement_angle) + ((j - Resolutiony / 2.0f) * sin(Movement_angle)) * momentum_multi);
                    }
                }
            }
            draw_new_wave = 0;
        }



    simulation_state=simulation_state_create_and_wait_for_start;
}

int simulate(){
    if(simulation_state == simulation_state_simulate) {
        timerForBlink(1); //reset standby counter so simulation isn't interrupted
        fftw_execute(fft);
        //momentum space
        for(int i = 0; i < Resolutionx * Resolutiony; i++) {
            double psi_re_temp = psi_transform[i][0];
            psi_transform[i][0] = psi_re_temp * prop[i][0] - psi_transform[i][1] * prop[i][1];
            psi_transform[i][1] = psi_re_temp * prop[i][1] + psi_transform[i][1] * prop[i][0];
        }
        fftw_execute(ifft);
        for(int i = 0; i < Resolutionx * Resolutiony; i++) {
            psi[i][0] = psi[i][0] / (double)(norm);
            psi[i][1] = psi[i][1] / (double)(norm);
        }
        for(int i = 0; i < Resolutionx * Resolutiony; i++) {
            double psi_re_temp = psi[i][0];
            psi[i][0] = psi_re_temp * cos(potential[i]) - psi[i][1] * sin(potential[i]);
            psi[i][1] = psi_re_temp * sin(potential[i]) + psi[i][1] * cos(potential[i]);
        }
        //Delete the border of the wavefunction horizontal
        for(int i = 0; i < Resolutionx; i++) {
            psi[i][0] = 0;
            psi[i][1] = 0;
            psi[i+1*Resolutionx][0] = 0;
            psi[i+1*Resolutionx][1] = 0;
            psi[i+2*Resolutionx][0] = 0;
            psi[i+2*Resolutionx][1] = 0;
            psi[i+3*Resolutionx][0] = 0;
            psi[i+3*Resolutionx][1] = 0;
            psi[i + (Resolutiony - 1)*Resolutionx][0] = 0;
            psi[i + (Resolutiony - 1)*Resolutionx][1] = 0;
            psi[i + (Resolutiony - 2)*Resolutionx][0] = 0;
            psi[i + (Resolutiony - 2)*Resolutionx][1] = 0;
            psi[i + (Resolutiony - 3)*Resolutionx][0] = 0;
            psi[i + (Resolutiony - 3)*Resolutionx][1] = 0;
            psi[i + (Resolutiony - 4)*Resolutionx][0] = 0;
            psi[i + (Resolutiony - 4)*Resolutionx][1] = 0;
        }
        //Delete the border of the wavefunction vertical
        for(int i = 0; i < Resolutiony; i++) {
            psi[1 + i * Resolutionx][0] = 0;//TODO problem for i=0?
            psi[1 + i * Resolutionx][1] = 0;
            psi[2 + i * Resolutionx][0] = 0;//TODO problem for i=0?
            psi[2 + i * Resolutionx][1] = 0;
            psi[3 + i * Resolutionx][0] = 0;//TODO problem for i=0?
            psi[3 + i * Resolutionx][1] = 0;
            psi[4 + i * Resolutionx][0] = 0;//TODO problem for i=0?
            psi[4 + i * Resolutionx][1] = 0;
            psi[Resolutionx - 1 + i * Resolutionx][0] = 0;
            psi[Resolutionx - 1 + i * Resolutionx][1] = 0;
            psi[Resolutionx - 2 + i * Resolutionx][0] = 0;
            psi[Resolutionx - 2 + i * Resolutionx][1] = 0;
            psi[Resolutionx - 3 + i * Resolutionx][0] = 0;
            psi[Resolutionx - 3 + i * Resolutionx][1] = 0;
            psi[Resolutionx - 4 + i * Resolutionx][0] = 0;
            psi[Resolutionx - 4 + i * Resolutionx][1] = 0;
        }
    }
}
