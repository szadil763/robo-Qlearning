/*
  PAINEL WI-FI do Robô Desviador (incluído por robo_desviador.ino)

  O ESP32 cria a rede definida em WIFI_NOME/WIFI_SENHA (não precisa de internet).
  Conecte o celular ou o computador e abra  http://192.168.4.1

  Endereços:
    /            página do painel (tabela Q, raciocínio ao vivo, ajustes, downloads)
    /estado      JSON com o que o robô está pensando agora
    /q.json      GET: baixa o cérebro. POST: carrega um cérebro (mesmo formato do simulador 3D)
    /log.csv     histórico dos últimos LOG_MAX passos (separador ";", decimal com vírgula)
    /ajustar     muda parâmetros: ?alfa=0.5&gama=0.9&decai=0.995&bateu=-10&livre=2&perto=0&abriu=1&giro=-0.2&re=-0.5
                 ?modo=1 (aprender) | 0 (demonstração)  ?pensar=1|0  ?padroes=1  ?apagar=1

  Este arquivo usa as variáveis globais de robo_desviador.ino (Q, alfa, gama, P, hist...).
*/
#pragma once

WebServer servidor(80);

// ---------- utilitários ----------
static String numJson(float v, int casas = 3) {   // ponto decimal (JSON)
  char b[24]; snprintf(b, sizeof(b), "%.*f", casas, v); return String(b);
}
static String numCsv(float v, int casas = 3) {    // vírgula decimal (Excel/Planilhas em português)
  String t = numJson(v, casas); t.replace('.', ','); return t;
}

static String cerebroJson() {
  String j = "{\"formato\":\"robo-qlearning-q\",\"versao\":1,\"estados\":4,\"acoes\":4,\"q\":[";
  for (int i = 0; i < N_ESTADOS; i++) {
    j += i ? ",[" : "[";
    for (int k = 0; k < N_ACOES; k++) { if (k) j += ","; j += numJson(Q[i][k], 4); }
    j += "]";
  }
  j += "],\"epsilon\":" + numJson(epsilon, 4) + ",\"passos\":" + String(passos) +
       ",\"alfa\":" + numJson(alfa, 3) + ",\"gama\":" + numJson(gama, 3) + "}";
  return j;
}

// Lê um cérebro em JSON. Aceita o arquivo baixado do simulador 3D ou de /q.json.
// (leitura simples, sem biblioteca: procura "q" e lê 16 números em sequência)
static bool lerCerebro(const String &js) {
  if (js.indexOf("\"estados\":4") < 0 || js.indexOf("\"acoes\":4") < 0) return false;
  int i = js.indexOf("\"q\"");
  if (i < 0) return false;
  i = js.indexOf('[', i);
  if (i < 0) return false;
  float tmp[N_ESTADOS * N_ACOES];
  const char *p = js.c_str() + i + 1;
  int n = 0;
  while (n < N_ESTADOS * N_ACOES) {
    while (*p && *p != '-' && *p != '.' && !isdigit((unsigned char)*p)) {
      if (*p == '}' ) return false;
      p++;
    }
    if (!*p) return false;
    char *fim; float v = strtof(p, &fim);
    if (fim == p || v != v || fabsf(v) > 1e30f) return false;   // rejeita "nan" e valores absurdos
    tmp[n++] = v; p = fim;
  }
  memcpy(Q, tmp, sizeof(Q));
  int e = js.indexOf("\"epsilon\":");
  if (e >= 0) epsilon = constrain(strtof(js.c_str() + e + 10, nullptr), 0.0f, 1.0f);
  int ps = js.indexOf("\"passos\":");
  if (ps >= 0) passos = (uint32_t)strtoul(js.c_str() + ps + 9, nullptr, 10);
  salvar();
  return true;
}

