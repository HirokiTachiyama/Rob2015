/*
 *�X�V����
 * 2015/05/07
 * motor_port_t�^��left~tail_motor�𑼂̃t�@�C����extern����ׁA
 * static�C���q���O����
 *
 *2015/05/26
 *���Ȉʒu��������������Ă݂�
 *self_localization.h
 *self_localization.c
 *��ǉ��Apid���s�����Ȃ��猻�ݍ��W���v�Z���A
 *�ڕW���W�ɒ�������(�ʂ�߂�����)��~����悤�ɂ��Ă݂�(�\��)
  */

/**
 ******************************************************************************
 ** �t�@�C���� : app.c
 **
 ** �T�v : 2�֓|���U�q���C���g���[�X���{�b�g��TOPPERS/HRP2�pC�T���v���v���O����
 **
 ** ���L : sample_c4 (sample_c3��Bluetooth�ʐM�����[�g�X�^�[�g�@�\��ǉ�)
 ******************************************************************************
 **/

#include "ev3api.h"
#include "app.h"
#include "balancer.h"

#include "pid.h"
#include "self_localization.h"

#if defined(BUILD_MODULE)
#include "module_cfg.h"
#else
#include "kernel_cfg.h"

#endif

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif


/**
 * �Z���T�[�A���[�^�[�̐ڑ����`���܂�
 */
const sensor_port_t
    touch_sensor    = EV3_PORT_1,
    sonar_sensor    = EV3_PORT_2,
    color_sensor    = EV3_PORT_3,
    gyro_sensor     = EV3_PORT_4;

const motor_port_t
    left_motor      = EV3_PORT_C,
    right_motor     = EV3_PORT_B,
    tail_motor      = EV3_PORT_A;

static int      bt_cmd = 0;     /* Bluetooth�R�}���h 1:�����[�g�X�^�[�g */
static FILE     *bt = NULL;     /* Bluetooth�t�@�C���n���h�� */

/* ���L�̃}�N���͌�/���ɍ��킹�ĕύX����K�v������܂� */
/* sample_c1�}�N�� */
#define LIGHT_WHITE 35 //���F�̌��Z���T�l
#define LIGHT_BLACK 1  //���F�̌��Z���T�l
#define GYRO_OFFSET 0          /* �W���C���Z���T�I�t�Z�b�g�l(�p���x0[deg/sec]��) */

/* sample_c2�}�N�� */
#define SONAR_ALERT_DISTANCE 30 /* �����g�Z���T�ɂ���Q�����m����[cm] */
/* sample_c3�}�N�� */
#define TAIL_ANGLE_STAND_UP  93 /* ���S��~���̊p�x[�x] */
#define TAIL_ANGLE_DRIVE      3 /* �o�����X���s���̊p�x[�x] */
#define P_GAIN             2.5F /* ���S��~�p���[�^������W�� */
#define PWM_ABS_MAX          60 /* ���S��~�p���[�^����PWM��΍ő�l */
/* sample_c4�}�N�� */
//#define DEVICE_NAME     "ET0"  /* Bluetooth�� hrp2/target/ev3.h BLUETOOTH_LOCAL_NAME�Őݒ� */
//#define PASS_KEY        "1234" /* �p�X�L�[    hrp2/target/ev3.h BLUETOOTH_PIN_CODE�Őݒ� */
#define CMD_START         '1'    /* �����[�g�X�^�[�g�R�}���h */

/* LCD�t�H���g�T�C�Y */
#define CALIB_FONT (EV3_FONT_SMALL)
#define CALIB_FONT_WIDTH (6/*TODO: magic number*/)
#define CALIB_FONT_HEIGHT (8/*TODO: magic number*/)

/* �֐��v���g�^�C�v�錾 */
static int sonar_alert(void);
static void tail_control(signed int angle);


/*���R��API�ŗV��ō�����֐�
 *�v���O�������N�������ҋ@����ENTER�{�^����
 *�`�����������̂�
 */

