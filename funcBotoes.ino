#include "funcBotoes.h"

void TaskBotoes(void *pvParameters) { // This is a task.
  // O ponteiro não é usado, o (void) é para evitar aviso do compilador
  (void)pvParameters;
  int estado_botao = 0; // Variável da máquina de estados
  int count = 0; // Número de iterações do botão em um estado específico
  int _porta = 7; // Pino ao qual o botão está conectado
  int acao; // Envia uma ação para a fila botoesQueue
  pinMode(_porta, INPUT_PULLUP);

  for (;;)
  // A Task shall never return or exit.
  {
    switch (estado_botao) {
      // Verifica se o botão foi pressionado
      // Se sim, incrementa a variável count e muda o estado para 1 ao atingir 50 (debounce)
      case 0:
        // vTaskDelay (1/ portTICK_PERIOD_MS ); // wait for one second
        if (!digitalRead(_porta)) {
          count++;
          Serial.print(count);
          if (count > 50) {
            estado_botao = 1;
          }
        } else {
          count = 0;
        }
        break;
      
      // Verifica se o botão foi liberado
      // Se sim, vai para o próximo estado que configura botão apertado
      case 1:
        // vTaskDelay (1 / portTICK_PERIOD_MS ); // wait for one second
        if (digitalRead(_porta)) {
          Serial.print(count);
          count++;
          if (count > 50) {
            estado_botao = 2;
          }
        } else {
          count = 0;
        }
        break;

      // Botão foi apertado (e solto)
      case 2:
        acao = pressionado; // Botão pressionado
        // Sempre que o botão for pressionado, enviar a ação para a fila
        xQueueSend(botoesQueue, &acao, 0);
        estado_botao = 0; // Reiniciar o ciclo de leitura do botão
        break;
    }
  }
}