/**
 * @file kalman_fliter.c
 * @brief kalman fliter algorithm based on arm dsp library
 * @version 1.0
 * @author jar chou
*/

#include "kalman_fliter.h"
#include "arm_math.h"

//the kalman fliter structure that include this kalman fliter's parameters and status variables ,base on dsp matrix structure
typedef struct kalman_fliter
{
    //the matrix of kalman fliter
    arm_matrix_instance_f32 x0; //the initial state vector
    arm_matrix_instance_f32 p0; //the initial error covariance matrix
    arm_matrix_instance_f32 a;  //the state transition matrix
    arm_matrix_instance_f32 b;  //the control input matrix
    arm_matrix_instance_f32 c;  //the observation matrix
    arm_matrix_instance_f32 q;  //the process noise covariance matrix
    arm_matrix_instance_f32 r;  //the observation noise covariance matrix
    arm_matrix_instance_f32 k;  //the kalman gain matrix
};


/**
 * @brief kalman fliter init
 * @param[in] kalman_fliter_t *kalman_fliter
 * @param[in] float32_t *x0
 * @param[in] float32_t *p0
 * @param[in] float32_t *a
 * @param[in] float32_t *b
 * @param[in] float32_t *c
 * @param[in] float32_t *q
 * @param[in] float32_t *r
 * @return none
*/

