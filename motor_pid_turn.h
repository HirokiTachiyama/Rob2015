/* 2015/05/07
 * 加筆
 * コンストラクタに使用されていたF32という型が
 * EV3ではコンパイル出来なかった為、floatならいける？
 * と推測し、書き換え
 */

#ifndef __MOTOR_PID_TURN__
#define __MOTOR_PID_TURN__

#include "pid.h"
#include "common_definition.h"
typedef struct motor_pid_turn_tructure{
  pid *my_pid;
  int last_angle_right_wheel;
  int last_angle_left_wheel;
  //  F32 toleracean;
    float toleracean;
  enum right_or_left direction;
}motor_pid_turn;


//_____________________コンストラクタ_______________________
//motor_pid_turn *__motor_pid_turn_make(F32 toleracean,enum right_or_left direction);
//motor_pid_turn *motor_pid_turn_make(F32 toleracean,double kp,double ki,double kd,
//				    enum right_or_left direction);
motor_pid_turn *__motor_pid_turn_make(float toleracean,enum right_or_left direction);
motor_pid_turn *motor_pid_turn_make(float toleracean,double kp,double ki,double kd,
				    enum right_or_left direction);


//______________________解放________________________________
void motor_pid_turn_free(motor_pid_turn *target_motor_pid_turn);
//______________________4ms毎に更新すること__________________
void motor_pid_turn_update(motor_pid_turn *target_motor_pid_turn);
//____________________この関数を用いてturnの値を取り出すこと________
double motor_pid_turn_get_turn(motor_pid_turn *target_motor_pid_turn);

#endif
