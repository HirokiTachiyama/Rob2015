#include "self_localization.h"
#include "common_definition.h"


void test_ev3_motor_rotate() {
  //setting sensor value of radius to 0
  ev3_motor_reset_counts(right_motor); //right motor
  ev3_motor_reset_counts(left_motor); //left motor
  char str[5];
  while(1){
    ev3_motor_rotate(right_motor, 1, 5, true);
    sprintf(str, "%d", (int)ev3_motor_get_counts(right_motor));
    ev3_lcd_draw_string(str, 0, 20);
    ev3_motor_rotate(left_motor, 1, 5, true);
    sprintf(str, "%d", (int)ev3_motor_get_counts(left_motor));
    ev3_lcd_draw_string(str, 0, 40);

  }
}

//constructor
self_localization* self_localization_constructor(){
  //setting sensor value of radius to 0
  ev3_motor_reset_counts(right_motor); //right motor
  ev3_motor_reset_counts(left_motor); //left motor
  return self_localization_make();
}

self_localization *self_localization_make(void) {
  self_localization *new_self_localization =
    (self_localization*)malloc(sizeof(self_localization));
  new_self_localization->right_motor_current_angle = ev3_motor_get_counts(right_motor);
  new_self_localization->left_motor_current_angle = ev3_motor_get_counts(left_motor);
  new_self_localization->right_motor_old_angle = 0;
  new_self_localization->left_motor_old_angle  = 0;
  new_self_localization->right_motor_rotation_angle = 0;
  new_self_localization->left_motor_rotation_angle = 0;
  new_self_localization->wheel_across = 8.1;
  new_self_localization->between_wheels = 15.5;
  new_self_localization->moving_distance = 0;
  new_self_localization->turning_angle = 0;
  new_self_localization->right_wheel_moving_distance = 0;
  new_self_localization->left_wheel_moving_distance  = 0;
  new_self_localization->current_x = 0;
  new_self_localization->current_y = 0;
  new_self_localization->current_angle = 0;
  new_self_localization->old_x = 0;
  new_self_localization->old_y = 0;
  new_self_localization->old_angle = 0;
  return new_self_localization;
}

void self_localization_free(self_localization *sl) {
  free(sl);
}

void self_localization_update(self_localization *sl){
  //rotation angle of right wheel
  sl->old_x = sl->current_x;
  sl->old_y = sl->current_y;
  sl->old_angle = sl->current_angle;

  sl->right_motor_old_angle = sl->right_motor_current_angle;
  sl->right_motor_current_angle = ev3_motor_get_counts(right_motor);
  sl->right_motor_rotation_angle =
    sl->right_motor_current_angle - sl->right_motor_old_angle;
  sl->right_wheel_moving_distance =
    sl->wheel_across * M_PI * (sl->right_motor_rotation_angle / 360.0);

  //rotation angle of left wheel
  sl->left_motor_old_angle = sl->left_motor_current_angle;
  sl->left_motor_current_angle = ev3_motor_get_counts(left_motor);
  sl->left_motor_rotation_angle =
    sl->left_motor_current_angle - sl->left_motor_old_angle;
  sl->left_wheel_moving_distance =
    sl->wheel_across * M_PI * (sl->left_motor_rotation_angle / 360.0);

  //moving distance and turning angle
  sl->moving_distance =
    (sl->right_wheel_moving_distance + sl->left_wheel_moving_distance) / 2;
  sl->turning_angle = (sl->right_wheel_moving_distance - sl->left_wheel_moving_distance)
    / sl->between_wheels;

  //updating of coordinates
  sl->current_x = sl->old_x + (sl->moving_distance * cos(sl->old_angle + (sl->turning_angle/2)));
  sl->current_y = sl->old_y + (sl->moving_distance * sin(sl->old_angle + (sl->turning_angle/2)));
  sl->current_angle = sl->old_angle + sl->turning_angle;
}

//When ev3 is in the circle which the center is(x, y) and the diameter is 2 * radius, return 1.
//And the others, return 0.
int self_localization_in_circle_of(float x, float y, float radius){
  if( ((sl->current_x*sl->current_x) + (sl->current_y* sl->current_y)) < (x*x + y*y - radius*radius) )
    return 1;
  else
    return 0;
}

void self_localization_display_coodinates(self_localization* sl){
  char str[40];
  sprintf(str, "X:%f Y:%f", sl->current_x, sl->current_y);
  ev3_lcd_draw_string(str, 0, 50);
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

void data_file(void) {
  FILE *fp;
  char s[256];
  if ((fp = fopen("coordinates.txt", "w")) == NULL) {
    ev3_lcd_draw_string("file open error", 0, 30);
  }

  fprintf(fp, "abcde%d\n", 100);

  fclose(fp);
  return;
}