void sing_charumera(void) {
  ev3_speaker_set_volume(1);
  ev3_speaker_play_tone(NOTE_C4, 300);
  tslp_tsk(300);
  ev3_speaker_play_tone(NOTE_D4, 300);
  tslp_tsk(300);
  ev3_speaker_play_tone(NOTE_E4, 600);
  tslp_tsk(600);
  ev3_speaker_play_tone(NOTE_D4, 200);
  tslp_tsk(200);
  ev3_speaker_play_tone(NOTE_C4, 200);
  tslp_tsk(200);

  ev3_speaker_play_tone(NOTE_C4, 200);
  tslp_tsk(200);
  ev3_speaker_play_tone(NOTE_D4, 200);
  tslp_tsk(200);
  ev3_speaker_play_tone(NOTE_E4, 200);
  tslp_tsk(200);
  ev3_speaker_play_tone(NOTE_D4, 200);
  tslp_tsk(200);
  ev3_speaker_play_tone(NOTE_C4, 200);
  tslp_tsk(200);
  ev3_speaker_play_tone(NOTE_D4, 600);
  tslp_tsk(600);
}

void get_black(void) {
  int light_u = ev3_color_sensor_get_reflect(color_sensor);
  int light = (int) light_u;
  char str[10];
  sprintf(str, "get black:%d", light);
  ev3_lcd_draw_string(str, 0, 20);
}

void get_white(void) {
  uint8_t light_u = ev3_color_sensor_get_reflect(color_sensor);
  int light = (int) light_u;
  char str[10];
  sprintf(str, "get white:%d", light);
  ev3_lcd_draw_string(str, 0, 30);
}

void get_gyrooffset(void) {
  uint8_t gyro_offset_t = ev3_gyro_sensor_get_rate(GYRO_SENSOR);
  int gyro_offset = (int) gyro_offset_t;
  char str[10];
  sprintf(str, "gyro offset:%d", gyro_offset);
  ev3_lcd_draw_string(str, 0, 20);
}



extern void test_ev3_motor_rotate();


