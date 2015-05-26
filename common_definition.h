/* 2015/05/07
 * NXTからEV3への移植のため、APIなどを書き換え
 * 邪魔そうなところをコメントアウト
 * app.cに書いてあるモーターのポート番号用変数をextern
 */

#ifndef __COMMON_DEFINITION__
#define __COMMON_DEFINITION__


#define DEL 0.004
//#define TAIL_ANGLE_STAND_UP 108 /* 完全停止時の角度[度] */
//#define TAIL_ANGLE_DRIVE      3 /* バランス走行時の角度[度] */

//#define TAIL_ANGLE_THREE_RUN  100  /* 3点走行時の角度[度] */
//#define TAIL_ANGLE_LOOK_UP     60  /* ルックアップ時の角度[度] */
//#define TAIL_ANGLE_WAKE_UP     95  /* ルックアップ後復帰時の角度[度] */

#include <stdlib.h>
//#include "kernel.h"
//#include "kernel_id.h"
//#include "ecrobot_interface.h"
#include "balancer.h" /* 倒立振子制御用ヘッダファイル */

#include "ev3api.h"
extern const motor_port_t left_motor, right_motor, tail_motor;

enum right_or_left {right,left};
enum yes_or_no {yes,no};

#endif
