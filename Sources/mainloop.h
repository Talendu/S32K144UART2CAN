/*
 * mainloop.h
 *
 *  Created on: 2018年4月8日
 *      Author: Administrator
 */

#ifndef MAINLOOP_H_
#define MAINLOOP_H_

#include "S32K144.h"
#include "initialization.h"
#include "sysmode.h"


/*
 * \brief   主循环, 初始化完成后进入该循环, 非意外情况不会退出
 */
void mainloop(void);

#endif /* MAINLOOP_H_ */
