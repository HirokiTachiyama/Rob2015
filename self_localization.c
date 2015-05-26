#include "self_localization.h"
#include "common_definition.h"


void run_forward_back() {

  //setting sensor value of radius to 0
  ev3_motor_reset_counts(right_motor); //right motor
  ev3_motor_reset_counts(left_motor); //left motor
  char str[5];
  while(1){
    ev3_motor_rotate(right_motor, 1, 5, true);
    sprintf(str, "%d", ev3_motor_get_counts(right_motor));
    ev3_lcd_draw_string(str, 0, 20);
    ev3_motor_rotate(left_motor, 1, 5, true);
    sprintf(str, "%d", ev3_motor_get_counts(left_motor));
    ev3_lcd_draw_string(str, 0, 40);

  }
}


/*
void get_black(void) {
  int light_u = ev3_color_sensor_get_reflect(color_sensor);
  int light = (int) light_u;
  char str[10];
  sprintf(str, "get black:%d", light);
  ev3_lcd_draw_string(str, 0, 20);
}
*/
