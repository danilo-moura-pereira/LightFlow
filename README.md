### Link do vídeo explicativo
https://youtu.be/...

# Embarcatech - Projeto Final

### Funcionamento 📋
O programa executa as seguintes operações utilizando a placa BitDogLab:

1. O joystick fornece valores analógicos correspondentes aos eixos X e Y, que são utilizados para:

a. Simular a intensidade do tráfego, permitindo que o tempo do sinal verde seja ajustado proporcionalmente à “demanda” de cada via. 
- O eixo “X” (horizontal) simula o tráfego na via A.
- O eixo “Y” (vertical) simula o tráfego na via B.

2. O botão "A" aciona a interrupção e, ao final do próximo ciclo amarelo, o sistema aciona o sinal de travessia, emitindo um alerta sonoro pelo buzzer, via PWM.

3. O display OLED exibe informações sobre o estado das vias “A” e “B”, o tempo restante do ciclo atual e avisos para os pedestres.

4. A matriz de LEDs WS2812B fornece feedback visual sobre o estado atual dos semáforos (verde ou amarelo para a via "A"; verde escuro ou laranja para a via "B").

5. O LED RGB muda de cor de acordo com o nível de congestionamento: verde para pista livre, amarelo para fluxo normal e vermelho para congestionamento.

### Requisitos cumpridos pelo projeto
1. Ser original, não sendo uma cópia, integral ou parcial, de um projeto encontrado na internet ou outra fonte;
- Seja inovador, tendo em vista de se tratar de semáforos inteligentes no cruzamento de vias;
- Está restrito aos conceitos, características e tecnologias de sistemas embarcado vistos durante a capacitação.
- Utilizar somente a BitDogLab para representar todo o projeto.