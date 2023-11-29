#include <Arduino_FreeRTOS.h>
#include "funcSirene.h"
#include "funcBotoes.h"
#include "funcJoystick.h"
#include "relogio.h"
#include <queue.h> // Biblioteca de gerenciamento de fila para comunicação entre tarefas
#include <LiquidCrystal.h>

// Declaração de funções que serão utilizadas como tarefas
// *pvParameters indica que essas funções podem receber um ponteiro
// de qualquer tipo de dado, o que é comum em tarefas do FreeRTOS
void TaskSirene(void *pvParameters);
void TaskBotoes(void *pvParameters);
void TaskInterface(void *pvParameters);
void TaskRelogio(void *pvParameters);

// Variáveis para armazenar os identificadores das tarefas
// Útil para realizar operações de controle, como pausar ou retomar uma tarefa
TaskHandle_t TaskSireneHandle;
TaskHandle_t TaskBotoesHandle;
TaskHandle_t TaskInterfaceHandle;
TaskHandle_t TaskSerialHandle;
// Variáveis para armazenar as filas que facilitam a comunicação entre as tarefas
QueueHandle_t buzzerQueue;
QueueHandle_t botoesQueue;
QueueHandle_t joystickQueue;
QueueHandle_t relogioQueue;
QueueHandle_t ajusteRelogioQueue;

// Declarações dos pinos do LCD
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // Configuração de filas (FIFO) utilizadas para a comunicação entre tarefas
  buzzerQueue = xQueueCreate(1,           // Queue length
                             sizeof(int)  // Queue item size
  );

  botoesQueue = xQueueCreate(1,           // Queue length
                             sizeof(int)  // Queue item size
  );

  joystickQueue = xQueueCreate(1,           // Queue length
                               sizeof(int)  // Queue item size
  );

  relogioQueue = xQueueCreate(10,                // Queue length
                              2 * sizeof(int));  // Queue item size

  ajusteRelogioQueue = xQueueCreate(10,                // Queue length
                                    2 * sizeof(int));  // Queue item size

  Serial.begin(115200);
  lcd.begin(16, 2);

  // Criação das tarefas que serão gerenciadas pelo RTOS
  // Parâmetros: ponteiro para a função que implementa a tarefa, tamanho da pilha e prioridade
  xTaskCreate(
    TaskSirene, "Sirene"  // A name just for humans
    ,
    128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,
    NULL, 1  // Priority , with 3 ( configMAX_PRIORITIES - 1) being the highest , and 0 being the lowest.
    ,
    &TaskSireneHandle);

  xTaskCreate(
    TaskBotoes, "Botoes", 128  // Stack size
    ,
    NULL, 1  // Priority
    ,
    &TaskBotoesHandle);

  xTaskCreate(
    TaskInterface, "Interface", 512  // Stack size
    ,
    NULL, 1  // Priority
    ,
    &TaskInterfaceHandle);

  xTaskCreate(
    TaskJoystick, "Joystick", 128  // Stack size
    ,
    NULL, 1  // Priority
    ,
    NULL);

  xTaskCreate(
    TaskRelogio, "Relogio", 128  // Stack size
    ,
    NULL, 1  // Priority
    ,
    NULL);
}

// Loop principal não tem código, o que é comum de sistemas baseados em FreeRTOS
// O RTOS é responsável por agendar e gerenciar a execução das tarefas criadas
void loop() {
  // Empty. Things are done in Tasks.
}