// ---------- rotas ----------
static void rotaEstado() {
  String j = "{";
  j += "\"etapa\":" + String(P.etapa) + ",\"s\":" + String(P.s) + ",\"s2\":" + String(P.s2) + ",\"a\":" + String(P.a);
  j += ",\"d\":" + numJson(P.d, 1) + ",\"d2\":" + numJson(P.d2, 1) + ",\"sorteio\":" + numJson(P.sorteio, 2);
  j += ",\"explorou\":" + String(P.explorou ? "true" : "false");
  j += ",\"r\":" + numJson(P.r, 1) + ",\"q0\":" + numJson(P.q0, 2) + ",\"q1\":" + numJson(P.q1, 2) + ",\"mq\":" + numJson(P.mq, 2);
  j += ",\"passos\":" + String(passos) + ",\"eps\":" + numJson(epsilon, 3);
  j += ",\"aprendendo\":" + String(modo == APRENDENDO ? "true" : "false") + ",\"pensar\":" + String(pensar ? "true" : "false");
  j += ",\"bat50\":" + String(batidasUltimaJanela);
  j += ",\"par\":{\"alfa\":" + numJson(alfa, 3) + ",\"gama\":" + numJson(gama, 3) + ",\"decai\":" + numJson(epsDecai, 4) +
       ",\"bateu\":" + numJson(rBateu, 1) + ",\"livre\":" + numJson(rLivre, 1) + ",\"perto\":" + numJson(rPerto, 1) +
       ",\"abriu\":" + numJson(rAbriu, 1) + ",\"giro\":" + numJson(rGiro, 1) + ",\"re\":" + numJson(rRe, 1) + "}";
  j += ",\"q\":[";
  for (int i = 0; i < N_ESTADOS; i++) {
    j += i ? ",[" : "[";
    for (int k = 0; k < N_ACOES; k++) { if (k) j += ","; j += numJson(Q[i][k], 2); }
    j += "]";
  }
  j += "]}";
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send(200, "application/json", j);
}

static void rotaCerebro() {
  if (servidor.method() == HTTP_POST) {
    bool ok = lerCerebro(servidor.arg("plain"));
    servidor.send(ok ? 200 : 400, "text/plain; charset=utf-8", ok ? "Cérebro carregado." : "Arquivo inválido (precisa ser um cérebro 4 estados x 4 ações).");
    return;
  }
  servidor.sendHeader("Content-Disposition", "attachment; filename=cerebro-robo.json");
  servidor.send(200, "application/json", cerebroJson());
}

static void rotaCsv() {
  servidor.sendHeader("Content-Disposition", "attachment; filename=historico-robo.csv");
  servidor.setContentLength(CONTENT_LENGTH_UNKNOWN);
  servidor.send(200, "text/csv; charset=utf-8", "");
  servidor.sendContent("passo;estado;acao;explorou;recompensa;q_antes;q_depois;epsilon\n");
  static const char *nomes[N_ACOES] = {"frente", "esquerda", "direita", "re"};
  int ini = (histN < LOG_MAX) ? 0 : histI;
  for (int k = 0; k < histN; k++) {
    const Registro &g = hist[(ini + k) % LOG_MAX];
    String l = String(g.passo) + ";" + String(g.s) + ";" + nomes[g.a] + ";" + String(g.explorou) + ";" +
               numCsv(g.r, 2) + ";" + numCsv(g.q0, 3) + ";" + numCsv(g.q1, 3) + ";" + numCsv(g.eps, 3) + "\n";
    servidor.sendContent(l);
  }
  servidor.sendContent("");
}

static void ajustar(const char *nome, float &alvo, float mn, float mx) {
  if (servidor.hasArg(nome)) alvo = constrain(servidor.arg(nome).toFloat(), mn, mx);
}

static void rotaAjustar() {
  if (servidor.hasArg("padroes")) restaurarPadroes();
  ajustar("alfa", alfa, 0.05f, 1.0f);
  ajustar("gama", gama, 0.0f, 0.99f);
  ajustar("decai", epsDecai, 0.98f, 0.9995f);
  ajustar("bateu", rBateu, -30.0f, 0.0f);
  ajustar("livre", rLivre, 0.0f, 5.0f);
  ajustar("perto", rPerto, -3.0f, 3.0f);
  ajustar("abriu", rAbriu, 0.0f, 3.0f);
  ajustar("giro", rGiro, -2.0f, 0.0f);
  ajustar("re", rRe, -2.0f, 0.0f);
  if (servidor.hasArg("modo")) modo = servidor.arg("modo").toInt() ? APRENDENDO : DEMONSTRACAO;
  if (servidor.hasArg("pensar")) pensar = servidor.arg("pensar").toInt() != 0;
  if (servidor.hasArg("apagar")) resetarAprendizado();
  servidor.send(200, "text/plain", "ok");
}

