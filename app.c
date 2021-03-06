/*
 *更新履歴
 * 2015/05/07
 * motor_port_t型のleft~tail_motorを他のファイルにexternする為、
 * static修飾子を外した
 *
 *2015/05/26
 *自己位置推定を書き加えてみる
 *self_localization.h
 *self_localization.c
 *を追加、pid走行をしながら現在座標を計算しつつ、
 *目標座標に着いたら(通り過ぎたら)停止するようにしてみる(予定)
  */

/**
 ******************************************************************************
 ** ファイル名 : app.c
 **
 ** 概要 : 2輪倒立振子ライントレースロボットのTOPPERS/HRP2用Cサンプルプログラム
 **
 ** 注記 : sample_c4 (sample_c3にBluetooth通信リモートスタート機能を追加)
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
 * センサー、モーターの接続を定義します
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

static int      bt_cmd = 0;     /* Bluetoothコマンド 1:リモートスタート */
static FILE     *bt = NULL;     /* Bluetoothファイルハンドル */

/* 下記のマクロは個体/環境に合わせて変更する必要があります */
/* sample_c1マクロ */
#define LIGHT_WHITE 35 //白色の光センサ値
#define LIGHT_BLACK 1  //黒色の光センサ値
#define GYRO_OFFSET 0          /* ジャイロセンサオフセット値(角速度0[deg/sec]時) */

/* sample_c2マクロ */
#define SONAR_ALERT_DISTANCE 30 /* 超音波センサによる障害物検知距離[cm] */
/* sample_c3マクロ */
#define TAIL_ANGLE_STAND_UP  93 /* 完全停止時の角度[度] */
#define TAIL_ANGLE_DRIVE      3 /* バランス走行時の角度[度] */
#define P_GAIN             2.5F /* 完全停止用モータ制御比例係数 */
#define PWM_ABS_MAX          60 /* 完全停止用モータ制御PWM絶対最大値 */
/* sample_c4マクロ */
//#define DEVICE_NAME     "ET0"  /* Bluetooth名 hrp2/target/ev3.h BLUETOOTH_LOCAL_NAMEで設定 */
//#define PASS_KEY        "1234" /* パスキー    hrp2/target/ev3.h BLUETOOTH_PIN_CODEで設定 */
#define CMD_START         '1'    /* リモートスタートコマンド */

/* LCDフォントサイズ */
#define CALIB_FONT (EV3_FONT_SMALL)
#define CALIB_FONT_WIDTH (6/*TODO: magic number*/)
#define CALIB_FONT_HEIGHT (8/*TODO: magic number*/)

/* 関数プロトタイプ宣言 */
static int sonar_alert(void);
static void tail_control(signed int angle);


