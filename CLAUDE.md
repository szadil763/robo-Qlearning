# CLAUDE.md

Contexto para quem (ou qual agente) for continuar este projeto.

## Objetivo
Robô didático que aprende a desviar de obstáculos com Q-learning, para ensinar a alunos (8º ano à 3ª série do Ensino Médio) como funciona o aprendizado de máquina por reforço. A clareza do código e do que aparece no display importa mais do que o desempenho.

## Idioma
Código, comentários, documentação e mensagens em **português do Brasil**.

## Stack
- ESP32 DevKit, Arduino-ESP32 core 3.x (usa `ledcAttach`/`ledcWrite` com pino; no core 2.x a API é diferente).
- Bibliotecas: Adafruit SSD1306, Adafruit GFX, Preferences (nativa).
- Peças impressas geradas por `cad/gerar_stl.py` (Python: trimesh + manifold3d + shapely + mapbox-earcut), saída em `cad/stl/`. PLA; TPU opcional para pneus. O `.scad` em `cad/legado/` é antigo e não é mantido.

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

- Parâmetros editáveis: `alfa`, `gama`, `epsDecai` e as recompensas (`rBateu`…`rRe`) são variáveis no firmware (valores `*_PADRAO` no bloco AJUSTES), editáveis pelo painel Wi-Fi (`painel.h`) e pelo Laboratório da ilustração 3D. Limites dos controles: manter iguais nos três lugares (3D, `rotaAjustar()` e a página do painel).
- Formato do cérebro (JSON `robo-qlearning-q`, 4 estados × 4 ações) é compartilhado entre simulador e robô: se mudar o tamanho da tabela Q, atualizar `exportBrain/importBrain` (HTML) e `cerebroJson/lerCerebro` (`painel.h`).
- Painel Wi-Fi e fichas nunca foram testados no robô real (só compilação com stubs e testes do simulador).

- STL e montagem 3D: **nunca editar os STL à mão**. Mudou medida/furo/pino no `gerar_stl.py`? Rode `python3 cad/gerar_stl.py` e depois `python3 cad/montar_pagina.py` (recria `docs/montagem-3d.html` a partir de `docs/montagem-3d.template.html`) e versione os STL. Se mudar pinos de parafuso ou componentes, atualizar também os passos e a lista de materiais.
- As peças compradas na ilustração de montagem (motor, pilhas, placa, OLED, LEDs) são formas simples com medidas típicas; **não foram conferidas com os componentes reais**. A peça `08_teste_ajuste.stl` deve ser impressa antes do resto. Nenhum STL foi impresso ainda.
- Espaço entre os suportes do OLED e do sensor: 72 mm para o suporte de 2 pilhas 18650.

## Projeto irmão
Robô 2 (identificador de gestos/cartões, aprendizado supervisionado) será um repositório separado. Manter o mesmo padrão de pastas.

## Site (GitHub Pages)
- `.github/workflows/pages.yml` publica a cada push na `main`: `site/index.html` (entrada), `docs/ilustracao-3d.html` → `simulador.html` e `docs/montagem-3d.html` → `montagem.html`. Não há cópias versionadas desses arquivos: editar sempre os originais em `docs/`.
- As páginas carregam o three.js r128 do cdnjs; se trocar a versão, trocar nas duas.
