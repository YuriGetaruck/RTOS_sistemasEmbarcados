#include "funcJoystick.h"

void TaskJoystick(void *pvParameters) { // This is a task.
  (void)pvParameters;
  int estado_botao = 0; // Máquina de estados
  int counta = 0, countb = 0, countc = 0, countd = 0; // Contadores das posições do joystick
  int _portaA = A14; // Pinos do Arduino ligados no joystick
  int _portaB = A15;
  int acao; // Envia uma ação para a fila joystickQueue

  pinMode(_portaA, INPUT);
  pinMode(_portaB, INPUT);

  // Loop principal lê as portas correspondentes aos eixos do joystick
  // Processo semelhante ao do botão, mas para os dois e eixos e para cada uma das direções
  for (;;) {
    Serial.println(analogRead(_portaB));
    switch (estado_botao) {
      case 0:
        // ************* altos *************
        if (analogRead(_portaA) > 750) {
          counta++;
          if (counta > 5) {
            estado_botao = 1;
          }
        } else {
          counta = 0;
        }
        // ************** baixos ************
        if (analogRead(_portaA) < 250) {
          countb++;
          if (countb > 5) {
            estado_botao = 4;
          }
        } else {
          countb = 0;
        }
        // ************* altos *************
        if (analogRead(_portaB) > 750) {
          countc++;
          if (countc > 5) {
            estado_botao = 6;
          }
        } else {
          countc = 0;
        }
        // ************** baixos ************
        if (analogRead(_portaB) < 250) {
          countd++;
          if (countd > 5) {
            estado_botao = 8;
          }
        } else {
          countd = 0;
        }
        break;

      case 1:
        if (analogRead(_portaA) <= 750) {
          counta++;
          if (counta > 5) {
            estado_botao = 2;
          }
        } else {
          counta = 0;
        }
        break;

      case 2:
        acao = Apressionadoalto;
        xQueueSend(joystickQueue, &acao, 0);
        estado_botao = 0;
        break;
      case 4:
        if (analogRead(_portaA) >= 250) {
          countb++;
          if (countb > 5) {
            estado_botao = 5;
          }
        } else {
          countb = 0;
        }
        break;

      case 5:
        acao = Apressionadobaixo;
        xQueueSend(joystickQueue, &acao, 0);
        estado_botao = 0;
        break;
        
      case 6:
        if (analogRead(_portaB) <= 750) {
          countc++;
          if (countc > 5) {
            estado_botao = 7;
          }
        } else {
          countc = 0;
        }
        break;

      case 7:
        acao = Bpressionadoalto;
        xQueueSend(joystickQueue, &acao, 0);
        estado_botao = 0;
        break;

      case 8:
        if (analogRead(_portaB) >= 250) {
          countd++;
          if (countd > 5) {
            estado_botao = 9;
          }
        } else {
          countd = 0;
        }
        break;

      case 9:
        acao = Bpressionadobaixo;
        xQueueSend(joystickQueue, &acao, 0);
        estado_botao = 0;
        break;
    }
  }
}