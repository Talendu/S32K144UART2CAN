/*
 * mainloop.h
 *
 *  Created on: 2018��4��8��
 *      Author: Administrator
 */

#ifndef MAINLOOP_H_
#define MAINLOOP_H_

#include "S32K144.h"

void mainloop(void);
void change_mode(void);
void save_config_paramater_to_EEPROM(void);

#endif /* MAINLOOP_H_ */
