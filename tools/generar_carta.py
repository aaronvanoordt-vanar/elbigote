#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Genera el bloque de la carta dentro de index.html a partir de tools/carta_datos.py.

    python3 tools/generar_carta.py

Reescribe lo que hay entre los marcadores <!-- CARTA:INICIO --> y <!-- CARTA:FIN -->
(y el bloque de datos estructurados entre CARTA-JSONLD:INICIO / FIN),
y vuelca data/carta.json para quien quiera consumir la carta desde fuera.

La carta se escribe en español e inglés a la vez: cada texto sale dos veces,
marcado con lang="es" y lang="en". El CSS enseña uno y esconde el otro, así que
el cambio de idioma es instantáneo y ambos idiomas quedan en el HTML para Google.
"""
import json, os, re, sys, unicodedata

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(RAIZ, "tools"))
from carta_datos import CATEGORIAS

ETIQUETAS = {
    "top": ("El más pedido", "Most ordered"),
    "veg": ("Sin carne", "Meat-free"),
}

def esc(t):
    return (t.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")) if t else ""

def sin_tildes(t):
    return "".join(c for c in unicodedata.normalize("NFD", t or "")
                   if unicodedata.category(c) != "Mn").lower()

def bilingue(es, en, clase=""):
    c = f' class="{clase}"' if clase else ""
    return f'<span lang="es"{c}>{esc(es)}</span><span lang="en"{c}>{esc(en)}</span>'

def html_carta():
    out = ['<div class="menu-groups" id="menuGroups">']
    for c in CATEGORIAS:
        gid = f"g-{c['id']}"
        out.append(f'\n  <section class="menu-group reveal" data-cat="{c["cat"]}" aria-labelledby="{gid}">')
        lid = f"lista-{c['id']}"
        # Patrón de acordeón: en móvil la cabecera pliega la sección.
        out.append('    <div class="menu-group__head">')
        out.append(f'      <h3 id="{gid}"><button class="menu-group__toggle" type="button"'
                   f' aria-expanded="true" aria-controls="{lid}">')
        out.append(f'        <span class="menu-group__title">{bilingue(c["es"], c["en"])}</span>')
        out.append('        <span class="menu-group__count" data-count></span>')
        out.append('        <svg class="menu-group__chev" viewBox="0 0 24 24" fill="none" stroke="currentColor"'
                   ' stroke-width="2" stroke-linecap="round" aria-hidden="true"><path d="m6 9 6 6 6-6"/></svg>')
        out.append('      </button></h3>')
        out.append('    </div>')
        if c.get("nota_es"):
            out.append(f'    <p class="menu-group__note">{bilingue(c["nota_es"], c["nota_en"])}</p>')
        out.append(f'    <ul class="menu-list" id="{lid}">')
        for nes, nen, precio, des, den, flags in c["items"]:
            buscar = sin_tildes(" ".join(filter(None, [nes, nen, des, den])))
            buscar = re.sub(r"\s+", " ", buscar)[:400]
            out.append(f'      <li class="menu-item" data-buscar="{esc(buscar)}">')
            marcas = "".join(
                f'<span class="menu-item__flag" data-flag="{f}">{bilingue(*ETIQUETAS[f])}</span>'
                for f in flags if f in ETIQUETAS)
            out.append(f'        <p class="menu-item__name">{bilingue(nes, nen)}{marcas}</p>')
            if precio:
                out.append(f'        <p class="menu-item__price num">S/ {esc(precio)}</p>')
            if des:
                out.append(f'        <p class="menu-item__desc">{bilingue(des, den)}</p>')
            out.append('      </li>')
        out.append('    </ul>')
        out.append('  </section>')
    out.append('\n</div>')
    return "\n".join(out)

def jsonld_carta(base):
    secciones = []
    for c in CATEGORIAS:
        items = []
        for nes, nen, precio, des, den, _ in c["items"]:
            it = {"@type": "MenuItem", "name": nes}
            if des:
                it["description"] = des
            if precio and re.fullmatch(r"[\d.]+", precio):
                it["offers"] = {"@type": "Offer", "price": precio, "priceCurrency": "PEN"}
            items.append(it)
        secciones.append({"@type": "MenuSection", "name": c["es"], "hasMenuItem": items})
    menu = {
        "@type": "Menu",
        "@id": f"{base}#carta",
        "name": "Carta de El Bigote Coffee & Waffles",
        "inLanguage": "es-PE",
        "hasMenuSection": secciones,
    }
    return json.dumps(menu, ensure_ascii=False, indent=2)

def reemplazar(texto, marca, nuevo):
    ini, fin = f"<!-- {marca}:INICIO -->", f"<!-- {marca}:FIN -->"
    patron = re.compile(re.escape(ini) + r".*?" + re.escape(fin), re.S)
    if not patron.search(texto):
        raise SystemExit(f"No se encontraron los marcadores {marca} en index.html")
    return patron.sub(lambda _: f"{ini}\n{nuevo}\n{fin}", texto)

def main():
    base = "https://elbigote.pe/"
    ruta = os.path.join(RAIZ, "index.html")
    with open(ruta, encoding="utf-8") as f:
        html = f.read()

    html = reemplazar(html, "CARTA", html_carta())
    html = reemplazar(html, "CARTA-JSONLD",
                      '<script type="application/ld+json">\n' + jsonld_carta(base) + '\n</script>')
    with open(ruta, "w", encoding="utf-8") as f:
        f.write(html)

    datos = [{"id": c["id"], "categoria_es": c["es"], "categoria_en": c["en"], "filtros": c["cat"].split(),
              "nota_es": c.get("nota_es"), "nota_en": c.get("nota_en"),
              "platos": [{"nombre_es": a, "nombre_en": b, "precio_pen": p,
                          "descripcion_es": d, "descripcion_en": e, "etiquetas": f}
                         for a, b, p, d, e, f in c["items"]]}
             for c in CATEGORIAS]
    os.makedirs(os.path.join(RAIZ, "data"), exist_ok=True)
    with open(os.path.join(RAIZ, "data", "carta.json"), "w", encoding="utf-8") as f:
        json.dump({"negocio": "El Bigote Coffee & Waffles", "moneda": "PEN",
                   "categorias": datos}, f, ensure_ascii=False, indent=2)

    n = sum(len(c["items"]) for c in CATEGORIAS)
    print(f"Carta regenerada · {len(CATEGORIAS)} secciones · {n} platos")

if __name__ == "__main__":
    main()
