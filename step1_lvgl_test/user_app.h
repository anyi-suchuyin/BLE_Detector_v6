#ifndef USER_APP_H
#define USER_APP_H

#include "src/display/epaper_driver_bsp.h"

// 全局 e-Paper 驱动指针
extern epaper_driver_display *driver;

#ifdef __cplusplus
extern "C" {
#endif

void user_app_init(void);
void user_ui_init(void);

#ifdef __cplusplus
}
#endif

#endif // USER_APP_H
