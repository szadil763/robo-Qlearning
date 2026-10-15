# Ligações elétricas

Para ver cada fio em 3D, abra a montagem (`montagem-3d.html`, passos 9 a 13).

## Pinagem (ESP32 DevKit)

| Função | Pino ESP32 | Vai para |
|---|---|---|
| Motor esquerdo IN1 | GPIO 26 | TB6612 AIN1 |
| Motor esquerdo IN2 | GPIO 25 | TB6612 AIN2 |
| Motor esquerdo PWM | GPIO 27 | TB6612 PWMA |
| Motor direito IN1 | GPIO 33 | TB6612 BIN1 |
| Motor direito IN2 | GPIO 32 | TB6612 BIN2 |
| Motor direito PWM | GPIO 14 | TB6612 PWMB |
| Standby do driver | GPIO 13 | TB6612 STBY |
| Ultrassônico TRIG | GPIO 5 | HC-SR04 TRIG |
| Ultrassônico ECHO | GPIO 18 | HC-SR04 ECHO **via divisor de tensão** |
| LED verde | GPIO 19 | Resistor 220 Ω, LED, GND |
| LED vermelho | GPIO 23 | Resistor 220 Ω, LED, GND |
| Botão de modo | GPIO 4 | Botão para GND (pull-up interno) |
| Botão de reset do aprendizado | GPIO 16 | Botão para GND (pull-up interno). Toque curto: liga/desliga o modo pensar. Segurar 2 s: apaga a memória |
| OLED SDA | GPIO 21 | OLED SDA |
| OLED SCL | GPIO 22 | OLED SCL |

Se o robô girar para o lado errado ou andar para trás, **inverta os dois fios do motor** correspondente no TB6612 (ou troque o sinal no código).

## Divisor de tensão no ECHO (obrigatório)

O HC-SR04 responde com 5 V, mas os pinos do ESP32 aguentam 3,3 V.

```
HC-SR04 ECHO ──[1 kΩ]──┬── GPIO 18 (ESP32)
                       │
                     [2 kΩ]
                       │
                      GND
```

(Saída ≈ 3,3 V.) O TRIG pode ir direto do ESP32: 3,3 V costuma bastar para disparar o sensor.

## Alimentação

```
2× 18650 (≈7,4 V) ── chave ──┬── TB6612 VM  (motores)
                             └── step-down (ajustado em 5 V) ──┬── ESP32 pino 5V/VIN
                                                               └── HC-SR04 VCC
```

- **TB6612 VCC (lógica)** vai no **3V3** do ESP32. **VM** vai direto na bateria.
- **OLED** em 3V3 do ESP32.
- **GND de tudo junto** (bateria, step-down, ESP32, driver, sensores).
- Motores N20 de 6 V ligados em ~7,4 V: o código limita o PWM a 200/255 (≈5,9 V efetivos). Não aumente `VELOCIDADE` sem medir.
- Nunca ligue o ESP32 pela USB e pela bateria ao mesmo tempo sem saber como seu módulo trata isso: desligue a chave da bateria ao programar.

## Painel Wi-Fi (sem pinos extras)

O ESP32 cria a rede `RoboQL` (senha `robo1234`) e serve o painel em `http://192.168.4.1`. Não precisa de internet nem de fiação nova. Sem Wi-Fi, ponha `WIFI_LIGADO = false` no firmware. Com o Wi-Fi ligado o consumo sobe um pouco: confira se a bateria e o regulador de 5 V aguentam (o rádio tem picos de corrente).

## Testes antes de montar tudo

1. Display aceso e mostrando a tela inicial.
2. Monitor serial (115200) mostrando distâncias coerentes ao aproximar a mão do sensor.
3. Robô **levantado**: conferir cada uma das 4 ações (frente, esquerda, direita, ré).
4. Só então colocar na arena.