/* ���C���^�X�N */
void main_task(intptr_t unused) {
    signed char forward;      /* �O��i���� */
    signed char turn;         /* ���񖽗� */
    signed char pwm_L, pwm_R; /* ���E���[�^PWM�o�� */

    /* LCD��ʕ\�� */
    ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);
    ev3_lcd_draw_string("EV3way ETRobcon 2015 KatLab", 0, CALIB_FONT_HEIGHT*1);

    /* �Z���T�[���̓|�[�g�̐ݒ� */
    ev3_sensor_config(sonar_sensor, ULTRASONIC_SENSOR);
    ev3_sensor_config(color_sensor, COLOR_SENSOR);
    ev3_color_sensor_get_reflect(color_sensor); /* ���˗����[�h */
    ev3_sensor_config(touch_sensor, TOUCH_SENSOR);
    ev3_sensor_config(gyro_sensor, GYRO_SENSOR);
    /* ���[�^�[�o�̓|�[�g�̐ݒ� */
    ev3_motor_config(left_motor, LARGE_MOTOR);
    ev3_motor_config(right_motor, LARGE_MOTOR);
    ev3_motor_config(tail_motor, LARGE_MOTOR);
    ev3_motor_reset_counts(tail_motor);

    /* Open Bluetooth file */
    bt = ev3_serial_open_file(EV3_SERIAL_BT);
    assert(bt != NULL);

    /* Bluetooth�ʐM�^�X�N�̋N�� */
    act_tsk(BT_TASK);

    ev3_led_set_color(LED_ORANGE); /* �����������ʒm */

    /* �X�^�[�g�ҋ@ */
    while(1) {
      tail_control(TAIL_ANGLE_STAND_UP); /* ���S��~�p�p�x�ɐ��� */
      if (bt_cmd == 1) break; /* �����[�g�X�^�[�g */
      if (ev3_touch_sensor_is_pressed(touch_sensor) == 1) break; /* �^�b�`�Z���T�������ꂽ */
      if(ev3_button_is_pressed(ENTER_BUTTON)) sing_charumera();
      if(ev3_button_is_pressed(UP_BUTTON)) get_white();
      if(ev3_button_is_pressed(DOWN_BUTTON)) get_black();
      if(ev3_button_is_pressed(LEFT_BUTTON)) get_gyrooffset();
      //      if(ev3_button_is_pressed(RIGHT_BUTTON))

      tslp_tsk(10); /* 10msec�E�F�C�g */
    }

    /* ���s���[�^�[�G���R�[�_�[���Z�b�g */
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);

    /* �W���C���Z���T�[���Z�b�g */
    ev3_gyro_sensor_reset(gyro_sensor);
    balance_init(); /* �|���U�qAPI������ */

    ev3_led_set_color(LED_GREEN); /* �X�^�[�g�ʒm */


    //PID�p�\���� ���C�g�̒l�Ɏg��
    pid* my_pid = pid_make(2.0, 0.1, 0.02, (LIGHT_WHITE + LIGHT_BLACK)/2);
    static unsigned int point1_passed = 0, point2_passed = 0, counter=0;
    //���Ȉʒu����
    self_localization* my_sl = self_localization_constructor();

    //coordinates data for map making.
    FILE* map_fp = fopen("map_coordinates.txt", "w");
    if(map_fp == NULL)
      ev3_lcd_draw_string("file open failured!", 0, 70);

    /**
    * Main loop for the self-balance control algorithm
    */
    while(1) {
      int32_t motor_ang_l, motor_ang_r;
      int  gyro, volt;

      if (ev3_button_is_pressed(BACK_BUTTON)) break; //BACK�{�^���Œ�~

      if (point1_passed == 0)
	tail_control(TAIL_ANGLE_DRIVE); /* �o�����X���s�p�p�x�ɐ��� */
      else
	tail_control(TAIL_ANGLE_STAND_UP);

      /*��Q�����m*/
      if (sonar_alert() == 1) {
        forward = turn = 0; //��Q�������m�������~
      }
      else {
	forward = 35; //�O�i����
        /* if (ev3_color_sensor_get_reflect(color_sensor) >= (LIGHT_WHITE + LIGHT_BLACK)/2) { */
        /*   turn =  20; // �����񖽗� */
        /* } */
        /* else { */
        /*   turn = -20; // �E���񖽗� */
        /* } */
	pid_input(my_pid, ev3_color_sensor_get_reflect(color_sensor));
	turn = (signed char)pid_get_output(*my_pid);

	if(turn >= 100)
	  turn = 100;
	else if(turn <= -100)
	  turn = -100;

	self_localization_update(my_sl);
	self_localization_display_coordinates(my_sl);

	//���ݍ��W���t�@�C���ɏ�������
	self_localization_writing_current_coordinates(map_fp, my_sl);

	if(self_localization_near_target_coordinates(-45.0, -60.0, 10.0, 10.0, my_sl) == 1) {
	  point1_passed = 1;
	  //sing_charumera();
	  ev3_speaker_play_tone(NOTE_C4, 150);
	  //tail_control(TAIL_ANGLE_STAND_UP);
	}

	if(self_localization_near_target_coordinates(60.0, -80.0, 5.0, 10.0, my_sl)==1 && point1_passed == 1){
	  point2_passed = 1;
	  ev3_speaker_play_tone(NOTE_D4, 150);
	}

	if(point1_passed==1 && point2_passed==1){
	  counter++;
	}

	if(counter !=0 && counter < 2000) {
	  forward = 10;
	  turn = 0;
	}
	  /* while(1){ */
	  /*   tail_control(TAIL_ANGLE_STAND_UP); // ���S��~�p�p�x�ɐ��� */
	  /*   sing_charumera(); */
	  /*   forward = 0; */
	  /* } */


	/* ���W�Ƃ��ꂼ��̔��a���w��A�߂Â�����~�߂�
	   if( self_localization_near_target_coordinates(0.0, 0.0, 30.0, 30.0, my_sl) ) {
	   if (hoge > 5000)
	   while(1){
	   tail_control(TAIL_ANGLE_STAND_UP); // ���S��~�p�p�x�ɐ���
	   forward = 0;
	   }
	   else {
	   hoge++;
	   sprintf(hogestr, "%d", hoge);
	   ev3_lcd_draw_string(hogestr, 0, 40);
	   }
	   }
	*/

	/* �|���U�q����API �ɓn���p�����[�^���擾���� */
	motor_ang_l = ev3_motor_get_counts(left_motor);
	motor_ang_r = ev3_motor_get_counts(right_motor);
	gyro = -1 * ev3_gyro_sensor_get_rate(gyro_sensor); /* ���W���C���Z���T�[�̌������t�̂��ߕ������] */
	volt = ev3_battery_voltage_mV();

	/* �|���U�q����API���Ăяo���A�|�����s���邽�߂� */
	/* ���E���[�^�o�͒l�𓾂� */
	if(counter <= 2000){
	  balance_control(
			  (float)forward,
			  (float)turn,
			  (float)gyro,
			  (float)GYRO_OFFSET,
			  (float)motor_ang_l,
			  (float)motor_ang_r,
			  (float)volt,
			  (signed char*)&pwm_L,
			  (signed char*)&pwm_R);
	}
	else if(counter > 2000 && counter <=6250){
	  pwm_L = 23 - (counter*4/1000);
	  pwm_R = 23 - (counter*4/1000);
	}
	
	else if(counter > 6250 && counter <= 7000){
	  pwm_L = pwm_R = 0;
	}
	
	
	/* EV3�ł̓��[�^�[��~���̃u���[�L�ݒ肪���O�ɂł��Ȃ����� */
	/* �o��0���ɁA���̓s�x�ݒ肷�� */
	if (pwm_L == 0)
	  {
	    ev3_motor_stop(left_motor, true);
	  }
	else
	  {
	    ev3_motor_set_power(left_motor, (int)pwm_L);
	  }
	
	if (pwm_R == 0)
	  {
	    ev3_motor_stop(right_motor, true);
	  }
	else
	  {
	    ev3_motor_set_power(right_motor, (int)pwm_R);
	  }
	
	tslp_tsk(4); /* 4msec�����N�� */
      }
    }

    fclose(map_fp);

    ev3_motor_stop(left_motor, false);
    ev3_motor_stop(right_motor, false);

    ter_tsk(BT_TASK);
    fclose(bt);

    ext_tsk();
}

