#include "relogio.h"

void TaskRelogio(void *pvParameters) {  // This is a task.
  (void)pvParameters;
  int horario[2] = { 0, 0 }; // Horas e minutos
  int _temp_base_tempo = 0; // Tempo decorrido desde a última atualização do horário
  int acao[2];

  for (;;) {
    vTaskDelay(10 / portTICK_PERIOD_MS);  // wait for one second
    _temp_base_tempo++;
    // Serial.println( _temp_base_tempo );
    if (xQueueReceive(ajusteRelogioQueue, &acao, 0) == pdPASS) {
      horario[1] = acao[1];
      horario[0] = acao[0];
    }

    if (_temp_base_tempo == 10) { // Se contou 1 segundo, atualiza o horário
      _temp_base_tempo = 0;
      horario[0]++;
      if (horario[0] >= 60) { // Se minutos maior que 60...
        horario[0] = 0;
        horario[1]++;
        if (horario[1] >= 24) { // Se horas maior que 24...
          horario[1] = 0;
        }
      }
    }
    xQueueSend(relogioQueue, &horario, 0);
  }
}