/*立山がAPIで遊んで作った関数
 *プログラムを起動させ待機中にENTERボタンで
 *チャルメラを歌う
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


/* メインタスク */
void main_task(intptr_t unused) {
    signed char forward;      /* 前後進命令 */
    signed char turn;         /* 旋回命令 */
    signed char pwm_L, pwm_R; /* 左右モータPWM出力 */

    /* LCD画面表示 */
    ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);
    ev3_lcd_draw_string("EV3way ETRobcon 2015 KatLab", 0, CALIB_FONT_HEIGHT*1);

    /* センサー入力ポートの設定 */
    ev3_sensor_config(sonar_sensor, ULTRASONIC_SENSOR);
    ev3_sensor_config(color_sensor, COLOR_SENSOR);
    ev3_color_sensor_get_reflect(color_sensor); /* 反射率モード */
    ev3_sensor_config(touch_sensor, TOUCH_SENSOR);
    ev3_sensor_config(gyro_sensor, GYRO_SENSOR);
    /* モーター出力ポートの設定 */
    ev3_motor_config(left_motor, LARGE_MOTOR);
    ev3_motor_config(right_motor, LARGE_MOTOR);
    ev3_motor_config(tail_motor, LARGE_MOTOR);
    ev3_motor_reset_counts(tail_motor);

    /* Open Bluetooth file */
    bt = ev3_serial_open_file(EV3_SERIAL_BT);
    assert(bt != NULL);

    /* Bluetooth通信タスクの起動 */
    act_tsk(BT_TASK);

    ev3_led_set_color(LED_ORANGE); /* 初期化完了通知 */

    /* スタート待機 */
    while(1) {
      tail_control(TAIL_ANGLE_STAND_UP); /* 完全停止用角度に制御 */
      if (bt_cmd == 1) break; /* リモートスタート */
      if (ev3_touch_sensor_is_pressed(touch_sensor) == 1) break; /* タッチセンサが押された */
      if(ev3_button_is_pressed(ENTER_BUTTON)) sing_charumera();
      if(ev3_button_is_pressed(UP_BUTTON)) get_white();
      if(ev3_button_is_pressed(DOWN_BUTTON)) get_black();
      if(ev3_button_is_pressed(LEFT_BUTTON)) get_gyrooffset();
      //      if(ev3_button_is_pressed(RIGHT_BUTTON))

      tslp_tsk(10); /* 10msecウェイト */
    }

    /* 走行モーターエンコーダーリセット */
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);

    /* ジャイロセンサーリセット */
    ev3_gyro_sensor_reset(gyro_sensor);
    balance_init(); /* 倒立振子API初期化 */

    ev3_led_set_color(LED_GREEN); /* スタート通知 */


    //PID用構造体 ライトの値に使う
    pid* my_pid = pid_make(2.0, 0.1, 0.02, (LIGHT_WHITE + LIGHT_BLACK)/2);
    static unsigned int point1_passed = 0, point2_passed = 0, counter=0;
    //自己位置推定
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

      if (ev3_button_is_pressed(BACK_BUTTON)) break; //BACKボタンで停止

      if (point1_passed == 0)
	tail_control(TAIL_ANGLE_DRIVE); /* バランス走行用角度に制御 */
      else
	tail_control(TAIL_ANGLE_STAND_UP);

      /*障害物検知*/
      if (sonar_alert() == 1) {
        forward = turn = 0; //障害物を検知したら停止
      }
      else {
	forward = 35; //前進命令
        /* if (ev3_color_sensor_get_reflect(color_sensor) >= (LIGHT_WHITE + LIGHT_BLACK)/2) { */
        /*   turn =  20; // 左旋回命令 */
        /* } */
        /* else { */
        /*   turn = -20; // 右旋回命令 */
        /* } */
	pid_input(my_pid, ev3_color_sensor_get_reflect(color_sensor));
	turn = (signed char)pid_get_output(*my_pid);

	if(turn >= 100)
	  turn = 100;
	else if(turn <= -100)
	  turn = -100;

	self_localization_update(my_sl);
	self_localization_display_coordinates(my_sl);

	//現在座標をファイルに書き込む
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
	  /*   tail_control(TAIL_ANGLE_STAND_UP); // 完全停止用角度に制御 */
	  /*   sing_charumera(); */
	  /*   forward = 0; */
	  /* } */


	/* 座標とそれぞれの半径を指定、近づいたら止める
	   if( self_localization_near_target_coordinates(0.0, 0.0, 30.0, 30.0, my_sl) ) {
	   if (hoge > 5000)
	   while(1){
	   tail_control(TAIL_ANGLE_STAND_UP); // 完全停止用角度に制御
	   forward = 0;
	   }
	   else {
	   hoge++;
	   sprintf(hogestr, "%d", hoge);
	   ev3_lcd_draw_string(hogestr, 0, 40);
	   }
	   }
	*/

	/* 倒立振子制御API に渡すパラメータを取得する */
	motor_ang_l = ev3_motor_get_counts(left_motor);
	motor_ang_r = ev3_motor_get_counts(right_motor);
	gyro = -1 * ev3_gyro_sensor_get_rate(gyro_sensor); /* ※ジャイロセンサーの向きが逆のため符号反転 */
	volt = ev3_battery_voltage_mV();

	/* 倒立振子制御APIを呼び出し、倒立走行するための */
	/* 左右モータ出力値を得る */
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
	
	
	/* EV3ではモーター停止時のブレーキ設定が事前にできないため */
	/* 出力0時に、その都度設定する */
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
	
	tslp_tsk(4); /* 4msec周期起動 */
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
// 関数名 : sonar_alert
// 引数 : 無し
// 返り値 : 1(障害物あり)/0(障害物無し)
// 概要 : 超音波センサによる障害物検知
//*****************************************************************************
static int sonar_alert(void)
{
  static unsigned int counter = 0;
  static int alert = 0;

  signed int distance;

  if (++counter == 40/4) /* 約40msec周期毎に障害物検知  */
    {
      /*
       * 超音波センサによる距離測定周期は、超音波の減衰特性に依存します。
       * NXTの場合は、40msec周期程度が経験上の最短測定周期です。
       * EV3の場合は、要確認
       */
      distance = ev3_ultrasonic_sensor_get_distance(sonar_sensor);
      if ((distance <= SONAR_ALERT_DISTANCE) && (distance >= 0))
        {
	  alert = 1; /* 障害物を検知 */
        }
      else
        {
	  alert = 0; /* 障害物無し */
        }
      counter = 0;
    }

  return alert;
}

//*****************************************************************************
// 関数名 : tail_control
// 引数 : angle (モータ目標角度[度])
// 返り値 : 無し
// 概要 : 走行体完全停止用モータの角度制御
//*****************************************************************************
static void tail_control(signed int angle)
{
  float pwm = (float)(angle - ev3_motor_get_counts(tail_motor))*P_GAIN; /* 比例制御 */
  /* PWM出力飽和処理 */
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
// 関数名 : bt_task
// 引数 : unused
// 返り値 : なし
// 概要 : Bluetooth通信によるリモートスタート。 Tera Termなどのターミナルソフトから、
//       ASCIIコードで1を送信すると、リモートスタートする。
//*****************************************************************************
void bt_task(intptr_t unused)
{
  while(1)
    {
      uint8_t c = fgetc(bt); /* 受信 */
      switch(c)
        {
        case '1':
	  bt_cmd = 1;
	  break;
        default:
	  break;
        }
      fputc(c, bt); /* エコーバック */
    }
}
