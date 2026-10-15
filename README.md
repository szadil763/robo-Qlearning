# Robô Desviador de Obstáculos com Q-learning

Robô didático com peças impressas em 3D que **aprende sozinho a desviar de obstáculos** usando aprendizado por reforço (Q-learning). Feito para mostrar a alunos, de forma visível, como uma inteligência artificial aprende por tentativa e erro.

> Este é o **Robô 1** de uma dupla. O **Robô 2** (identificador de gestos/cartões, aprendizado supervisionado) fica em um projeto separado.

## O que o aluno enxerga

- No começo o robô **bate em tudo** (cérebro vazio, tabela Q zerada).
- A tabela Q aparece no display OLED e **os números mudam em tempo real**.
- LED **verde** = recompensa, LED **vermelho** = punição.
- O contador "bat/50" (batidas a cada 50 passos) **cai** com o tempo: é a prova de que ele aprendeu.
- Botão de modo: alterna entre **APRENDENDO** e **DEMONSTRAÇÃO** (usa o que aprendeu, sem explorar).
- Botão de reset (segurar 2 s): apaga a memória e o robô volta a ser "bebê" para repetir o experimento.
- **Modo pensar** (toque curto no botão de reset): o display mostra, etapa por etapa, o que o robô pensa: 1/5 percebe (distância e estado), 2/5 escolhe (o "dado" contra o epsilon: explorar ou aproveitar), 3/5 age, 4/5 recompensa e 5/5 aprende (a conta do Q-learning com os números reais: nota velha → nota nova). Desligado, o display volta a mostrar a tabela Q inteira, e o robô anda mais rápido. O tempo de cada tela é `PENSAR_MS` no bloco `AJUSTES`.
- **Painel Wi-Fi:** o robô cria a rede `RoboQL` (senha `robo1234`, mude em `painel.h`/`AJUSTES`). Abra `http://192.168.4.1` no celular ou computador para ver a tabela Q e o raciocínio ao vivo (dá para projetar), mudar α, γ, ε e as recompensas **sem regravar o código**, baixar o histórico (CSV) e salvar/carregar o cérebro (JSON).
- A ilustração 3D tem o mesmo painel **Mente do robô**, com barras das notas, o dado, a recompensa e a fórmula preenchida, além do histórico dos últimos pensamentos.

## Como funciona (resumo)

| Conceito de IA | No robô |
|---|---|
| Estado | Distância à frente em 4 faixas (muito perto, perto, médio, livre) |
| Ação | Frente, girar esquerda, girar direita, ré |
| Recompensa | −10 ao bater, +2 ao andar livre, pequenas penalidades para girar/ré |
| Tabela Q | 4 estados × 4 ações = 16 notas, mostradas no OLED |
| Explorar × aproveitar | Começa 100% aleatório e vai reduzindo (epsilon) até 5% |

## Estrutura da pasta

```
robo-desviador-ql/
├── README.md
├── CLAUDE.md                  contexto para trabalhar com Claude Code
├── firmware/robo_desviador/   código Arduino (ESP32): .ino + painel.h (Wi-Fi)
├── cad/chassi_desviador.scad  chassi paramétrico (OpenSCAD)
└── docs/
    ├── lista-de-materiais.md
    ├── ligacoes.md            pinagem e alimentação
    ├── roteiro-de-aula.md     2 aulas de 50 min
    ├── atividades/            fichas de experimento para os alunos
    └── ilustracao-3d.html     ilustração 3D interativa (abre no navegador)
```

## Para a sala de aula

- `docs/atividades/`: 3 fichas de experimento (recompensas; α, γ e ε; pensando como o robô) com quadro de resultados e critérios de avaliação.
- **Laboratório** na ilustração 3D e no painel Wi-Fi: os alunos mudam as regras e veem o efeito.
- **Adivinhe a ação:** o simulador pausa antes de cada escolha e o aluno tenta prever.
- **Cérebro (JSON):** treine no simulador e carregue no robô (ou o contrário). Serve de plano B quando falta tempo de aula.

## Início rápido

1. Monte conforme `docs/lista-de-materiais.md` e `docs/ligacoes.md`.
2. No Arduino IDE, instale o **núcleo ESP32 versão 3.x** (Gerenciador de Placas) e as bibliotecas **Adafruit SSD1306** e **Adafruit GFX**.
3. Abra `firmware/robo_desviador/robo_desviador.ino`, selecione "ESP32 Dev Module" e envie.
4. Teste primeiro com o robô **levantado** (rodas no ar) para conferir o sentido dos motores.
5. Calibre os tempos de ação no bloco `AJUSTES` do código (giro de ~45–90° por ação é um bom alvo).
6. Solte na arena e acompanhe a tabela mudando.

## Arena sugerida

Cerca fechada de MDF (corte a laser ou chapa comum), cerca de 1,2 × 0,8 m e 15 cm de altura, com 4 a 6 obstáculos soltos (na ilustração 3D são 6). Paredes lisas e retas dão leituras melhores no ultrassônico.

## Extensão: labirinto

A ilustração 3D (`docs/ilustracao-3d.html`) tem um cenário **Labirinto** (6 × 4 células de 20 cm): o robô aprende, tentativa após tentativa, o caminho do INÍCIO ao FIM com o mesmo Q-learning. Aqui o estado é **célula + direção** (96 estados × 3 ações: frente, girar à esquerda, girar à direita), com recompensa +100 ao chegar, −5 ao bater na parede e −1 por ação.

Importante: isso **não roda no robô atual**. O desviador só tem o ultrassônico, que não informa em que célula ele está. Para o labirinto real o robô precisa saber a posição, por exemplo com **encoders nas rodas** (odometria, corrigida ao encostar nas paredes) ou **marcas no chão** lidas por sensor. O firmware do labirinto ainda não foi escrito.

## Avisos honestos

- É aprendizado por reforço **tabular**, simples de propósito: dá para explicar cada linha. Não é deep learning.
- Tempo típico para comportamento razoável: de 5 a 20 minutos de arena. Vale ter o robô **pré-treinado** salvo como plano B (a memória fica gravada no ESP32 e sobrevive ao desligamento).
- O ultrassônico erra em superfícies inclinadas ou macias (tecido, por exemplo) e em quinas. Isso é ótimo para discutir "dados ruidosos" com a turma.
- O firmware e o chassi foram escritos sem testar no hardware: espere ajustar pinos, tempos e medidas na primeira montagem.

## Licença

A definir (sugestão: CC BY-NC-SA para materiais didáticos e MIT para o código).
