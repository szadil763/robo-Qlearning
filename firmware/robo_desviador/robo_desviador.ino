/*
  ROBÔ DESVIADOR DE OBSTÁCULOS COM Q-LEARNING
  Projeto didático: mostrar aos alunos como uma IA aprende por tentativa e erro
  (aprendizado por reforço).

  Placa: ESP32 DevKit (core Arduino-ESP32 3.x)
  Bibliotecas: Adafruit SSD1306, Adafruit GFX  (Preferences já vem com o core)

  IDEIA GERAL (ciclo que se repete sem parar):
    1. O robô MEDE a distância e descobre em que ESTADO está (perto? longe?)
    2. ESCOLHE uma AÇÃO (frente, esquerda, direita, ré)
         - às vezes por tentativa aleatória (explorar)
         - às vezes pela melhor nota da tabela Q (aproveitar o que já aprendeu)
    3. EXECUTA a ação e mede de novo
    4. Recebe uma RECOMPENSA (boa ou ruim)
    5. ATUALIZA a nota da tabela Q  -> é aqui que ele "aprende"

  Botões:
    BTN_MODO  (curto)  : alterna APRENDENDO <-> DEMONSTRAÇÃO (sem explorar, sem aprender)
    BTN_RESET (curto)  : liga/desliga o MODO PENSAR (o display mostra cada etapa do raciocínio)
    BTN_RESET (2 s)    : apaga tudo que o robô aprendeu (volta a ser "bebê")
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

// ======================= PINOS (ver docs/ligacoes.md) =======================
const int PIN_AIN1 = 26, PIN_AIN2 = 25, PIN_PWMA = 27;  // motor ESQUERDO
const int PIN_BIN1 = 33, PIN_BIN2 = 32, PIN_PWMB = 14;  // motor DIREITO
const int PIN_STBY = 13;
const int PIN_TRIG = 5, PIN_ECHO = 18;                  // HC-SR04 (ECHO com divisor de tensão!)
const int PIN_LED_VERDE = 19, PIN_LED_VERMELHO = 23;
const int PIN_BTN_MODO = 4, PIN_BTN_RESET = 16;         // botões para GND (INPUT_PULLUP)

// ======================= AJUSTES (mexa aqui para calibrar) ===================
const int   VELOCIDADE      = 200;   // 0 a 255 (limita a tensão nos motores N20)
const int   TEMPO_FRENTE_MS = 350;   // quanto tempo cada ação dura
const int   TEMPO_GIRO_MS   = 300;
const int   TEMPO_RE_MS     = 350;

const int   PENSAR_MS       = 900;   // MODO PENSAR: quanto tempo cada tela de raciocínio fica visível

const float DIST_BATEU      = 8.0;   // cm: menos que isso conta como "bateu"
const float LIMITES[3]      = {10.0, 25.0, 50.0};  // cm: separam os 4 estados

const float ALFA            = 0.3;   // taxa de aprendizado (quanto aceita a novidade)
const float GAMA            = 0.9;   // valor do futuro (quanto pensa à frente)
const float EPS_INICIAL     = 1.0;   // 100% tentativa aleatória no começo
const float EPS_MIN         = 0.05;  // nunca para de explorar totalmente
const float EPS_DECAI       = 0.995; // a cada passo, explora um pouco menos

// ======================= ESTADOS E AÇÕES =====================================
// Estados (distância à frente):
//   0 = MUITO PERTO (< 10 cm)   1 = PERTO (10-25)   2 = MÉDIO (25-50)   3 = LIVRE (> 50)
const int N_ESTADOS = 4;
// Ações:
const int ACAO_FRENTE = 0, ACAO_ESQUERDA = 1, ACAO_DIREITA = 2, ACAO_RE = 3;
const int N_ACOES = 4;

const char* NOME_ACAO[N_ACOES]     = {"Frente", "Esquerda", "Direita", "Re"};
const char* NOME_ESTADO[N_ESTADOS] = {"muito perto", "perto", "medio", "livre"};

enum Modo { APRENDENDO, DEMONSTRACAO };
Modo modo = APRENDENDO;
bool pensar = true;   // MODO PENSAR ligado: o display mostra o que o robô pensa em cada etapa

// ======================= A TABELA Q (o "cérebro") ============================
// Q[estado][ação] = nota que o robô dá para fazer aquela ação naquele estado.
// Começa tudo zero: ele não sabe NADA.
float Q[N_ESTADOS][N_ACOES];
float epsilon = EPS_INICIAL;
uint32_t passos = 0;

// Estatística para mostrar que ele melhora: batidas nos últimos 50 passos
int batidasJanela = 0, batidasUltimaJanela = 0, passosJanela = 0;

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Preferences prefs;

// ======================= MOTORES =============================================
void motor(int in1, int in2, int pwmPin, int vel) {  // vel: -255..255
  if (vel > 0)      { digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  }
  else if (vel < 0) { digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); }
  else              { digitalWrite(in1, LOW);  digitalWrite(in2, LOW);  }
  ledcWrite(pwmPin, abs(vel));
}

void parar() {
  motor(PIN_AIN1, PIN_AIN2, PIN_PWMA, 0);
  motor(PIN_BIN1, PIN_BIN2, PIN_PWMB, 0);
}

// Se o robô girar para o lado errado, troque os sinais do motor
// ou inverta os fios do motor no TB6612.
void executarAcao(int a) {
  switch (a) {
    case ACAO_FRENTE:
      motor(PIN_AIN1, PIN_AIN2, PIN_PWMA,  VELOCIDADE);
      motor(PIN_BIN1, PIN_BIN2, PIN_PWMB,  VELOCIDADE);
      delay(TEMPO_FRENTE_MS); break;
    case ACAO_ESQUERDA:
      motor(PIN_AIN1, PIN_AIN2, PIN_PWMA, -VELOCIDADE);
      motor(PIN_BIN1, PIN_BIN2, PIN_PWMB,  VELOCIDADE);
      delay(TEMPO_GIRO_MS); break;
    case ACAO_DIREITA:
      motor(PIN_AIN1, PIN_AIN2, PIN_PWMA,  VELOCIDADE);
      motor(PIN_BIN1, PIN_BIN2, PIN_PWMB, -VELOCIDADE);
      delay(TEMPO_GIRO_MS); break;
    case ACAO_RE:
      motor(PIN_AIN1, PIN_AIN2, PIN_PWMA, -VELOCIDADE);
      motor(PIN_BIN1, PIN_BIN2, PIN_PWMB, -VELOCIDADE);
      delay(TEMPO_RE_MS); break;
  }
  parar();
}

// ======================= SENSOR ULTRASSÔNICO =================================
float lerDistancia() {
  digitalWrite(PIN_TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long dur = pulseIn(PIN_ECHO, HIGH, 30000);   // timeout 30 ms (~5 m)
  if (dur == 0) return 400.0;                  // sem eco = nada à frente
  return dur * 0.0343 / 2.0;                   // som: 343 m/s
}

// Mediana de 3 leituras: ignora leituras malucas do sensor
float distanciaMediana() {
  float a = lerDistancia(); delay(10);
  float b = lerDistancia(); delay(10);
  float c = lerDistancia();
  return a + b + c - min(a, min(b, c)) - max(a, max(b, c));
}

int estadoDe(float d) {
  if (d < LIMITES[0]) return 0;
  if (d < LIMITES[1]) return 1;
  if (d < LIMITES[2]) return 2;
  return 3;
}

// ======================= RECOMPENSA (a "professora") =========================
// Aqui está a única coisa que NÓS ensinamos: o que é bom e o que é ruim.
// O resto, o robô descobre sozinho.
float calcularRecompensa(int acao, float d, float d2) {
  if (d2 < DIST_BATEU) return -10.0;                        // bateu: muito ruim
  if (acao == ACAO_FRENTE) return (d2 >= LIMITES[1]) ? 2.0 : 0.0;  // andar livre: bom
  if (acao == ACAO_RE) return -0.5;                         // ré: custa um pouco
  return (d2 > d + 10.0) ? 1.0 : -0.2;                      // girar: bom só se abriu caminho
}

// ======================= ESCOLHA DA AÇÃO (explorar x aproveitar) =============
int melhorAcao(int s) {
  float melhor = Q[s][0];
  for (int a = 1; a < N_ACOES; a++) if (Q[s][a] > melhor) melhor = Q[s][a];
  int empatadas[N_ACOES], n = 0;                 // desempate aleatório
  for (int a = 0; a < N_ACOES; a++) if (Q[s][a] == melhor) empatadas[n++] = a;
  return empatadas[random(n)];
}

// Devolve a ação escolhida. "explorou" e "sorteio" servem só para mostrar o raciocínio no display.
int escolherAcao(int s, bool &explorou, float &sorteio) {
  sorteio  = random(1000) / 1000.0;              // o "dado" do robô: número entre 0 e 1
  explorou = (modo == APRENDENDO && sorteio < epsilon);
  if (explorou) return random(N_ACOES);          // EXPLORAR: tenta algo ao acaso
  return melhorAcao(s);                          // APROVEITAR: usa o que aprendeu
}

// ======================= APRENDIZADO (equação de Q-learning) =================
// Q novo = Q velho + alfa * ( recompensa + gama * melhor Q do próximo estado - Q velho )
float maiorQ(int s) {
  float m = Q[s][0];
  for (int i = 1; i < N_ACOES; i++) if (Q[s][i] > m) m = Q[s][i];
  return m;
}

void atualizarQ(int s, int a, float r, int s2) {
  Q[s][a] += ALFA * (r + GAMA * maiorQ(s2) - Q[s][a]);
}

// ======================= MEMÓRIA (não esquece ao desligar) ===================
void salvar() {
  prefs.putBytes("q", Q, sizeof(Q));
  prefs.putFloat("eps", epsilon);
  prefs.putUInt("passos", passos);
}

void carregar() {
  if (prefs.getBytesLength("q") == sizeof(Q)) {
    prefs.getBytes("q", Q, sizeof(Q));
    epsilon = prefs.getFloat("eps", EPS_INICIAL);
    passos  = prefs.getUInt("passos", 0);
  }
}

void resetarAprendizado() {
  memset(Q, 0, sizeof(Q));
  epsilon = EPS_INICIAL; passos = 0;
  batidasJanela = batidasUltimaJanela = passosJanela = 0;
  salvar();
  display.clearDisplay(); display.setCursor(0, 20);
  display.println("Memoria apagada!"); display.println("Robo voltou a ser bebe.");
  display.display(); delay(1500);
}

// ======================= TELA OLED ===========================================
void desenhar(int s, float d) {
  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(modo == APRENDENDO ? "APRENDENDO" : "DEMO");
  display.print(" eps="); display.print(epsilon, 2);
  display.setCursor(0, 9);  display.print("   F   E   D   R");   // Frente Esq Dir Ré
  char buf[8];
  for (int i = 0; i < N_ESTADOS; i++) {
    display.setCursor(0, 18 + i * 9);
    display.print(i == s ? ">" : " ");
    for (int j = 0; j < N_ACOES; j++) {
      snprintf(buf, sizeof(buf), "%3d ", (int)roundf(Q[i][j]));
      display.print(buf);
    }
  }
  display.setCursor(0, 55);
  display.print("bat/50:"); display.print(batidasUltimaJanela);
  display.print(" d="); display.print((int)min(d, 999.0f)); display.print("cm");
  display.display();
}

// ======================= MODO PENSAR: o que o robô está pensando =============
// Uma tela para cada etapa do ciclo. Dá para ler no display, passo a passo,
// de onde vem cada decisão e como o número da tabela muda.
void cabecalho(int etapa, const char* nome) {
  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(etapa); display.print("/5 "); display.print(nome);
  display.setCursor(86, 0); display.print(modo == APRENDENDO ? "APREND" : "DEMO");
  display.drawFastHLine(0, 9, 128, SSD1306_WHITE);
}

// Linha da tabela Q do estado s (as 4 notas). A ação "destaque" aparece em vídeo invertido.
void linhaQ(int s, int destaque) {
  const char letras[N_ACOES] = {'F', 'E', 'D', 'R'};
  char buf[8];
  for (int j = 0; j < N_ACOES; j++) {
    int x = j * 32;
    display.setCursor(x + 12, 41); display.print(letras[j]);
    snprintf(buf, sizeof(buf), "%.1f", Q[s][j]);
    if (j == destaque) {
      display.fillRect(x, 50, 31, 10, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    }
    display.setCursor(x + 2, 52); display.print(buf);
    display.setTextColor(SSD1306_WHITE);
  }
}

void telaPerceber(int s, float d) {
  cabecalho(1, "PERCEBER");
  display.setCursor(0, 13); display.print("Sensor mede: "); display.print((int)min(d, 999.0f)); display.print(" cm");
  display.setCursor(0, 23); display.print("Estado "); display.print(s); display.print(": "); display.print(NOME_ESTADO[s]);
  display.setCursor(0, 32); display.print("Notas deste estado:");
  linhaQ(s, -1);
  display.display();
}

void telaEscolher(int s, int a, bool explorou, float sorteio) {
  cabecalho(2, "ESCOLHER");
  char buf[24];
  display.setCursor(0, 13);
  if (modo == DEMONSTRACAO) {
    display.print("DEMO: nao explora");
    display.setCursor(0, 23); display.print("Maior nota vence");
  } else {
    snprintf(buf, sizeof(buf), "dado %.2f  eps %.2f", sorteio, epsilon);
    display.print(buf);
    display.setCursor(0, 23);
    display.print(explorou ? "dado<eps: EXPLORAR!" : "dado>=eps: APROVEITA");
  }
  display.setCursor(0, 32); display.print("Escolhi: "); display.print(NOME_ACAO[a]);
  linhaQ(s, a);
  display.display();
}

void telaAgir(int a) {
  cabecalho(3, "AGIR");
  display.setCursor(0, 14); display.print("Vou executar:");
  display.setTextSize(2); display.setCursor(0, 30); display.print(NOME_ACAO[a]);
  display.setTextSize(1);
  display.display();
}

void telaRecompensa(float d, float d2, float r) {
  cabecalho(4, "RECOMPENSA");
  display.setCursor(0, 13); display.print("antes "); display.print((int)min(d, 999.0f));
  display.print(" cm  depois "); display.print((int)min(d2, 999.0f));
  display.setCursor(0, 25); display.print("Nota da professora:");
  char buf[12]; snprintf(buf, sizeof(buf), "%+.1f", r);
  display.setTextSize(2); display.setCursor(0, 38); display.print(buf);
  display.setTextSize(1); display.setCursor(70, 46);
  display.print(r > 0 ? "BOM" : (r < 0 ? "RUIM" : "neutro"));
  display.display();
}

void telaAprender(int s, int a, float q0, float q1, float r, float mq) {
  cabecalho(5, "APRENDER");
  char buf[24];
  display.setCursor(0, 13); display.print("Q["); display.print(s); display.print("]["); display.print(NOME_ACAO[a]); display.print("]");
  if (modo == DEMONSTRACAO) {
    display.setCursor(0, 25); display.print("DEMO: nao aprende");
    display.setCursor(0, 35); display.print("nota fica em "); display.print(q0, 2);
  } else {
    snprintf(buf, sizeof(buf), "velho %.2f", q0);       display.setCursor(0, 23); display.print(buf);
    snprintf(buf, sizeof(buf), "NOVO  %.2f", q1);       display.setCursor(0, 32); display.print(buf);
    snprintf(buf, sizeof(buf), "r=%.1f melhorQ'=%.1f", r, mq); display.setCursor(0, 43); display.print(buf);
    snprintf(buf, sizeof(buf), "Q+=%.1f*(r+%.1f*mQ'-Q)", ALFA, GAMA); display.setCursor(0, 53); display.print(buf);
  }
  display.display();
}

// ======================= BOTÕES ==============================================
void tratarBotoes() {
  static bool ultModo = HIGH;
  static bool contando = false;
  static unsigned long tReset = 0;

  bool b = digitalRead(PIN_BTN_MODO);
  if (ultModo == HIGH && b == LOW) {
    modo = (modo == APRENDENDO) ? DEMONSTRACAO : APRENDENDO;
    delay(50);
  }
  ultModo = b;

  // BTN_RESET: toque curto = liga/desliga MODO PENSAR; segurar 2 s = apagar a memória
  static bool longo = false;
  if (digitalRead(PIN_BTN_RESET) == LOW) {
    if (!contando) { contando = true; longo = false; tReset = millis(); }
    else if (!longo && millis() - tReset > 2000) {
      longo = true;
      resetarAprendizado();
    }
  } else {
    if (contando && !longo && millis() - tReset > 30) {
      pensar = !pensar;
      display.clearDisplay(); display.setCursor(0, 20);
      display.println(pensar ? "PENSAR: ligado" : "PENSAR: desligado");
      display.display(); delay(700);
    }
    contando = false;
  }
}

// ======================= SETUP E LOOP ========================================
void setup() {
  Serial.begin(115200);
  pinMode(PIN_AIN1, OUTPUT); pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_BIN1, OUTPUT); pinMode(PIN_BIN2, OUTPUT);
  pinMode(PIN_STBY, OUTPUT); digitalWrite(PIN_STBY, HIGH);
  ledcAttach(PIN_PWMA, 20000, 8);   // core 3.x: (pino, frequência, resolução em bits)
  ledcAttach(PIN_PWMB, 20000, 8);
  pinMode(PIN_TRIG, OUTPUT); pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_VERDE, OUTPUT); pinMode(PIN_LED_VERMELHO, OUTPUT);
  pinMode(PIN_BTN_MODO, INPUT_PULLUP); pinMode(PIN_BTN_RESET, INPUT_PULLUP);
  parar();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED nao encontrado (endereco 0x3C). O robo funciona sem ele.");
  }
  randomSeed(esp_random());

  prefs.begin("qlearn", false);
  memset(Q, 0, sizeof(Q));
  carregar();

  display.clearDisplay(); display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10); display.println("Robo Desviador Q-learning");
  display.println(passos == 0 ? "Cerebro vazio (bebe)" : "Cerebro carregado");
  display.println("Solte na arena em 3 s");
  display.display();
  delay(3000);
}

void loop() {
  tratarBotoes();

  // 1) Percebe o mundo
  float d  = distanciaMediana();
  int   s  = estadoDe(d);
  if (pensar) { telaPerceber(s, d); delay(PENSAR_MS); }

  // 2) Escolhe (explorar x aproveitar)
  bool  explorou; float sorteio;
  int   a = escolherAcao(s, explorou, sorteio);
  if (pensar) { telaEscolher(s, a, explorou, sorteio); delay(PENSAR_MS); }

  // 3) Executa
  if (pensar) telaAgir(a);
  executarAcao(a);
  delay(60);

  // 4) Vê o resultado e recebe a recompensa
  float d2 = distanciaMediana();
  int   s2 = estadoDe(d2);
  float r  = calcularRecompensa(a, d, d2);

  digitalWrite(PIN_LED_VERDE,    r > 0);
  digitalWrite(PIN_LED_VERMELHO, r < 0);
  if (pensar) { telaRecompensa(d, d2, r); delay(PENSAR_MS); }

  // 5) Aprende (só no modo APRENDENDO)
  float q0 = Q[s][a];
  float mq = maiorQ(s2);
  if (modo == APRENDENDO) {
    atualizarQ(s, a, r, s2);
    epsilon = max(EPS_MIN, epsilon * EPS_DECAI);
    passos++;
    if (passos % 20 == 0) salvar();
  }
  if (pensar) { telaAprender(s, a, q0, Q[s][a], r, mq); delay(PENSAR_MS); }

  // Estatística: batidas a cada 50 passos
  if (d2 < DIST_BATEU) batidasJanela++;
  if (++passosJanela >= 50) {
    batidasUltimaJanela = batidasJanela;
    batidasJanela = 0; passosJanela = 0;
  }

  if (!pensar) desenhar(s2, d2);   // modo rápido: mostra a tabela Q inteira
  Serial.printf("passo=%u estado=%d acao=%d recompensa=%.1f eps=%.2f\n", passos, s, a, r, epsilon);
}
