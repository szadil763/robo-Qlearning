// LEGADO: primeira versão em OpenSCAD, nunca renderizada nem impressa. Use cad/gerar_stl.py (fonte atual dos STL).
// As medidas aqui podem divergir das do gerador em Python.
// Chassi paramétrico do Robô Desviador de Obstáculos
// OpenSCAD. Eixo X = frente do robô. Unidades em mm.
//
// ATENÇÃO: modelo inicial, ainda NÃO testado em impressão.
// Imprima primeiro só uma peça de teste (suporte de motor e furos do sensor)
// e ajuste as folgas (parâmetros "folga_*") antes de imprimir tudo.
//
// Como usar: ajuste os parâmetros abaixo, renderize (F6) e exporte STL.

$fn = 48;

// ---------- Placa base ----------
base_c = 110;     // comprimento (X)
base_l = 85;      // largura (Y)
base_e = 3;       // espessura
raio   = 8;       // arredondamento dos cantos

// ---------- Motor N20 com redução ----------
folga_motor = 0.3;
n20_w = 12 + folga_motor;     // largura do corpo
n20_h = 10 + folga_motor;     // altura do corpo
n20_c = 24;                   // comprimento do conjunto motor + caixa de redução (confira o seu)
parede = 2.4;
motor_x = -15;                // posição do eixo das rodas em relação ao centro

// ---------- Rodas ----------
roda_d = 42;                  // diâmetro da roda (com pneu)

// ---------- Sensor HC-SR04 ----------
folga_sensor = 0.4;
sensor_furo = 16 + folga_sensor;   // diâmetro de cada "olho"
sensor_dist = 26;                  // distância entre os centros dos olhos (confira o seu)
sensor_l    = 56;                  // largura da placa-suporte
sensor_h    = 26;                  // altura da placa-suporte
sensor_z    = 16;                  // altura do centro dos furos acima da base

// ---------- Apoio frontal (patim) ----------
patim_d = 10;
patim_x = base_c/2 - 22;

// ---------- Cálculos ----------
// Distância do fundo da base até o chão para o eixo ficar na altura certa
vao = roda_d/2 - (base_e + n20_h/2);

module placa() {
    linear_extrude(base_e)
        hull()
            for (x = [-1, 1], y = [-1, 1])
                translate([x * (base_c/2 - raio), y * (base_l/2 - raio)])
                    circle(raio);
}

module suporte_motor(lado) {
    // lado = +1 (esquerda) ou -1 (direita). Eixo do motor aponta para fora.
    translate([motor_x, lado * (base_l/2 - n20_c/2), base_e])
        difference() {
            translate([-(n20_w/2 + parede), -n20_c/2, 0])
                cube([n20_w + 2*parede, n20_c, n20_h + parede]);
            // cavidade do motor, aberta em cima
            translate([-n20_w/2, -n20_c/2 - 1, 0])
                cube([n20_w, n20_c + 2, n20_h + parede + 1]);
        }
}

module suporte_sensor() {
    x0 = base_c/2 - 8;
    // placa vertical com os dois furos
    translate([x0, -sensor_l/2, base_e])
        difference() {
            cube([3, sensor_l, sensor_h]);
            for (s = [-1, 1])
                translate([-1, sensor_l/2 + s*sensor_dist/2, sensor_z - base_e])
                    rotate([0, 90, 0]) cylinder(d = sensor_furo, h = 5);
        }
    // reforços triangulares atrás da placa
    for (s = [-1, 1])
        translate([x0 - 8, s * (sensor_l/2 - 1.5) - 1.5, base_e])
            hull() {
                cube([8, 3, 0.1]);
                translate([7.9, 0, 0]) cube([0.1, 3, sensor_h - 6]);
            }
}

module patim() {
    // apoio liso embaixo da base, no lugar de uma roda boba
    translate([patim_x, 0, 0]) {
        hull() {
            translate([0, 0, -vao + patim_d/2]) sphere(d = patim_d);
            translate([0, 0, -0.01]) cylinder(d = patim_d, h = 0.01);
        }
    }
}

module chassi() {
    difference() {
        union() {
            placa();
            suporte_motor(1);
            suporte_motor(-1);
            suporte_sensor();
            patim();
        }
        // furos para abraçadeiras de nylon ao lado de cada motor
        for (lado = [-1, 1], dx = [-1, 1])
            translate([motor_x + dx * (n20_w/2 + parede + 3),
                       lado * (base_l/2 - n20_c/2) - 1.5, -1])
                cube([3, 8, base_e + 2]);
        // furos de passagem de fios
        for (x = [-30, 0, 30])
            translate([x, 0, -1]) cylinder(d = 6, h = base_e + 2);
    }
}

chassi();

// Dica de montagem:
// - Caixa de 2 baterias 18650: prender no centro/rabeira com abraçadeiras ou fita dupla face.
// - ESP32 e protoboard: fita dupla face ou parafusos M3 em furos adicionais conforme seu módulo.
// - Se o robô ficar "de nariz" para baixo ou para cima, ajuste patim_d, vao ou a posição motor_x.
