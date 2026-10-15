#!/usr/bin/env python3
"""
Gerador paramétrico das peças impressas do Robô Desviador (Q-learning).

Uso:   python3 gerar_stl.py            (cria a pasta cad/stl/ e cad/montagem.json)
Requer: pip install trimesh manifold3d numpy shapely mapbox-earcut

Eixos (pose de MONTAGEM): X = frente do robô, Y = esquerda, Z = para cima. Unidades: mm.
O plano de baixo do chassi está em Z = 0. Os STL são exportados já deitados para IMPRESSÃO.
O arquivo montagem.json guarda as mesmas malhas na pose de montagem (usado pela ilustração 3D).

ATENÇÃO: as medidas dos componentes comprados (motor N20, HC-SR04, OLED, suporte 18650) variam
entre fornecedores. Imprima primeiro a peça de TESTE (08_teste_ajuste.stl), confira o encaixe e
ajuste as folgas abaixo antes de imprimir o resto.
"""
import base64, json, math, os, sys
import numpy as np
import trimesh
from trimesh import transformations as T
from shapely.geometry import box as sbox, Point

# ======================= PARÂMETROS (mexa aqui) =======================
# --- chassi ---
BASE_C, BASE_L, BASE_E, RAIO = 110.0, 85.0, 4.0, 8.0     # comprimento (X), largura (Y), espessura, canto
# --- motor N20 com caixa de redução (corpo 12 x 10 mm, comprimento total ~24 mm) ---
FOLGA_MOTOR = 0.4
N20_W, N20_H, N20_C = 12.0 + FOLGA_MOTOR, 10.0 + FOLGA_MOTOR, 24.0
PAREDE = 2.4
ALTURA_BERCO = 8.0            # altura das paredes do berço acima da placa
MOTOR_X = -17.0               # posição do eixo das rodas (X)
EIXO_D, EIXO_FLAT = 3.0, 1.0  # eixo em D do N20: diâmetro 3 mm, face plana a 1 mm do centro
# --- roda e pneu ---
RODA_D = 42.0                 # diâmetro externo (com pneu)
RODA_L = 8.0                  # largura
ASSENTO_D = 36.0              # diâmetro do assento do pneu
FLANGE_D, FLANGE_E = 40.0, 1.2
FOLGA_EIXO = 0.12             # folga do furo em D (aumente se ficar apertado demais)
FOLGA_RODA_PLACA = 1.5        # espaço entre a placa e a roda
# --- sensor HC-SR04 (placa 45 x 20 x 1,6 mm; olhos de 16 mm a 26 mm de distância) ---
FOLGA_SENSOR = 0.4
OLHO_D, OLHO_DIST = 16.0 + FOLGA_SENSOR, 26.0
PCB_W, PCB_H, PCB_E = 45.0 + 0.6, 20.0 + 0.6, 1.6 + 0.4
SUP_L, SUP_H = 56.0, 26.0     # largura e altura da placa do suporte do sensor
SUP_PAREDE = 3.0              # parede frontal (onde ficam os furos)
FLANGE_SENS = 9.0             # profundidade da aba que apoia no chassi
# --- patim (apoio frontal) ---
PATIM_X, PATIM_D, PINO_D = 33.0, 10.0, 6.0
FOLGA_PINO = 0.3
# --- suporte do display OLED 0,96" (módulo 27 x 27,8 mm, furos 23,3 x 23,8 mm) ---
OLED_INCL = 55.0              # inclinação da tela em relação à horizontal (graus)
OLED_COMP = 36.0              # comprimento da rampa
OLED_LARG = 60.0
OLED_X_TRAS = -54.0           # borda de trás da base
SEG = 48                      # lados dos cilindros grandes

H_BASE_MIN = 0.0
VAO = RODA_D / 2 - (BASE_E + N20_H / 2)       # altura do fundo da placa até o chão