// ---------- a página ----------
static const char PAGINA[] PROGMEM = R"rawliteral(<!doctype html><html lang="pt-BR"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>Painel do Robô</title>
<style>
:root{--bg:#EAEEF1;--card:#F7F9FA;--fg:#14202A;--mut:#566673;--line:#C9D3DA;--ac:#E0530B;--ok:#16885A;--bad:#CC3838}
@media(prefers-color-scheme:dark){:root{--bg:#0E151B;--card:#16212A;--fg:#E6EDF2;--mut:#93A4B1;--line:#2A3946;--ac:#FF7A33;--ok:#3DD598;--bad:#FF6B6B}}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--fg);font:15px/1.5 system-ui,sans-serif;padding:16px;max-width:820px;margin:auto}
h1{font-size:22px;margin:0 0 4px}h2{font:600 12px monospace;letter-spacing:.08em;text-transform:uppercase;color:var(--mut);margin:0 0 10px}
.c{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:14px;margin:12px 0}
.m{font:15px/1.5 monospace}.big{font-size:22px}.pos{color:var(--ok)}.neg{color:var(--bad)}
table{width:100%;border-collapse:collapse;font:14px monospace}td,th{border:1px solid var(--line);padding:8px 4px;text-align:center}th{color:var(--mut);font-weight:500}
tr.cur td,tr.cur th{background:color-mix(in srgb,var(--ac) 18%,transparent)}td.sel{outline:2px solid var(--ac);outline-offset:-2px}
.st{display:flex;gap:6px;flex-wrap:wrap}.st span{padding:6px 10px;border:1px solid var(--line);border-radius:8px;font:12px monospace;color:var(--mut)}.st span.on{background:var(--ac);color:#fff;border-color:var(--ac)}
.r{display:grid;grid-template-columns:1fr auto;gap:2px 10px;margin-bottom:8px}.r input{grid-column:1/-1;width:100%;accent-color:var(--ac)}
button,a.b{font:600 14px system-ui;padding:10px 12px;border-radius:8px;border:1px solid var(--line);background:var(--card);color:var(--fg);cursor:pointer;text-decoration:none;display:inline-block;margin:4px 4px 0 0}
small{color:var(--mut)}
</style></head><body>
<h1>Painel do Robô Desviador</h1><small id="sub">conectando…</small>
<div class="c"><h2>Mente do robô</h2><div class="st" id="et"></div><p class="m big" id="th" style="margin:12px 0 4px">…</p><p class="m" id="dt" style="margin:0;color:var(--mut)"></p></div>
<div class="c"><h2>Tabela Q (o cérebro)</h2><table id="tq"><thead><tr><th>Estado</th><th>Frente</th><th>Esq.</th><th>Dir.</th><th>Ré</th></tr></thead><tbody></tbody></table>
<p><small>Linha destacada = situação atual. Contorno = ação escolhida.</small></p></div>
<div class="c"><h2>Laboratório: mude as regras</h2><div id="lab"></div>
<button onclick="env('padroes=1')">Restaurar padrões</button><button onclick="if(confirm('Apagar tudo que o robô aprendeu?'))env('apagar=1')">Cérebro vazio</button>
<button id="bm" onclick="env('modo='+(S.aprendendo?0:1))"></button><button id="bp" onclick="env('pensar='+(S.pensar?0:1))"></button></div>
<div class="c"><h2>Cérebro e dados</h2><a class="b" href="/q.json" download>Baixar cérebro (JSON)</a><a class="b" href="/log.csv" download>Baixar histórico (CSV)</a>
<button onclick="document.getElementById('f').click()">Carregar cérebro</button><input type="file" id="f" accept=".json" hidden>
<p><small id="msg">O cérebro (JSON) tem o mesmo formato do simulador 3D: treine lá e carregue aqui. O CSV abre no Excel ou Planilhas.</small></p></div>
<script>
var S={},N=['Frente','Esquerda','Direita','Ré'],E=['< 10 cm','10 a 25 cm','25 a 50 cm','> 50 cm'],ET=['Perceber','Escolher','Agir','Recompensa','Aprender'],
L=[['alfa','α · taxa de aprendizado',.05,1,.05],['gama','γ · olhar para o futuro',0,.99,.01],['decai','Queda do ε por passo',.98,.9995,.0005],
['bateu','Recompensa: bater',-30,0,1],['livre','Recompensa: frente livre',0,5,.5],['perto','Recompensa: frente com obstáculo perto',-3,3,.5],
['abriu','Recompensa: girar e abrir caminho',0,3,.5],['giro','Recompensa: girar sem ganho',-2,0,.1],['re','Recompensa: ré',-2,0,.1]];
var lab=document.getElementById('lab'),drag=false;
lab.innerHTML=L.map(function(x,i){return '<div class="r"><label>'+x[1]+'</label><b id="o'+i+'"></b><input type="range" id="s'+i+'" min="'+x[2]+'" max="'+x[3]+'" step="'+x[4]+'"></div>';}).join('');
L.forEach(function(x,i){var e=document.getElementById('s'+i);
 e.onpointerdown=function(){drag=true};e.oninput=function(){document.getElementById('o'+i).textContent=e.value;};
 e.onchange=function(){drag=false;env(x[0]+'='+e.value);};});
document.getElementById('et').innerHTML=ET.map(function(n,i){return '<span>'+(i+1)+' '+n+'</span>';}).join('');
var tb=document.querySelector('#tq tbody');tb.innerHTML=E.map(function(e,i){return '<tr><th>'+i+' · '+e+'</th><td></td><td></td><td></td><td></td></tr>';}).join('');
function env(q){return fetch('/ajustar?'+q).then(function(){return poll();});}
function f(v,d){return (v<0?'−':'')+Math.abs(v).toFixed(d);}
function poll(){return fetch('/estado',{cache:'no-store'}).then(function(r){return r.json();}).then(function(s){S=s;
 document.getElementById('sub').textContent='passo '+s.passos+' · exploração '+Math.round(s.eps*100)+'% · batidas nos últimos 50: '+s.bat50+(s.aprendendo?'':' · DEMONSTRAÇÃO');
 [].forEach.call(document.querySelectorAll('#et span'),function(e,i){e.className=(i+1===s.etapa)?'on':'';});
 var a=N[s.a],t='',d='';
 if(s.etapa===1){t='Estou a '+(s.d>=400?'mais de 4 m':Math.round(s.d)+' cm')+' de algo. Situação '+s.s+'.';}
 else if(s.etapa===2){t=s.aprendendo?(s.explorou?'O dado ('+s.sorteio.toFixed(2)+') caiu abaixo de ε: vou testar '+a+' ao acaso!':'O dado ('+s.sorteio.toFixed(2)+') passou de ε: vou usar a maior nota: '+a+'.'):'Demonstração: maior nota vence → '+a+'.';}
 else if(s.etapa===3){t='Mãos à obra: '+a+'!';}
 else if(s.etapa===4){t='Recompensa: '+(s.r>0?'+':'')+f(s.r,1)+(s.r>0?' — boa!':s.r<0?' — ruim.':'');d='distância '+Math.round(s.d)+' → '+Math.round(s.d2)+' cm';}
 else if(s.etapa===5){t=s.aprendendo?('Nota de '+a+' na situação '+s.s+': '+f(s.q0,2)+' → '+f(s.q1,2)):'Demonstração: a tabela não muda.';
  d=s.aprendendo?('Q novo = '+f(s.q0,2)+' + '+s.par.alfa+' × ( '+f(s.r,1)+' + '+s.par.gama+' × '+f(s.mq,2)+' − '+f(s.q0,2)+' )'):'';}
 document.getElementById('th').textContent=t;document.getElementById('dt').textContent=d;
 [].forEach.call(tb.rows,function(r,i){r.className=(i===s.s)?'cur':'';[].forEach.call(r.cells,function(c,j){if(j>0){c.textContent=f(s.q[i][j-1],1);c.className=(i===s.s&&j-1===s.a&&s.etapa>=2)?'sel':'';}});});
 if(!drag)L.forEach(function(x,i){var e=document.getElementById('s'+i);if(document.activeElement!==e){e.value=s.par[x[0]];document.getElementById('o'+i).textContent=e.value;}});
 document.getElementById('bm').textContent=s.aprendendo?'Modo: aprendendo (tocar para demonstração)':'Modo: demonstração (tocar para aprender)';
 document.getElementById('bp').textContent=s.pensar?'Modo pensar: ligado (tocar para desligar)':'Modo pensar: desligado (tocar para ligar)';
 }).catch(function(){document.getElementById('sub').textContent='sem conexão com o robô…';});}
document.getElementById('f').onchange=function(e){var a=e.target.files[0];if(!a)return;var r=new FileReader();r.onload=function(){
 fetch('/q.json',{method:'POST',body:r.result}).then(function(x){return x.text();}).then(function(t){document.getElementById('msg').textContent=t;poll();});};r.readAsText(a);e.target.value='';};
poll();setInterval(poll,700);
</script></body></html>)rawliteral";

static void iniciarPainel() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_NOME, WIFI_SENHA);
  servidor.on("/", []() { servidor.send_P(200, "text/html; charset=utf-8", PAGINA); });
  servidor.on("/estado", rotaEstado);
  servidor.on("/q.json", rotaCerebro);
  servidor.on("/log.csv", rotaCsv);
  servidor.on("/ajustar", rotaAjustar);
  servidor.begin();
  Serial.printf("Painel Wi-Fi: rede %s, senha %s, abra http://%s\n", WIFI_NOME, WIFI_SENHA, WiFi.softAPIP().toString().c_str());
}
