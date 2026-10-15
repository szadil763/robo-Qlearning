# Lista de materiais

Valores em reais são **estimativas aproximadas** para orçamento. Confira os preços atuais antes de comprar.

| Qtd | Item | Observação | Estimativa (R$) |
|---|---|---|---|
| 1 | ESP32 DevKit (30 pinos) | Qualquer variante com os pinos usados em `ligacoes.md` | 40 a 70 |
| 1 | Driver de motor TB6612FNG | Mais eficiente que o L298N, bom para bateria | 20 a 40 |
| 2 | Motor N20 com redução (6 V, ~100 a 200 rpm) | Eixo de 3 mm, com roda compatível | 20 a 40 cada |
| 2 | Rodas para N20 (~42 mm) | Ou imprimir em 3D com pneu de TPU/borracha | 10 a 20 o par |
| 1 | Sensor ultrassônico HC-SR04 | Versão 5 V comum | 10 a 20 |
| 1 | Display OLED 0,96" I2C (SSD1306, 128×64) | Endereço 0x3C | 20 a 35 |
| 2 | LEDs 5 mm (verde e vermelho) + 2 resistores de 220 Ω | Recompensa e punição | 2 |
| 2 | Botões táteis | Modo e reset | 2 |
| 1 | Resistor de 1 kΩ + 1 de 2 kΩ | Divisor de tensão do ECHO (ver ligações) | 1 |
| 2 | Baterias 18650 + suporte duplo | Ou pack equivalente de ~7,4 V | 30 a 60 |
| 1 | Módulo step-down (MP1584 ou LM2596) ajustado em 5 V | Alimenta ESP32 e sensor | 10 a 20 |
| 1 | Chave liga/desliga | Na linha da bateria | 3 |
| 1 | Protoboard pequena ou placa perfurada | Protoboard para testes; placa perfurada para versão final | 10 a 20 |
| – | Jumpers, 2 abraçadeiras de nylon (~2,5 mm), fita dupla-face ou velcro | Fixação dos motores, das pilhas e da placa | 10 |
| 2 + 4 | Parafusos M3×8 (2, suporte do sensor) e M3×6 (4, suporte do OLED) | Autorroscantes nos furos-guia de 2,7 mm do chassi | 5 |
| 2 | Pneus de TPU impressos (opcional) | Ou um O-ring/elástico grosso de ~36 mm interno | – |
| ~150 g | Filamento PLA | Chassi, rodas e suportes (arquivos em `cad/stl/`); rolo de 1 kg por ~R$ 100 a 160 | 15 a 25 |

**Arena:** chapa de MDF (~1,2 × 0,8 m de piso e paredes de ~15 cm) ou papelão duro. Pode ser cortada no laser.

**Total aproximado do robô:** de R$ 230 a R$ 410, já contando filamento e parafusos (soma das linhas acima, com os 2 motores), dependendo de onde comprar e do que você já tem na bancada. Sem a arena.

## Se for fazer em turma (alunos montando)

- Compre um kit por grupo de 3 a 4 alunos.
- Use a protoboard e jumpers (sem solda) para reduzir o tempo de montagem.
- Deixe um robô "gabarito" já montado e testado.
