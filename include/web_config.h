// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WEB_CONFIG_H
#define WEB_CONFIG_H

#include <Arduino.h>

extern bool setupModeActive;

void initWebServer();
void initWebConfigAP();
void handleWebConfig();
void handleWebServer();

#endif
