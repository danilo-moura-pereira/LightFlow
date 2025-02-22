### Link do vídeo explicativo
https://youtu.be/9pYW9pMwoRY

# ADC
Embarcatech - Tarefa 1 - Aula Síncrona 10/02

### Funcionamento 📋
O programa executa as seguintes operações utilizando a placa BitDogLab:

1. O joystick fornece valores analógicos correspondentes aos eixos X e Y, que são utilizados para:

a. Controlar a intensidade luminosa dos LEDs RGB, onde:
- O LED Azul tem seu brilho ajustado conforme o valor do eixo Y. Quando o joystick é solto, o LED apaga. À medida que o joystick é movido para cima (valores menores) ou para baixo (valores maiores), o LED aumenta seu brilho gradualmente, atingindo a intensidade máxima nos extremos (0 e 4095).
- O LED Vermelho segue o mesmo princípio, mas de acordo com o eixo X. Quando o joystick está solto, o LED é apagado. Movendo o joystick para a esquerda (valores menores) ou para a direita (valores maiores), o LED aumenta de brilho, sendo mais intenso nos extremos (0 e 4095).
- Os LEDs são controlados via PWM para permitir variação suave da intensidade luminosa.

b. Exibir no display SSD1306 um quadrado de 8x8 pixels, inicialmente centralizado, que se move proporcionalmente aos valores capturados pelo joystick.

c. Adicionalmente, o botão do joystick tem as seguintes funcionalidades:
- Alterna o estado do LED Verde a cada acionamento.
- Modifica a borda do display para indicar quando foi pressionado, alternando entre diferentes estilos de borda a cada novo acionamento.

d. O botão A ativa/desativa os LED RGB a cada acionamento.

### Requisitos cumpridos pelo projeto
1. Uso de interrupções: Todas as funcionalidades relacionadas aos botões foram implementadas utilizando rotinas de interrupção (IRQ).
2. Debouncing: foi implementado o tratamento do bouncing dos botões via software.
3. Utilização do Display 128 x 64.
4. Organização do código: o código está bem estruturado e comentado para facilitar o entendimento.