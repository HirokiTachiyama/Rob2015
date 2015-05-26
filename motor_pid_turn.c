/*2015/05/07
 * 加筆
 * NXT用APIをEV3用APIに書き換え
 * F32型をfloatへ書き換え
 */
#include "motor_pid_turn.h"

//__________________コンストラクタ______________________
//motor_pid_turn *__motor_pid_turn_make(F32 toleracean,enum right_or_left direction){
motor_pid_turn *__motor_pid_turn_make(float toleracean,enum right_or_left direction){
  return motor_pid_turn_make(toleracean,0.7,0.3,0.08,direction);
}


//motor_pid_turn *motor_pid_turn_make(F32 toleracean,double kp,double ki,double kd,enum right_or_left direction){
motor_pid_turn *motor_pid_turn_make(float toleracean,double kp,double ki,double kd,
				    enum right_or_left direction){
  motor_pid_turn *new_motor_pid_turn = (motor_pid_turn *) malloc(sizeof(motor_pid_turn));
  new_motor_pid_turn->my_pid  = pid_make(kp,ki,kd,0);
  new_motor_pid_turn->direction = direction;
  //  new_motor_pid_turn->last_angle_right_wheel = nxt_motor_get_count(NXT_PORT_B);
  new_motor_pid_turn->last_angle_left_wheel = ev3_motor_get_counts(right_motor);

  //  new_motor_pid_turn->last_angle_left_wheel = nxt_motor_get_count(NXT_PORT_C);
  new_motor_pid_turn->last_angle_left_wheel = ev3_motor_get_counts(left_motor);

  new_motor_pid_turn->toleracean = toleracean;

  return new_motor_pid_turn;
}

void motor_pid_turn_init(){

}


//_________________解放_________________________
void motor_pid_turn_free(motor_pid_turn *target_motor_pid_turn){
  pid_free(target_motor_pid_turn->my_pid);
  free(target_motor_pid_turn);
}

//__________________更新____________________
void motor_pid_turn_update(motor_pid_turn *target_motor_pid_turn){
  //int motor_aoc_b = nxt_motor_get_count(NXT_PORT_B) -
  //  target_motor_pid_turn->last_angle_right_wheel;
  int motor_aoc_b = ev3_motor_get_counts(right_motor) -
    target_motor_pid_turn->last_angle_right_wheel;

  //int motor_aoc_c = nxt_motor_get_count(NXT_PORT_C) -
  //  target_motor_pid_turn->last_angle_left_wheel;
  int motor_aoc_c = ev3_motor_get_counts(left_motor) -
    target_motor_pid_turn->last_angle_left_wheel;

  switch(target_motor_pid_turn->direction){
  case right:
    motor_aoc_b = motor_aoc_b * target_motor_pid_turn->toleracean;
    break;
  case left:
    motor_aoc_c = motor_aoc_c * target_motor_pid_turn->toleracean;
    break;
  }
  int diff = motor_aoc_b - motor_aoc_c;
  pid_input(target_motor_pid_turn->my_pid,diff);
}

double motor_pid_turn_get_turn(motor_pid_turn *target_motor_pid_turn){
  return pid_get_output(*target_motor_pid_turn->my_pid);
}

