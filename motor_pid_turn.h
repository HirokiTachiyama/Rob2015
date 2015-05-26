/* 2015/05/07
 * ���M
 * �R���X�g���N�^�Ɏg�p����Ă���F32�Ƃ����^��
 * EV3�ł̓R���p�C���o���Ȃ������ׁAfloat�Ȃ炢����H
 * �Ɛ������A��������
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


//_____________________�R���X�g���N�^_______________________
//motor_pid_turn *__motor_pid_turn_make(F32 toleracean,enum right_or_left direction);
//motor_pid_turn *motor_pid_turn_make(F32 toleracean,double kp,double ki,double kd,
//				    enum right_or_left direction);
motor_pid_turn *__motor_pid_turn_make(float toleracean,enum right_or_left direction);
motor_pid_turn *motor_pid_turn_make(float toleracean,double kp,double ki,double kd,
				    enum right_or_left direction);


//______________________���________________________________
void motor_pid_turn_free(motor_pid_turn *target_motor_pid_turn);
//______________________4ms���ɍX�V���邱��__________________
void motor_pid_turn_update(motor_pid_turn *target_motor_pid_turn);
//____________________���̊֐���p����turn�̒l�����o������________
double motor_pid_turn_get_turn(motor_pid_turn *target_motor_pid_turn);

#endif
