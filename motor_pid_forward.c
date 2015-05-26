
/*2015/05/07
 * ‰Á•M
 * NXT‚©‚çEV3‚Ö‚ÆˆÚA
 * NXT‚ÌAPI‚Ì•”•ª‚ğ“¯—l‚Ì‹@”\‚ğ‚Â
 * EV3‚ÌAPI‚É‘‚«Š·‚¦‚½
 */

#include "motor_pid_forward.h"

motor_pid_forward  *__motor_pid_forward_make(){
  return motor_pid_forward_make(0.05,0.01,0.02);
}

motor_pid_forward *motor_pid_forward_make(double kp,double ki,double kd){
  motor_pid_forward *new_motor_pid_forward =
    (motor_pid_forward *) malloc(sizeof(motor_pid_forward));

  //new_motor_pid_forward->sum_motor_get_count =
  // nxt_motor_get_count(NXT_PORT_B) + nxt_motor_get_count(NXT_PORT_C);
new_motor_pid_forward->sum_motor_get_count =
  ev3_motor_get_counts(right_motor) + ev3_motor_get_counts(left_motor);

  new_motor_pid_forward->my_pid  = pid_make(kp,ki,kd,0);
  return new_motor_pid_forward;
}

void motor_pid_forward_free(motor_pid_forward *target_motor_pid_forward){
  pid_free(target_motor_pid_forward->my_pid);
  free(target_motor_pid_forward);
}

void motor_pid_forward_update(motor_pid_forward *target_motor_pid_forward){
  //int diff = nxt_motor_get_count(NXT_PORT_B) +
  //  nxt_motor_get_count(NXT_PORT_C) - target_motor_pid_forward->sum_motor_get_count;
  int diff = ev3_motor_get_counts(right_motor) +
    ev3_motor_get_counts(left_motor) - target_motor_pid_forward->sum_motor_get_count;

  pid_input(target_motor_pid_forward->my_pid,diff);
}

double motor_pid_forward_get_forward(motor_pid_forward *target_motor_pid_forward){
  return -pid_get_output(*target_motor_pid_forward->my_pid);
}
