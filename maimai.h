/**
 * 2015/05/07
 * ���M
 * U8 -> unsignd char
 **/

#ifndef __MAIMAI__
#define __MAIMAI__
#include "pid.h"
#include "common_definition.h"
enum light_state {on,off};

typedef struct maimai_structure{
  //U16 on_amount_of_light;
  //U16 char off_amount_of_light;
  unsigned short  on_amount_of_light;
  unsigned short  off_amount_of_light;
  int counter;
  enum light_state state;
  int light_on_del;
  int light_off_del;
  double luminance;
  double turn;
  struct pid_structure *pid_maimai;
}maimai;

maimai *__maimai_make();
maimai *__maimai_make_three_point();
maimai *maimai_make(enum light_state first_state,int on_del,int off_del,double p_gain,double i_gain,double d_gain,double light_target);
void maimai_free(maimai *target_maimai);
void maimai_change_on_off(maimai *target_maimai,enum light_state next_state);
void maimai_get_light_sensor(maimai *target_maimai);
void maimai_update(maimai *target_maimai);
void maimai_luminomater(maimai *target_maimai);
double maimai_get_turn(maimai *target_maimai);
enum yes_or_no maimai_on_line(maimai *target_maimai);
//________________bluetuthで輝度をPCに送信___________________
void maimai_send_luminance(maimai *target_maimai);

#endif
