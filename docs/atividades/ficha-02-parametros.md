# Ficha 02 · Os três botões do aprendizado

**Ideia:** além das recompensas, o aprendizado tem "botões" que ajustam *como* o robô aprende.

| Botão | Nome | Em palavras simples |
|---|---|---|
| α (alfa) | taxa de aprendizado | O quanto ele aceita a novidade. Alto: muda de ideia rápido. Baixo: devagar. |
| γ (gama) | olhar para o futuro | Zero: só importa o que ganha agora. Perto de 1: pensa nas consequências. |
| ε (épsilon) | exploração | A chance de testar uma ação ao acaso. Começa em 100% e cai a cada passo. A *queda do ε* controla a velocidade dessa queda. |

## Experimentos
Em cada um mude **só um botão**, dê *Cérebro vazio*, treine e **repita 3 vezes**.

### A · α
Teste α = 0,05 · 0,3 (padrão) · 1,0. Treine 500 passos e compare o gráfico de batidas.
- Qual aprendeu mais rápido? Algum ficou "nervoso" (números mudando muito)?
- Observe a tabela Q a cada passo (velocidade 1×): com α = 1, o que acontece com a nota quando o robô recebe uma recompensa nova?

### B · γ
Teste γ = 0 · 0,9 (padrão) · 0,99.
- Com γ = 0, ele só olha a recompensa imediata. Isso atrapalha o desvio? Em que situação?
- No cenário **Labirinto**, repita com γ = 0,5 e γ = 0,99. O valor do final do labirinto chega longe no mapa de cores?

### C · exploração (queda do ε)
Teste a queda do ε = 0,98 · 0,995 (padrão) · 0,9995. Treine **só 500 passos**.
- Com 0,9995, o robô ainda está sorteando ações no fim? (veja a exploração no painel)
- Explorar demais atrapalha? Explorar de menos também? Por quê?

## Quadro de resultados
Registre: botão, valor, rodada 1/2/3 (batidas nos últimos 50 passos), média.
*Dica de Matemática:* calcule a média das 3 rodadas e compare. Por que a média é mais confiável que uma rodada só?

## Para discutir
1. Qual combinação deu o melhor resultado em 500 passos? E em 3000?
2. Existe um valor "certo" para todos os casos? Por quê?
3. Compare com estudar para uma prova: o que seria α, γ e ε?

## Pista para o professor
Os efeitos variam bastante de rodada para rodada. Use isso a favor: a turma precisa de **repetição e média** para concluir. Um efeito claro e fácil de ver é o da queda do ε: com 0,9995 o robô segue explorando por muito tempo e as batidas continuam altas em 500 passos.
