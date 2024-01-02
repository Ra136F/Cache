// 实现文件中定义全局变量
// global_variables.cpp

#include "global_variables.h"

int turn = 0;  // q-learning训练轮次
int lastAction = 0; //上一次执行的动作，1为写入smr，0为其他
int currentTime = 0; //当前动作时间
int lastTime = 0; //上一次动作时间
int llTime = 0; //上上次动作时间
double qTable[4][4][2][2] = {0};