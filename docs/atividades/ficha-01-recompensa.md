# Ficha 01 · Quem ensina é a recompensa

**Ideia:** o robô não sabe o que é "bom". Quem decide somos nós, pela **recompensa**. Se mudarmos as notas, o robô aprende outro comportamento.

**Onde mexer:** painel *Laboratório → O que é bom e ruim* (simulador ou painel Wi-Fi do robô).

## Antes de começar
Anote os valores padrão: bater = −10 · frente livre = +2 · ré = −0,5.
Rode *Cérebro vazio* e *Treinar 1500 passos* **uma vez, sem mudar nada**, e anote:
- quantas batidas a cada 50 passos no começo e no fim (gráfico);
- qual é a melhor ação em cada linha da tabela Q (bolinha laranja).

## Experimentos
Para cada um: **escreva a hipótese**, mude **só uma** coisa, dê *Cérebro vazio*, treine, **repita 3 vezes**.

| # | Mude | Pergunta | Hipótese | Resultado (3 rodadas) |
|---|---|---|---|---|
| A | Bater = **0** | Bater deixa de importar? O robô para de desviar? | | |
| B | Ré = **+2** | O que o robô passa a gostar de fazer? Olhe a coluna *Ré* da tabela Q. | | |
| C | Frente com obstáculo perto = **+2** | Premiamos andar para perto das coisas. Ele aprende a bater? | | |
| D | (livre) Invente a sua regra | | | |

## Para discutir
1. No experimento B, o robô "trapaceou"? Ele fez o que pedimos ou o que queríamos?
2. Os resultados foram iguais nas 3 rodadas? Por que não? (Dica: o dado do robô no começo.)
3. Quando damos recompensa para a coisa errada, o que acontece? Dê um exemplo fora da robótica (jogos, redes sociais, escola).
4. Escreva em uma frase: *"Neste robô, o que a IA aprende depende de ______."*

## Pista para o professor
O ponto central é separar **o que pedimos** (recompensa) de **o que queremos** (não bater). Quando a recompensa mal escolhida produz um comportamento estranho, abre-se a discussão sobre alinhamento entre objetivo e recompensa. Não afirme resultados antes dos testes: peça que os grupos descubram e comparem.
