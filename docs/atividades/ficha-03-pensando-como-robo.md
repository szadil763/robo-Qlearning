# Ficha 03 · Pensando como o robô

**Ideia:** quem entende a conta consegue prever o que a IA vai fazer. Aqui você vai **adivinhar a escolha** do robô e **refazer à mão** a atualização da tabela Q.

## Parte 1 · Adivinhe a ação
1. No simulador, ative **Adivinhe a ação**. A cada passo, o robô pausa antes de escolher.
2. Olhe as notas da situação atual (display *Mente do robô*) e clique na ação que acha que ele vai escolher.
3. Marque no quadro: acertei ou errei, e se o robô **explorou** (sorteio) ou **aproveitou** (maior nota).

| Passo | Situação (0 a 3) | Minha previsão | Escolha do robô | Explorou ou aproveitou? | Acertei? |
|---|---|---|---|---|---|
| | | | | | |

Faça 15 previsões.
- Quando o robô **aproveita**, você acerta quase sempre? Por quê?
- Quando ele **explora**, dá para acertar? Qual é a chance de acertar ao acaso?
- Dê *Treinar 1500 passos* e refaça 15 previsões: o placar muda? Por quê? (Dica: a exploração diminui.)

## Parte 2 · A conta do Q-learning
A fórmula, que o display mostra na etapa 5 (*Aprender*):

> **Q novo = Q velho + α × ( r + γ × melhor Q do próximo estado − Q velho )**

Exemplo com α = 0,3 e γ = 0,9. Estado antes: *perto*, ação *Frente*, Q velho = 1,0. Recompensa r = +2. No estado seguinte, a melhor nota é 1,5.

1. Calcule: `r + γ × melhor Q = 2 + 0,9 × 1,5 = ____`
2. Subtraia o Q velho: `____ − 1,0 = ____`
3. Multiplique por α: `____ × 0,3 = ____`
4. Some ao Q velho: `1,0 + ____ = ____` ← **Q novo**

**Agora com o robô:** pause o simulador na etapa 5 (*Aprender*), copie os números do display e **refaça a conta no caderno**. Bateu com o valor que o robô calculou? Faça 3 vezes.

## Parte 3 · Desafio
1. Com a recompensa **−10** (bateu) e Q velho = 0, α = 0,3, melhor Q seguinte = 0: qual é o Q novo? O robô vai querer repetir essa ação?
2. Se o mesmo erro acontecer 5 vezes seguidas, para onde vai a nota? (Faça a conta passo a passo.)
3. Explique com suas palavras, sem usar a palavra "fórmula", como o robô "decide" o que fazer.

## Para o professor
Respostas da Parte 2: `2 + 0,9 × 1,5 = 3,35` → `3,35 − 1,0 = 2,35` → `2,35 × 0,3 = 0,705` → **Q novo = 1,705**.
Parte 3: Q novo = 0 + 0,3 × (−10 + 0 − 0) = **−3**. Repetindo, a nota segue caindo (−3; −5,1; −6,57…), cada vez mais devagar, e tende a −10 se o próximo estado continuar valendo 0.