# ======================= utilitários de geometria =======================
def box(x0, x1, y0, y1, z0, z1):
    return trimesh.creation.box(extents=[x1 - x0, y1 - y0, z1 - z0],
                                transform=T.translation_matrix([(x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2]))

def cil(r, h, p0, eixo="z", sec=SEG):
    """cilindro de raio r e altura h, com a base em p0 e eixo 'x','y' ou 'z'."""
    m = trimesh.creation.cylinder(radius=r, height=h, sections=sec)
    m.apply_translation([0, 0, h / 2])
    if eixo == "x": m.apply_transform(T.rotation_matrix(math.pi / 2, [0, 1, 0]))
    if eixo == "y": m.apply_transform(T.rotation_matrix(-math.pi / 2, [1, 0, 0]))
    m.apply_translation(p0)
    return m

def uniao(lista):
    lista = [m for m in lista if m is not None]
    return trimesh.boolean.union(lista, engine="manifold") if len(lista) > 1 else lista[0]

def sub(a, lista):
    return trimesh.boolean.difference([a] + [m for m in lista if m is not None], engine="manifold")

def extrudar(poly, h):
    return trimesh.creation.extrude_polygon(poly, h)

def prisma_xz(pontos, y0, y1):
    """polígono no plano XZ extrudado de y0 a y1."""
    m = trimesh.creation.extrude_polygon(__import__("shapely.geometry", fromlist=["Polygon"]).Polygon(pontos), y1 - y0)
    # extrude_polygon cria em XY com Z = altura; troca para XZ com Y = altura
    m.apply_transform(T.rotation_matrix(math.pi / 2, [1, 0, 0]))   # (x,y,z)->(x,-z,y)
    m.apply_translation([0, y1, 0])
    return m

# ======================= PEÇAS =======================
def chassi():
    placa = extrudar(sbox(-BASE_C / 2 + RAIO, -BASE_L / 2 + RAIO, BASE_C / 2 - RAIO, BASE_L / 2 - RAIO).buffer(RAIO, resolution=16), BASE_E)
    extras, furos = [], []
    for s in (1, -1):
        y_ext = s * BASE_L / 2                     # borda externa da placa
        y_int = y_ext - s * N20_C                  # fim interno do berço
        ya, yb = sorted((y_int, y_ext))
        # paredes laterais do berço
        for sx in (-1, 1):
            x0 = MOTOR_X + sx * N20_W / 2
            xa, xb = sorted((x0, x0 + sx * PAREDE))
            extras.append(box(xa, xb, ya, yb, BASE_E, BASE_E + ALTURA_BERCO))
        # parede de fundo (lado interno) com passagem para os fios do motor
        yi0, yi1 = sorted((y_int, y_int + s * PAREDE))
        fundo = box(MOTOR_X - N20_W / 2 - PAREDE, MOTOR_X + N20_W / 2 + PAREDE, yi0, yi1, BASE_E, BASE_E + ALTURA_BERCO)
        extras.append(sub(fundo, [box(MOTOR_X - 3.5, MOTOR_X + 3.5, yi0 - 1, yi1 + 1, BASE_E - 0.1, BASE_E + ALTURA_BERCO + 1)]))
        # fendas para abraçadeiras de nylon (uma de cada lado do motor)
        for sx in (-1, 1):
            cx = MOTOR_X + sx * (N20_W / 2 + PAREDE + 2.2)
            furos.append(box(cx - 1.5, cx + 1.5, s * (BASE_L / 2 - N20_C / 2) - 4, s * (BASE_L / 2 - N20_C / 2) + 4, -1, BASE_E + 1))
        # passagem de fios do motor
        furos.append(cil(3.0, BASE_E + 2, [MOTOR_X, s * 12.0, -1], sec=32))
    furos.append(cil(3.0, BASE_E + 2, [MOTOR_X, 0, -1], sec=32))
    furos.append(cil(4.5, BASE_E + 2, [-38.0, 0, -1], sec=32))                     # fios da chave/baterias
    furos.append(cil((PINO_D + FOLGA_PINO) / 2, BASE_E + 2, [PATIM_X, 0, -1], sec=32))  # pino do patim
    for sy in (-1, 1):                                                           # parafusos do suporte do sensor
        furos.append(cil(1.35, BASE_E + 2, [BASE_C / 2 - SUP_PAREDE - 2 - FLANGE_SENS / 2 + 1.0, sy * 20.0, -1], sec=24))
    for x in (OLED_X_TRAS + 4, OLED_X_TRAS + 22):                               # parafusos do suporte do OLED
        for sy in (-1, 1):
            furos.append(cil(1.35, BASE_E + 2, [x, sy * 24.0, -1], sec=24))
    return sub(uniao([placa] + extras), furos)

def patim():
    """Apoio liso: pino que entra no chassi + haste + ponta esférica. Origem em Z=0 (fundo da placa)."""
    pino = cil(PINO_D / 2, BASE_E, [PATIM_X, 0, 0], sec=32)                       # atravessa a placa (de 0 a BASE_E)
    haste = cil(PATIM_D / 2, VAO - PATIM_D / 2, [PATIM_X, 0, -(VAO - PATIM_D / 2)])
    esf = trimesh.creation.icosphere(subdivisions=3, radius=PATIM_D / 2)
    esf.apply_translation([PATIM_X, 0, -(VAO - PATIM_D / 2)])
    return uniao([pino, haste, esf])

def suporte_sensor():
    x_f = BASE_C / 2                 # face frontal do suporte (borda do chassi)
    x_p = x_f - SUP_PAREDE - 2.0     # início da região do rebaixo (2 mm de profundidade)
    z0 = BASE_E
    placa = box(x_p, x_f, -SUP_L / 2, SUP_L / 2, z0, z0 + SUP_H)
    zc = z0 + SUP_H / 2
    cortes = [box(x_p - 1, x_p + 2.0, -PCB_W / 2, PCB_W / 2, zc - PCB_H / 2, zc + PCB_H / 2)]          # bolso da placa HC-SR04
    for sy in (-1, 1):
        cortes.append(cil(OLHO_D / 2, SUP_PAREDE + 3, [x_p - 0.5, sy * OLHO_DIST / 2, zc], eixo="x", sec=SEG))  # olhos
    aba = box(x_p - FLANGE_SENS, x_p, -SUP_L / 2, SUP_L / 2, z0, z0 + BASE_E)
    cortes.append(box(x_p - FLANGE_SENS - 1, x_p + 2.1, -8, 8, z0 - 1, zc - PCB_H / 2 + 3))          # passagem dos pinos
    for sy in (-1, 1):
        cortes.append(cil(1.6, BASE_E + 2, [x_p - FLANGE_SENS / 2 + 1.0, sy * 20.0, z0 - 1], sec=24))   # furos M3
    reforcos = []
    for sy in (-1, 1):
        yc = sy * (SUP_L / 2 - 1.5)
        reforcos.append(prisma_xz([(x_p - FLANGE_SENS, z0), (x_p, z0), (x_p, z0 + 16), (x_p - FLANGE_SENS, z0 + BASE_E)], yc - 1.5, yc + 1.5))
    return sub(uniao([placa, aba] + reforcos), cortes)

def roda():
    """Roda com eixo no Z local, de -L/2 a +L/2. Face 'de dentro' em -Z."""
    a = RODA_L / 2
    corpo = uniao([
        cil(ASSENTO_D / 2, RODA_L, [0, 0, -a]),
        cil(FLANGE_D / 2, FLANGE_E, [0, 0, -a]),
        cil(FLANGE_D / 2, FLANGE_E, [0, 0, a - FLANGE_E]),
        cil(5.0, RODA_L, [0, 0, -a], sec=36),               # cubo
    ])
    # alívio: miolo vazado entre cubo e aro (deixa 5 raios e uma alma)
    cortes = []
    r_miolo_i, r_miolo_e = 6.5, ASSENTO_D / 2 - 2.4
    anel = sub(cil(r_miolo_e, RODA_L, [0, 0, -a + 1.6]), [cil(r_miolo_i, RODA_L + 2, [0, 0, -a - 1])])  # aberto na face +Z (sem vazio fechado)
    cortes.append(anel)
    corpo = sub(corpo, cortes)
    raios = []
    for k in range(5):
        ang = k * 2 * math.pi / 5 + math.pi / 10
        b = box(r_miolo_i - 0.6, r_miolo_e + 0.6, -1.5, 1.5, -a + 1.0, a - 0.4)
        b.apply_transform(T.rotation_matrix(ang, [0, 0, 1]))
        raios.append(b)
    corpo = uniao([corpo] + raios)
    # furo em D para o eixo do N20
    r = EIXO_D / 2 + FOLGA_EIXO
    d = Point(0, 0).buffer(r, resolution=24).intersection(sbox(-r - 1, -r - 1, EIXO_FLAT + FOLGA_EIXO, r + 1))
    furo = extrudar(d, RODA_L + 2)
    furo.apply_translation([0, 0, -a - 1])
    return sub(corpo, [furo])

def pneu():
    """Pneu em TPU (opcional): anel que encaixa entre as flanges da roda. Eixo no Z local."""
    larg = RODA_L - 2 * FLANGE_E - 0.4
    ext = cil(RODA_D / 2, larg, [0, 0, -larg / 2])
    return sub(ext, [cil(ASSENTO_D / 2 - 0.5, larg + 2, [0, 0, -larg / 2 - 1])])

def suporte_oled():
    th = math.radians(OLED_INCL)
    z_base = BASE_E + 3.0
    x0 = OLED_X_TRAS
    L = OLED_COMP
    xf = x0 + L * math.cos(th)
    zt = z_base + L * math.sin(th)
    base = box(x0, xf + 2, -OLED_LARG / 2, OLED_LARG / 2, BASE_E, z_base)
    cortes_base = []
    for x in (x0 + 4, x0 + 22):
        for sy in (-1, 1):
            cortes_base.append(cil(1.7, 6, [x, sy * 24.0, BASE_E - 1], sec=24))
    # laterais triangulares
    tri = []
    for sy in (-1, 1):
        y0 = sy * OLED_LARG / 2
        ya, yb = sorted((y0, y0 - sy * 3.0))
        tri.append(prisma_xz([(x0, z_base), (xf, z_base), (xf, zt)], ya, yb))
    # placa inclinada (face em z=0 local, espessura 3 abaixo)
    rampa = box(0, L, -OLED_LARG / 2, OLED_LARG / 2, -3.0, 0.0)
    pegas = []
    sc, sl = 18.5, 0.0    # centro do módulo: s ao longo da rampa e y
    for dy in (-11.65, 11.65):
        for ds in (-11.9, 11.9):
            pegas.append(cil(0.95, 3.5, [sc + ds, sl + dy, -0.2], sec=16))
    rampa_cortes = []
    for sy in (-1, 1):                                                # furos de LED 5 mm
        rampa_cortes.append(cil(2.6, 5, [sc, sy * 22.5, -4], sec=24))
    rampa_cortes.append(box(sc - 11.9 - 4.2, sc - 11.9 - 0.2, -6.5, 6.5, -4, 1))   # passagem dos pinos do display
    rampa = sub(rampa, rampa_cortes)
    rampa = uniao([rampa] + pegas)
    rampa.apply_transform(T.rotation_matrix(-th, [0, 1, 0]))
    rampa.apply_translation([x0, 0, z_base])
    pecas = uniao([sub(base, cortes_base), rampa] + tri)
    return pecas

def teste_ajuste():
    """Peça pequena para testar as folgas ANTES de imprimir o robô inteiro (~15 min)."""
    z = BASE_E
    base = box(0, 70, 0, 40, 0, z)
    # trecho de berço de motor
    ber = [box(4, 4 + PAREDE, 4, 20, z, z + ALTURA_BERCO), box(4 + PAREDE + N20_W, 4 + 2 * PAREDE + N20_W, 4, 20, z, z + ALTURA_BERCO)]
    # placa com dois olhos do sensor
    pl = box(32, 32 + SUP_PAREDE, 4, 4 + 36, z, z + 22)
    olhos = [cil(OLHO_D / 2, SUP_PAREDE + 2, [31, 4 + 18 + s * OLHO_DIST / 2, z + 11], eixo="x") for s in (-1, 1)]
    # furo do pino do patim e furo de parafuso
    furos = [cil((PINO_D + FOLGA_PINO) / 2, z + 2, [52, 12, -1], sec=32), cil(1.35, z + 2, [52, 28, -1], sec=24)]
    # disco com furo em D (teste do eixo)
    disco = cil(7.0, 5.0, [20, 32, z])
    r = EIXO_D / 2 + FOLGA_EIXO
    d = Point(0, 0).buffer(r, resolution=24).intersection(sbox(-r - 1, -r - 1, EIXO_FLAT + FOLGA_EIXO, r + 1))
    fd = extrudar(d, 8); fd.apply_translation([20, 32, z - 1])
    return sub(uniao([base] + ber + [pl, disco]), olhos + furos + [fd])

# ======================= LISTA DE PEÇAS (pose de montagem + impressão) =======================
def para_impressao(malha, nome):
    m = malha.copy()
    if nome == "suporte_sensor":     # deita sobre a face frontal (rebaixo para cima)
        m.apply_transform(T.rotation_matrix(-math.pi / 2, [0, 1, 0]))       # +X vira +Z
        m.apply_transform(T.rotation_matrix(math.pi, [0, 1, 0]))              # frente para baixo
    if nome == "patim":              # pino para baixo (cabeça da esfera para cima)
        m.apply_transform(T.rotation_matrix(math.pi, [1, 0, 0]))
        m.apply_transform(T.translation_matrix([0, 0, 0]))
    m.apply_translation(-m.bounds[0])
    m.apply_translation([0, 0, 0])
    return m

def main():
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "stl")
    os.makedirs(out, exist_ok=True)
    pecas = []
    ch = chassi();            pecas.append(("01_chassi", "chassi", ch, 1, "PLA", "Placa base com berços de motor, furos e fendas para abraçadeiras."))
    pt = patim();             pecas.append(("02_patim", "patim", pt, 1, "PLA", "Apoio liso. Cole o pino no furo da placa."))
    ss = suporte_sensor();    pecas.append(("03_suporte_sensor", "suporte_sensor", ss, 1, "PLA", "Suporte do HC-SR04 (2 parafusos M3×6 no chassi)."))
    ro = roda()
    # pose de montagem das rodas: eixo local Z -> Y
    def pose_roda(sinal):
        m = ro.copy()
        m.apply_transform(T.rotation_matrix(-math.pi / 2, [1, 0, 0]))        # z -> y
        yc = BASE_L / 2 + FOLGA_RODA_PLACA + RODA_L / 2
        if sinal < 0:
            m.apply_transform(T.rotation_matrix(math.pi, [0, 0, 1]))
        m.apply_translation([MOTOR_X, sinal * yc, BASE_E + N20_H / 2])
        return m
    pn = pneu()
    def pose_pneu(sinal):
        m = pn.copy()
        m.apply_transform(T.rotation_matrix(-math.pi / 2, [1, 0, 0]))
        yc = BASE_L / 2 + FOLGA_RODA_PLACA + RODA_L / 2
        m.apply_translation([MOTOR_X, sinal * yc, BASE_E + N20_H / 2])
        return m
    so = suporte_oled();      pecas.append(("06_suporte_oled", "suporte_oled", so, 1, "PLA", "Rampa inclinada com 4 pinos para o display e 2 furos de LED 5 mm."))
    te = teste_ajuste()
    # exportação
    montagem = {"params": {"BASE_C": BASE_C, "BASE_L": BASE_L, "BASE_E": BASE_E, "MOTOR_X": MOTOR_X, "RODA_D": RODA_D, "RODA_L": RODA_L,
                           "N20_W": N20_W, "N20_H": N20_H, "N20_C": N20_C, "PATIM_X": PATIM_X, "VAO": VAO, "SUP_PAREDE": SUP_PAREDE,
                           "SUP_L": SUP_L, "SUP_H": SUP_H, "FLANGE_SENS": FLANGE_SENS, "OLED_X_TRAS": OLED_X_TRAS, "OLED_INCL": OLED_INCL,
                           "OLED_COMP": OLED_COMP, "OLED_LARG": OLED_LARG, "FOLGA_RODA_PLACA": FOLGA_RODA_PLACA, "OLHO_DIST": OLHO_DIST},
                "partes": {}}
    relatorio = []
    def salvar(arquivo, nome_imp, malha_montagem, qtd, material, nota, instancias=None):
        imp = para_impressao(malha_montagem, nome_imp)
        assert imp.is_watertight, f"{arquivo}: malha não é fechada"
        assert imp.is_winding_consistent, f"{arquivo}: orientação das faces inconsistente"
        imp.export(os.path.join(out, arquivo + ".stl"))
        bb = imp.extents
        relatorio.append({"arquivo": arquivo + ".stl", "qtd": qtd, "material": material, "mm": [round(float(v), 1) for v in bb],
                          "volume_cm3": round(float(imp.volume) / 1000, 1), "triangulos": int(len(imp.faces)), "nota": nota})
        return imp
    for arq, nome, m, qtd, mat, nota in pecas:
        salvar(arq, nome, m, qtd, mat, nota)
    # rodas e pneus: o STL de impressão é a roda deitada (eixo em Z), quantidade 2
    r_imp = ro.copy(); r_imp.apply_translation(-r_imp.bounds[0])
    assert r_imp.is_watertight and r_imp.is_winding_consistent
    r_imp.export(os.path.join(out, "04_roda.stl"))
    relatorio.append({"arquivo": "04_roda.stl", "qtd": 2, "material": "PLA", "mm": [round(float(v), 1) for v in r_imp.extents],
                      "volume_cm3": round(float(r_imp.volume) / 1000, 1), "triangulos": int(len(r_imp.faces)), "nota": "Aro e cubo com furo em D de 3 mm."})
    p_imp = pn.copy(); p_imp.apply_translation(-p_imp.bounds[0])
    assert p_imp.is_watertight and p_imp.is_winding_consistent
    p_imp.export(os.path.join(out, "05_pneu_tpu.stl"))
    relatorio.append({"arquivo": "05_pneu_tpu.stl", "qtd": 2, "material": "TPU 95A (opcional)", "mm": [round(float(v), 1) for v in p_imp.extents],
                      "volume_cm3": round(float(p_imp.volume) / 1000, 1), "triangulos": int(len(p_imp.faces)), "nota": "Pneu de borracha. Pode usar um O-ring ou elástico no lugar."})
    assert te.is_watertight and te.is_winding_consistent
    te.export(os.path.join(out, "08_teste_ajuste.stl"))
    relatorio.append({"arquivo": "08_teste_ajuste.stl", "qtd": 1, "material": "PLA", "mm": [round(float(v), 1) for v in te.extents],
                      "volume_cm3": round(float(te.volume) / 1000, 1), "triangulos": int(len(te.faces)), "nota": "IMPRIMA PRIMEIRO: testa motor, olhos do sensor, pino do patim, parafuso e eixo em D."})

    # malhas na pose de montagem para a ilustração
    def empacota(m):
        v = np.asarray(m.vertices, dtype=np.float32).round(3)
        f = np.asarray(m.faces, dtype=np.uint32)
        return {"v": base64.b64encode(v.tobytes()).decode(), "f": base64.b64encode(f.tobytes()).decode(), "nv": int(len(v)), "nf": int(len(f))}
    montagem["partes"] = {"chassi": empacota(ch), "patim": empacota(pt), "suporte_sensor": empacota(ss), "suporte_oled": empacota(so),
                          "roda_d": empacota(pose_roda(1)), "roda_e": empacota(pose_roda(-1)),
                          "pneu_d": empacota(pose_pneu(1)), "pneu_e": empacota(pose_pneu(-1))}
    with open(os.path.join(os.path.dirname(out), "montagem.json"), "w") as fh:
        json.dump(montagem, fh, separators=(",", ":"))
    with open(os.path.join(out, "relatorio.json"), "w") as fh:
        json.dump(relatorio, fh, indent=1, ensure_ascii=False)
    print(f"VAO (fundo da placa ao chão) = {VAO:.1f} mm")
    for r in sorted(relatorio, key=lambda x: x["arquivo"]):
        print(f"{r['arquivo']:<24} x{r['qtd']}  {r['mm']}  {r['volume_cm3']:>6} cm³  {r['triangulos']:>6} tri  {r['material']}")

if __name__ == "__main__":
    main()
