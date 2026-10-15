# Peças impressas (STL)

Os arquivos de `stl/` são gerados por `gerar_stl.py` (Python). As medidas ficam no começo do script: mude lá e gere de novo, em vez de editar o STL.

```bash
pip install trimesh manifold3d numpy shapely mapbox-earcut
python3 cad/gerar_stl.py        # recria cad/stl/*.stl, cad/stl/relatorio.json e cad/montagem.json
python3 cad/montar_pagina.py    # recria docs/montagem-3d.html (ilustração de montagem)
```

O script confere que cada malha é fechada (sem furos) e com faces consistentes. **Nenhuma peça foi impressa ainda**: as medidas dos componentes comprados (motor N20, HC-SR04, OLED, suporte 18650) variam entre fornecedores.

## 1º passo: imprima o corpo de prova

`08_teste_ajuste.stl` (~14 cm³, poucos minutos) tem um pedaço de berço de motor, a parede do sensor com os dois furos dos “olhos”, o furo do pino do patim, um furo-guia de parafuso e um disco com furo em D para o eixo.

Teste com as peças reais:

| O que conferir | Se estiver apertado ou frouxo |
|---|---|
| Motor N20 entra no berço com leve pressão | `FOLGA_MOTOR` |
| Olhos do HC-SR04 passam nos furos redondos | `FOLGA_SENSOR` |
| Eixo do motor entra no furo em D | `FOLGA_EIXO` |
| Pino do patim entra no furo | `FOLGA_PINO` |

## Lista de impressão

| Arquivo | Qtd | Material | Como imprimir |
|---|---|---|---|
| `01_chassi.stl` | 1 | PLA | Deitado (placa no leito). Sem suporte. |
| `02_patim.stl` | 1 | PLA | Pino para baixo, esfera para cima. Se a esfera ficar feia, use suporte. |
| `03_suporte_sensor.stl` | 1 | PLA | Face frontal (a dos furos redondos) no leito. Sem suporte. |
| `04_roda.stl` | 2 | PLA | Deitada, lado aberto para cima. Sem suporte. |
| `05_pneu_tpu.stl` | 2 | TPU 95A (opcional) | Deitado, devagar (20 a 30 mm/s). Alternativa: O-ring ou elástico. |
| `06_suporte_oled.stl` | 1 | PLA | Base no leito. A rampa tem 35° de inclinação em relação à vertical, não precisa de suporte. |
| `08_teste_ajuste.stl` | 1 | PLA | Imprima primeiro. |

Sugestão de ajuste (bico 0,4 mm): camada de 0,2 mm, 3 paredes, preenchimento de 20%. Nas rodas e no suporte do sensor, use 4 paredes.

O `relatorio.json` traz tamanho, volume e número de triângulos de cada arquivo.

## Eixos e medidas importantes

- Montagem: X = frente, Y = esquerda, Z = para cima. O fundo da placa fica em Z = 0.
- Roda de 42 mm: o fundo da placa fica a cerca de 11,8 mm do chão (`VAO`).
- Espaço entre o suporte do OLED e o suporte do sensor: 72 mm, para o suporte de 2 pilhas 18650. Meça o seu antes de colar.
- Parafusos: 2× M3×8 (suporte do sensor) e 4× M3×6 (suporte do OLED), em furos-guia de 2,7 mm.

A ilustração `docs/montagem-3d.html` mostra a montagem passo a passo usando as mesmas malhas.

## Legado

`legado/chassi_desviador.scad` é a primeira versão, em OpenSCAD. Nunca foi renderizada e as medidas podem divergir. Não é mantida.
