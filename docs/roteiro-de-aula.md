# Roteiro de aula: "Como uma IA aprende?"

Duração: 2 aulas de 50 minutos. Público: 8º ano ao 3º ano do Ensino Médio (ajustar profundidade).
Materiais: robô desviador, arena, projetor (opcional), papel e lápis, dado de 4 faces (ou aplicativo de sorteio).

## Objetivos

- Entender que **aprender**, para uma IA, é ajustar números com base em recompensas e punições.
- Distinguir **estado, ação, recompensa** e **tabela de valores**.
- Compreender **explorar × aproveitar** e por que o robô começa "burro".
- Relacionar com a Matemática: tabelas/matrizes, médias ponderadas, probabilidade, funções.

## Aula 1: Aprendendo sem computador (50 min)

**1. Provocação (5 min).** Pergunta: "Como um bebê aprende a andar? Alguém explica as regras para ele?" Anote no quadro: *tenta, erra, ajusta*.

**2. Apresentar o robô (5 min).** Mostrar o robô com a memória apagada. Dizer apenas: "Ele não sabe nada. Só sabe medir distância e mexer as rodas." Prever em voz alta: o que vai acontecer?

**3. Atividade desplugada: a tabela no papel (25 min).** Em duplas, cada dupla recebe a tabela vazia 4 estados × 4 ações (todas as notas = 0).
- Um aluno é o **robô**, outro é o **ambiente**. O ambiente diz o estado (muito perto, perto, médio, livre).
- O robô escolhe a ação: no começo, sorteando no dado (explorar). Depois, escolhendo a maior nota (aproveitar).
- O ambiente responde com a recompensa: bater = −10, andar livre = +2, ré = −0,5, girar = −0,2 (ou +1 se abriu caminho).
- A dupla atualiza a nota com a regra simplificada: **nota nova = nota velha + 0,3 × (recompensa − nota velha)**.
- Repetir por 10 a 15 rodadas e observar a tabela mudar.

**4. Fechamento (15 min).** Perguntas:
- Qual ação ficou com a maior nota em "muito perto"? Fez sentido?
- O que acontece se só aproveitarmos e nunca explorarmos?
- Quem ensinou o robô a desviar? (Resposta: ninguém ensinou a regra, só definimos o que é bom e ruim.)

## Aula 2: O robô de verdade (50 min)

**1. Previsão (5 min).** Cada grupo escreve: "Em quantos minutos o robô para de bater?"

**2. Experimento (25 min).** Solte o robô de memória vazia na arena com o display projetado (ou passando entre os grupos).
- Anotar a cada minuto o valor de **bat/50** no quadro: vira um gráfico de linha da turma.
- Pausar quando a tabela mudar bastante e perguntar "o que mudou nessa linha?".
- Mostrar os LEDs: verde = recompensa, vermelho = punição.

**3. Comparação (10 min).** Alternar para o modo **DEMONSTRAÇÃO** (sem aprender, sem explorar). Colocar um obstáculo novo. Ele generaliza? Por que às vezes erra? (Sensor ruidoso, só 4 estados.)

**4. Discussão final (10 min).**
- Onde existe esse tipo de aprendizado no mundo real? (Jogos, robôs, carros autônomos, recomendação.)
- O que mudaria se o robô tivesse uma câmera em vez de um sensor?
- Convite para a aula seguinte: o **Robô 2**, que aprende com **exemplos** dados por nós (aprendizado supervisionado). Perguntar: "Qual dos dois robôs precisou de professor?"

## Ligação com o currículo de Matemática

- Tabelas de dupla entrada e matrizes (a tabela Q é uma matriz 4×4).
- Média ponderada e taxa de variação (a regra de atualização).
- Probabilidade (exploração aleatória, epsilon).
- Gráfico de linha com dados coletados (batidas por minuto).

## Avaliação sugerida

Cada grupo entrega: (1) a tabela do papel antes e depois, (2) o gráfico de batidas da turma, (3) um parágrafo respondendo "O robô aprendeu ou foi programado? Justifique."

## Plano B

- Robô travou ou a bateria acabou: usar a memória pré-treinada gravada antes da aula e mostrar só a parte de demonstração, mais a atividade de papel.
- Aula curta: fazer só a atividade desplugada e uma demonstração de 10 minutos.

## Extensões (aulas seguintes)

- **Aula 3 · Quem ensina é a recompensa** (`atividades/ficha-01-recompensa.md`): laboratório de recompensas no simulador ou no painel Wi-Fi.
- **Aula 4 · Os botões do aprendizado** (`atividades/ficha-02-parametros.md`): α, γ e exploração, com média de rodadas.
- **Aula 5 · Pensando como o robô** (`atividades/ficha-03-pensando-como-robo.md`): modo *Adivinhe a ação* e a conta do Q-learning à mão.
- **Temporada completa (8º ano):** `temporada-1/` transforma este roteiro e as fichas em 6 aulas-episódio com tabelas, gráficos e médias.
- **Plano B turbinado:** treine o cérebro no simulador, baixe o JSON e carregue no robô pelo painel Wi-Fi.
