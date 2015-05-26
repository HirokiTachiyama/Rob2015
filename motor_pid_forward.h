
#ifndef __MOTOR_PID_FORWARD__
#define __MOTOR_PID_FORWARD__

#include "pid.h"
#include "common_definition.h"


typedef struct motor_pid_forward_tructure{
  pid *my_pid;
  int sum_motor_get_count; //左右のロータリーエンコーダの値の合計の目標値
}motor_pid_forward;

motor_pid_forward  *__motor_pid_forward_make();
motor_pid_forward *motor_pid_forward_make(double kp,double ki,double kd);
void motor_pid_forward_free(motor_pid_forward *target_motor_pid_forward);
void motor_pid_forward_update(motor_pid_forward *target_motor_pid_forward);
double motor_pid_forward_get_forward(motor_pid_forward *target_motor_pid_forward);

#endif
