#include "funcSirene.h"

void TaskSirene(void *pvParameters)  // This is a task.
{
  (void)pvParameters;
  int acao; // Enviar ação para a fila
  int _porta = 8; // Pino do Arduino ligado ao buzzer
  pinMode(_porta, OUTPUT);
  int estado_sirene = 0; // Máquina de estados

  for (;;)  // A Task shall never return or exit.
  {
    // Verifica se há alguma ação disponível na fila
    // Se sim, é lida e comparada com os estados da sirene
    if (xQueueReceive(buzzerQueue, &acao, 0) == pdPASS) { 
      if (estado_sirene == 5) { // Verifica se a ação recebida é ligar a sirene ou o bip
        if (acao == ligar_sirene) {
          estado_sirene = 1;
        } else if (acao == ligar_bip) {
          estado_sirene = 3;
        }
      } // Verifica se é para desligar a sirene e ela está em um estado válido para desligá-la
      if (acao == desliga_sirene && (estado_sirene == 1 || estado_sirene == 2)) {
        estado_sirene = 4;
      }
    }

    // vTaskDelay (1); // wait for one second

    switch (estado_sirene) {
      case 0:
        estado_sirene = 5;
        break;

      case 1:
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // wait for one second
        tone(_porta, 440);
        Serial.println("LED ligado");
        estado_sirene = 2;
        break;

      case 2:
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // wait for one second
        noTone(_porta);
        Serial.println("LED desligado");
        estado_sirene = 1;
        break;

      case 3:
        tone(_porta, 440);
        estado_sirene = 4;
        break;

      case 4:
        vTaskDelay(100 / portTICK_PERIOD_MS);  // wait for one second
        noTone(_porta);
        estado_sirene = 5;
        break;
      case 5:
        break;
    }
  }
}