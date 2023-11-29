#include "Arduino.h"
#include <Arduino_FreeRTOS.h>

#define pressionado 11
#define naopressionado 22

void TaskBotoes(void *pvParameters); // This is a task.