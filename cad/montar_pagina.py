#!/usr/bin/env python3
"""Gera docs/montagem-3d.html a partir do modelo docs/montagem-3d.template.html + cad/montagem.json + cad/stl/relatorio.json.
Rode depois de gerar_stl.py:  python3 cad/montar_pagina.py"""
import json, os
aqui = os.path.dirname(os.path.abspath(__file__))
raiz = os.path.dirname(aqui)
modelo = open(os.path.join(raiz, "docs", "montagem-3d.template.html"), encoding="utf-8").read()
dados = open(os.path.join(aqui, "montagem.json"), encoding="utf-8").read()
rel = open(os.path.join(aqui, "stl", "relatorio.json"), encoding="utf-8").read()
assert "/*#dados*/null" in modelo and "/*#relatorio*/null" in modelo
saida = modelo.replace("/*#dados*/null", dados).replace("/*#relatorio*/null", rel)
destino = os.path.join(raiz, "docs", "montagem-3d.html")
open(destino, "w", encoding="utf-8").write(saida)
print(destino, round(len(saida) / 1024), "KB")
