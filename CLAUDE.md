# CLAUDE.md

Contexto para quem (ou qual agente) for continuar este projeto.

## Objetivo
Robô didático que aprende a desviar de obstáculos com Q-learning, para ensinar a alunos (8º ano à 3ª série do Ensino Médio) como funciona o aprendizado de máquina por reforço. A clareza do código e do que aparece no display importa mais do que o desempenho.

## Idioma
Código, comentários, documentação e mensagens em **português do Brasil**.

## Stack
- ESP32 DevKit, Arduino-ESP32 core 3.x (usa `ledcAttach`/`ledcWrite` com pino; no core 2.x a API é diferente).
- Bibliotecas: Adafruit SSD1306, Adafruit GFX, Preferences (nativa).
- Chassi em OpenSCAD, impressão em PLA (TPU opcional para pneus).

## Regras do projeto
- Manter o algoritmo **tabular e legível**. Não trocar por rede neural sem pedido explícito.
- Constantes ajustáveis ficam no bloco `AJUSTES` do `.ino`; não espalhar números mágicos.
- Se mudar qualquer pino no firmware, **atualizar `docs/ligacoes.md`** na mesma alteração.
- Se mudar estados/ações (tamanho da tabela Q), conferir o layout do OLED (`desenhar()`), o tamanho salvo na memória (`salvar/carregar`) e o roteiro de aula.
- O ECHO do HC-SR04 é 5 V e o ESP32 é 3,3 V: manter o divisor de tensão documentado.

## Pendências conhecidas
- Nada foi testado em hardware ainda: validar pinos, sentido dos motores, tempos de ação e medidas do chassi.
- Possível melhoria: estado extra com a última ação, ou 2 sensores para distinguir esquerda/direita.
- Possível melhoria: envio da tabela Q por Wi-Fi para um painel no computador/projetor.

- Labirinto: só existe na ilustração 3D (`docs/ilustracao-3d.html`, simulação em JS). Falta o firmware, que exige localização (encoders ou marcas no chão). Manter os parâmetros da simulação (ALFA 0,5, GAMA 0,95, 150 passos por tentativa) em sincronia se o firmware for criado.

- Modo pensar: as 5 telas do OLED (`telaPerceber`…`telaAprender`) espelham o painel "Mente do robô" e o OLED da ilustração 3D. Se mudar o ciclo ou a fórmula, atualizar os três. O botão de reset tem toque curto (alterna o modo pensar) e 2 s (apaga a memória): manter a documentação em dia.

## Projeto irmão
Robô 2 (identificador de gestos/cartões, aprendizado supervisionado) será um repositório separado. Manter o mesmo padrão de pastas.