// Gerencia a interface de usuário do despertador, como informações exibidas no LCD
// e processamento de interações de entrada, como botões e joystick
// Atua como uma tarefa contínua dentro do RTOS
void TaskInterface(void *pvParameters) {

  (void)pvParameters;

  int acao;
  int horario[2]; // Minuto e segundo
  int estado_loop = 0; // Máquina de estado
  int loop_hora, loop_min;
  int despertador[2] = {0, 0};
  bool flag = false;


  for (;;)  // A Task shall never return or exit.
  {
    // Define o cursor para coluna 0, linha 1 e exibe a hora atual
    // (linha 1 é a segunda, contagem começa em 0)
    lcd.setCursor(0, 1);
    // Recebe a hora da fila do relógio
    if (xQueueReceive(relogioQueue, &horario, 0) == pdPASS) {
      if (horario[1] < 10) {
        lcd.print("0");
      }
      lcd.print(horario[1]);
      lcd.print(":");
      if (horario[0] < 10) {
        lcd.print("0");
      }
      lcd.print(horario[0]);
    }

    // Máquina de estados
    switch (estado_loop) {
      // Aguarda ação do usuário
      case 0:
        if (xQueueReceive(botoesQueue, &acao, 0) == pdPASS) {
          if (acao == pressionado) { // Se botão pressionado
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 20;
          }
        }
        if (horario[0] == despertador[0] && horario[1] == despertador[1] && flag == true) {
          acao = ligar_sirene; // Aciona a sirene ao horário chegar no definido para o alarme
          xQueueSend(buzzerQueue, &acao, 0);
          estado_loop = 2; // Aguarda para desligar a sirene
        }
        break;

      // Aguardar o usuário pressionar o botão para parar a sirene
      case 2:
        if (xQueueReceive(botoesQueue, &acao, 0) == pdPASS) {
          if (acao == pressionado) {
            acao = desliga_sirene;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 0;
          }
        }
        break;

      // Início do ajuste do despertador
      case 20:
        lcd.setCursor(0, 0);
        lcd.print("Ajuste de hora");
        if (xQueueReceive(joystickQueue, &acao, 0) == pdPASS) {
          // Se mexer no joystick, vai para o ajuste de hora
          if (acao == Apressionadoalto || acao == Apressionadobaixo) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 21;
          }
        }
        if (xQueueReceive(botoesQueue, &acao, 0) == pdPASS) {
          // Se apertar o botão, vai para o ajuste de minuto
          if (acao == pressionado) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 31;
            loop_hora = horario[1];
            loop_min = horario[0];
          }
        }
        break;

      // Ajuste de hora do relógio
      case 21:
        lcd.setCursor(0, 0);
        lcd.print("Despertador");
        // Se mexer o joystick, vai para o modo de ajuste
        if (xQueueReceive(joystickQueue, &acao, 0) == pdPASS) {
          if (acao == Apressionadoalto || acao == Apressionadobaixo) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 20;
          }
        }
        // Se apertar o botão, vai para o ajuste do despertador
        if (xQueueReceive(botoesQueue, &acao, 0) == pdPASS) {
          if (acao == pressionado) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 41;
          }
        }
        break;

      // Ajuste de hora
      case 31:
        lcd.setCursor(0, 0);
        lcd.print("Ajuste de horario");
        lcd.setCursor(8, 1);
        if (loop_hora < 10) {
          lcd.print("0");
        }
        lcd.print(loop_hora);
        lcd.print(":");
        if (loop_min < 10) {
          lcd.print("0");
        }
        lcd.print(loop_min);
        if (xQueueReceive(joystickQueue, &acao, 0) == pdPASS) {
          if (acao == Bpressionadoalto) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            if (loop_hora < 23) {
              loop_hora++;
            }
          } else if (acao == Bpressionadobaixo) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            if (loop_hora > 0) {
              loop_hora--;
            }
          } else if (acao == Apressionadoalto || acao == Apressionadobaixo) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 32;
          }
        }
        if (xQueueReceive(botoesQueue, &acao, 0) == pdPASS) {
          if (acao == pressionado) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 33;
          }
        }
        break;

      // Ajuste de minuto
      case 32:
        lcd.setCursor(0, 0);
        lcd.print("Ajuste de horario");
        lcd.setCursor(8, 1);
        if (loop_hora < 10) {
          lcd.print("0");
        }
        lcd.print(loop_hora);
        lcd.print(":");
        if (loop_min < 10) {
          lcd.print("0");
        }
        lcd.print(loop_min);
        if (xQueueReceive(joystickQueue, &acao, 0) == pdPASS) {
          if (acao == Bpressionadoalto) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            if (loop_min < 59) {
              loop_min++;
            }
          } else if (acao == Bpressionadobaixo) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            if (loop_min > 0) {
              loop_min--;
            }
          } else if (acao == Apressionadoalto || acao == Apressionadobaixo) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 31;
          }
        }
        if (xQueueReceive(botoesQueue, &acao, 0) == pdPASS) {
          if (acao == pressionado) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 33;
          }
        }
        break;

      // Finaliza o ajuste de horário
      case 33:
        horario[1] = loop_hora;
        horario[0] = loop_min;
        // Envia o novo horário para a fila do relógio
        xQueueSend(ajusteRelogioQueue, &horario, 0);
        estado_loop = 0;
        break;

      // Semelhante ao 31, mas para o despertador
      case 41:
        lcd.setCursor(0, 0);
        lcd.print("Ajuste de horario");
        lcd.setCursor(8, 1);
        if (despertador[1] < 10) {
          lcd.print("0");
        }
        lcd.print(despertador[1]);
        lcd.print(":");
        if (despertador[0] < 10) {
          lcd.print("0");
        }
        lcd.print(despertador[0]);
        if (xQueueReceive(joystickQueue, &acao, 0) == pdPASS) {
          if (acao == Bpressionadoalto) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            if (despertador[1] < 23) {
              despertador[1]++;
            }
          } else if (acao == Bpressionadobaixo) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            if (despertador[1] > 0) {
              despertador[1]--;
            }
          } else if (acao == Apressionadoalto || acao == Apressionadobaixo) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 42;
          }
        }
        if (xQueueReceive(botoesQueue, &acao, 0) == pdPASS) {
          if (acao == pressionado) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 0;
            lcd.clear();
            flag = true;
          }
        }
        break;

      // Semelhante ao 32, mas para o despertador
      case 42:
        lcd.setCursor(0, 0);
        lcd.print("Ajuste de horario");
        lcd.setCursor(8, 1);
        if (despertador[1] < 10) {
          lcd.print("0");
        }
        lcd.print(despertador[1]);
        lcd.print(":");
        if (despertador[0] < 10) {
          lcd.print("0");
        }
        lcd.print(despertador[0]);
        if (xQueueReceive(joystickQueue, &acao, 0) == pdPASS) {
          if (acao == Bpressionadoalto) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            if (despertador[0] < 59) {
              despertador[0]++;
            }
          } else if (acao == Bpressionadobaixo) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            if (despertador[0] > 0) {
              despertador[0]--;
            }
          } else if (acao == Apressionadoalto || acao == Apressionadobaixo) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 41;
          }
        }
        if (xQueueReceive(botoesQueue, &acao, 0) == pdPASS) {
          if (acao == pressionado) {
            acao = ligar_bip;
            xQueueSend(buzzerQueue, &acao, 0);
            estado_loop = 0;
            lcd.clear();
          }
        }
        break;
    }
  }
}