//self_localization.h in 2015

//strcture for self_localization
typedef struct self_localization_structure {
  float right_motor_current_angle, right_motor_old_angle, right_motor_rotation_angle;
  float left_motor_current_angle, left_motor_old_angle, left_motor_rotation_angle;
  float wheel_across;
  float between_wheels;
  float right_wheel_moving_distance, left_wheel_moving_distance;
  float moving_distance;
  float turning_angle;
  float current_x, current_y, current_angle;
  flaot old_x, old_y, old_angle;
} self_localization;

//constructor. self_localization_make is used in self_localization_constructor
self_localization* self_localization_constructor(void);
self_localization* self_localization_make(void);

//destructor. self_localization_free is used in self_localization_destructor
void self_localization_destructor(void);
void self_localization_free(self_localization*);

//updating of current coordinates
void self_localization_update(self_localization *);

//display of current coordinates
void self_localization_display_coodinates(self_localization*);

//decision in or out of the circle.
int self_localization_in_circle_of(float, float, float);

//data of self localization
void data_file(void);