//*****************************************************************************
// �֐��� : sonar_alert
// ���� : ����
// �Ԃ�l : 1(��Q������)/0(��Q������)
// �T�v : �����g�Z���T�ɂ���Q�����m
//*****************************************************************************
static int sonar_alert(void)
{
  static unsigned int counter = 0;
  static int alert = 0;

  signed int distance;

  if (++counter == 40/4) /* ��40msec�������ɏ�Q�����m  */
    {
      /*
       * �����g�Z���T�ɂ�鋗����������́A�����g�̌��������Ɉˑ����܂��B
       * NXT�̏ꍇ�́A40msec�������x���o����̍ŒZ��������ł��B
       * EV3�̏ꍇ�́A�v�m�F
       */
      distance = ev3_ultrasonic_sensor_get_distance(sonar_sensor);
      if ((distance <= SONAR_ALERT_DISTANCE) && (distance >= 0))
        {
	  alert = 1; /* ��Q�������m */
        }
      else
        {
	  alert = 0; /* ��Q������ */
        }
      counter = 0;
    }

  return alert;
}

//*****************************************************************************
// �֐��� : tail_control
// ���� : angle (���[�^�ڕW�p�x[�x])
// �Ԃ�l : ����
// �T�v : ���s�̊��S��~�p���[�^�̊p�x����
//*****************************************************************************
static void tail_control(signed int angle)
{
  float pwm = (float)(angle - ev3_motor_get_counts(tail_motor))*P_GAIN; /* ��ᐧ�� */
  /* PWM�o�͖O�a���� */
  if (pwm > PWM_ABS_MAX)
    {
      pwm = PWM_ABS_MAX;
    }
  else if (pwm < -PWM_ABS_MAX)
    {
      pwm = -PWM_ABS_MAX;
    }

  if (pwm == 0)
    {
      ev3_motor_stop(tail_motor, true);
    }
  else
    {
      ev3_motor_set_power(tail_motor, (signed char)pwm);
    }
}

//*****************************************************************************
// �֐��� : bt_task
// ���� : unused
// �Ԃ�l : �Ȃ�
// �T�v : Bluetooth�ʐM�ɂ�郊���[�g�X�^�[�g�B Tera Term�Ȃǂ̃^�[�~�i���\�t�g����A
//       ASCII�R�[�h��1�𑗐M����ƁA�����[�g�X�^�[�g����B
//*****************************************************************************
void bt_task(intptr_t unused)
{
  while(1)
    {
      uint8_t c = fgetc(bt); /* ��M */
      switch(c)
        {
        case '1':
	  bt_cmd = 1;
	  break;
        default:
	  break;
        }
      fputc(c, bt); /* �G�R�[�o�b�N */
    }
}
