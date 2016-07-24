#ifndef PREDICTOR_H
#define PREDICTOR_H
typedef unsigned int kalman_t; 
kalman_t kalman_construct();
void kalman_destruct(kalman_t kalman);
void kalman_set_dt(kalman_t kalman, double dt);
void kalman_reset(kalman_t kalman);
void kalman_observe(kalman_t kalman, double x);
double kalman_get_pos(kalman_t kalman);
double kalman_get_vel(kalman_t kalman);
#ifndef MODEL_O_2
void kalman_initial_observe(kalman_t kalman, double x, double v);
void kalman_set_params(kalman_t kalman, double dt, double v_noise, double v_gamma, double i_p_noise, double i_v_noise, double obs_noise);
#else
void kalman_initial_observe(kalman_t kalman, double x, double v,double a);
void kalman_set_params(kalman_t kalman, double dt, double v_noise, double v_gamma, double i_p_noise, double i_v_noise, double i_a_noise, double obs_noise);
double kalman_get_acc(kalman_t kalman);
#endif

#endif // PREDICTOR_H
