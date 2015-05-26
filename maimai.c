#include "maimai.h"

#define MAIMAI_BLACK 0.1075 /* 黒色の光センサ値 */
#define MAIMAI_WHITE 0.7415 /* 白色の光センサ値 */

#define MAIMAI_BLACK_THREE_POINT 0.1575 /* 黒色の光センサ値 */
#define MAIMAI_WHITE_THREE_POINT 0.6556 /* 白色の光センサ値 */

//_______________コンストラクタ__________________________
maimai *__maimai_make(){
  double p_gain = 50.0; //60.0
  double i_gain = 5.0; //0.0
  double d_gain = 2.0; //1.5
  int on_del = 20;
  int off_del = 20;
  double target_light = (MAIMAI_BLACK + MAIMAI_WHITE*6)/7;
  return maimai_make(off,on_del,off_del,p_gain,i_gain,d_gain,target_light);
}

maimai *__maimai_make_three_point(){
  double p_gain = 60.0;
  double i_gain = 0.0;
  double d_gain = 1.5;
  int on_del = 20;
  int off_del = 20;
  double target_light = (MAIMAI_BLACK_THREE_POINT + MAIMAI_WHITE_THREE_POINT*6)/7;
  return maimai_make(off,on_del,off_del,p_gain,i_gain,d_gain,target_light);
}


maimai *maimai_make(enum light_state first_state,int on_del,int off_del,double p_gain,double i_gain,double d_gain,double light_target){
  maimai *new_maimai = (maimai *)malloc(sizeof(maimai));
  new_maimai->on_amount_of_light = 0;
  new_maimai->off_amount_of_light = 0;
  new_maimai->state = first_state;
  new_maimai->light_on_del = on_del;
  new_maimai->light_off_del = off_del;
  new_maimai->luminance = 0;
  new_maimai->turn = 0;
  new_maimai->pid_maimai = pid_make(p_gain,i_gain,d_gain,light_target);
  //pid_change_del(new_maimai->pid_maimai,DEL*10);
  maimai_change_on_off(new_maimai,off);

  return new_maimai;

}

void maimai_free(maimai *target_maimai){
  pid_free(target_maimai->pid_maimai);
  free(target_maimai);
}

void maimai_change_on_off(maimai *target_maimai,enum light_state next_state){
  switch(next_state){
  case on:
    //ecrobot_set_light_sensor_active(NXT_PORT_S3);
    ecrobot_set_light_sensor_active(NXT_PORT_S3); 
    target_maimai->counter = target_maimai->light_on_del;
    target_maimai->state = on;
    break;
  case off:
    ecrobot_set_light_sensor_inactive(NXT_PORT_S3); 
    target_maimai->counter = target_maimai->light_off_del;
    target_maimai->state = off;
    break;
  }

}

//ライトセンサの値取得
void maimai_get_light_sensor(maimai *target_maimai){
  switch(target_maimai->state){
  case on:
    target_maimai->on_amount_of_light = ecrobot_get_light_sensor(NXT_PORT_S3); 
    break;
  case off:
    target_maimai->off_amount_of_light = ecrobot_get_light_sensor(NXT_PORT_S3); 
    break;
  }
}

void maimai_update(maimai *target_maimai){
  maimai_get_light_sensor(target_maimai);
  target_maimai->counter -= (int) (DEL * 1000);
  if(0 < target_maimai->counter)	
    return;

  switch(target_maimai->state){
  case on:
    maimai_change_on_off(target_maimai,off);
    maimai_luminomater(target_maimai);
    pid_input(target_maimai->pid_maimai,target_maimai->luminance);
    target_maimai->turn = pid_get_output(*(target_maimai->pid_maimai));
    break;
  case off:
    maimai_change_on_off(target_maimai,on);
    break;

  }

}

void maimai_luminomater(maimai *target_maimai){

  U16 light_diff;		/* 点灯時と消灯時の変化量 */
  float k;			/* 光センサー非線形補正値 */
  U16 light_upped = target_maimai->on_amount_of_light;
  U16 light_downed = target_maimai->off_amount_of_light;

  /* 光センサーの変化量を計算 */
  if(light_downed - light_upped > 0){
    light_diff = light_downed - light_upped;
  } else {
    light_diff = 0U;		/* 符号なしで全ビット0(ゼロ) */
  }

  /* 光センサー非線形補正係数を計算 （実験データより） */
  k = (1.0382E-3 * light_downed - 6.3295E-1) * light_downed + 1.1024E+2;

  /* コース明度を計算 */
  target_maimai->luminance =  (float)light_diff / k;
}


//____________________出力____________________________
double maimai_get_turn(maimai *target_maimai){
  return target_maimai->turn;
}

void maimai_send_luminance(maimai *target_maimai){
  int int_luminance = (int) (target_maimai->luminance*10000);	
  ecrobot_bt_data_logger(int_luminance/100,int_luminance&100);
}

enum yes_or_no maimai_on_line(maimai *target_maimai){
  if( target_maimai->luminance < (MAIMAI_BLACK_THREE_POINT + MAIMAI_WHITE_THREE_POINT)/2 )
    return yes;
  return no;
}
