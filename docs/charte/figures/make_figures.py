#!/usr/bin/env python3
"""Draws the figures of the notes (SVG, house colours) and converts them to PNG
with Inkscape: charte/figures/<name>.png (French), charte/figures/en/<name>.png.

Usage: python3 docs/charte/figures/make_figures.py
"""
import os
import subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
NAVY, CYAN, RED, GREY, TEXT = '#11203A', '#0FB5D6', '#B03A2E', '#6B7684', '#1A1A1A'
LIGHT, PALE = '#E7F7FB', '#F4F6F8'

TEXTS = {
    'fr': {
        'rbg': 'SP 800-90C : générateur complet (RBG1, RBG2 ou RBG3)',
        'es': "SP 800-90B : source d'entropie",
        'noise': 'Source de bruit',
        'raw': 'échantillons bruts (GetNoise)',
        'cond': ['Conditionnement', '(facultatif)'],
        'out_es': 'sortie de la source (GetEntropy)',
        'drbg': ['SP 800-90A : DRBG', 'Instantiate · Reseed · Generate'],
        'output': 'Sortie aléatoire',
        'A': ['A', 'Tests 1, 2 et 3', 'hors ligne : laboratoire ou', 'qualification sur la machine cible', '→ entropie validée H'],
        'B': ['B', 'RCT et APT', 'en continu, dans le produit,', 'sur chaque échantillon brut', '→ autorise ou bloque la graine'],
        'C': ['C', 'Test 4', 'calcul à la conception', '→ entropie h_out de la sortie'],
        'D': ['D', 'Tests à réponse connue (KAT)', "intégrité de l'algorithme,", "pas une mesure d'entropie"],
        'E': ['E', 'Aucun test SP 800-90B', 'un DRBG passe tous les tests,', 'même avec une graine nulle'],
        'legend': ['Hors ligne', 'En continu', 'Conception', 'Hors évaluation'],
    },
    'en': {
        'rbg': 'SP 800-90C: complete generator (RBG1, RBG2 or RBG3)',
        'es': 'SP 800-90B: entropy source',
        'noise': 'Noise source',
        'raw': 'raw samples (GetNoise)',
        'cond': ['Conditioning', '(optional)'],
        'out_es': 'source output (GetEntropy)',
        'drbg': ['SP 800-90A: DRBG', 'Instantiate · Reseed · Generate'],
        'output': 'Random output',
        'A': ['A', 'Tests 1, 2 and 3', 'offline: laboratory or', 'qualification on the target machine', '→ validated entropy H'],
        'B': ['B', 'RCT and APT', 'continuous, in the product,', 'on every raw sample', '→ allows or blocks the seed'],
        'C': ['C', 'Test 4', 'computed at design time', '→ entropy h_out of the output'],
        'D': ['D', 'Known-answer tests (KAT)', 'integrity of the algorithm,', 'not an entropy measurement'],
        'E': ['E', 'No SP 800-90B test', 'a DRBG passes every test,', 'even with an all-zero seed'],
        'legend': ['Offline', 'Continuous', 'Design time', 'Not assessed'],
    },
}


def esc(t):
    return t.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')


def text(x, y, t, size=15, color=TEXT, weight='normal', anchor='start', style='normal'):
    return ('<text x="%g" y="%g" font-family="Arial, Liberation Sans, sans-serif" font-size="%g" fill="%s" '
            'font-weight="%s" font-style="%s" text-anchor="%s">%s</text>' % (x, y, size, color, weight, style, anchor, esc(t)))


def box(x, y, w, h, fill, stroke, sw=1.5, rx=8, dash=None):
    d = ' stroke-dasharray="%s"' % dash if dash else ''
    return '<rect x="%g" y="%g" width="%g" height="%g" rx="%g" fill="%s" stroke="%s" stroke-width="%g"%s/>' % (x, y, w, h, rx, fill, stroke, sw, d)


def arrow(x1, y1, x2, y2, color=NAVY, sw=2, dash=None):
    d = ' stroke-dasharray="%s"' % dash if dash else ''
    return '<line x1="%g" y1="%g" x2="%g" y2="%g" stroke="%s" stroke-width="%g" marker-end="url(#h%s)"%s/>' % (
        x1, y1, x2, y2, color, sw, color[1:], d)


def badge(x, y, w, h, lines, color, dash=None):
    """A test: letter in a disc, title in bold, then comment lines."""
    out = [box(x, y, w, h, '#FFFFFF', color, 2, 8, dash),
           '<circle cx="%g" cy="%g" r="14" fill="%s"/>' % (x + 24, y + 26, color),
           text(x + 24, y + 31, lines[0], 15, '#FFFFFF', 'bold', 'middle'),
           text(x + 48, y + 31, lines[1], 16, color, 'bold')]
    for i, l in enumerate(lines[2:]):
        out.append(text(x + 48, y + 54 + 20 * i, l, 14, TEXT))
    return out


def placement(lang):
    t = TEXTS[lang]
    W, H = 1000, 760
    s = ['<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d">' % (W, H, W, H),
         '<defs>']
    for c in (NAVY, CYAN, RED, GREY):
        s.append('<marker id="h%s" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">'
                 '<path d="M0,0 L10,5 L0,10 z" fill="%s"/></marker>' % (c[1:], c))
    s.append('</defs>')
    s.append('<rect width="%d" height="%d" fill="#FFFFFF"/>' % (W, H))

    # SP 800-90C: the whole generator
    s.append(box(10, 10, 980, 740, PALE, NAVY, 1.5, 12, '8 5'))
    s.append(text(30, 40, t['rbg'], 17, NAVY, 'bold'))

    # SP 800-90B: the entropy source
    s.append(box(30, 60, 940, 400, LIGHT, CYAN, 2, 10))
    s.append(text(50, 90, t['es'], 17, NAVY, 'bold'))

    # chain: noise source -> conditioning -> DRBG -> output (left column)
    cx = 170
    s.append(box(60, 115, 220, 60, NAVY, NAVY, 1.5, 8))
    s.append(text(cx, 152, t['noise'], 17, '#FFFFFF', 'bold', 'middle'))
    s.append(arrow(cx, 175, cx, 325))
    s.append(text(cx + 10, 255, t['raw'], 13, GREY, 'normal', 'start', 'italic'))
    s.append(box(60, 330, 220, 64, '#FFFFFF', NAVY, 2, 8))
    s.append(text(cx, 357, t['cond'][0], 16, NAVY, 'bold', 'middle'))
    s.append(text(cx, 380, t['cond'][1], 14, GREY, 'normal', 'middle'))
    s.append(arrow(cx, 394, cx, 505))
    s.append(text(cx + 10, 480, t['out_es'], 13, GREY, 'normal', 'start', 'italic'))

    # SP 800-90A: DRBG
    s.append(box(60, 510, 300, 76, NAVY, NAVY, 1.5, 8))
    s.append(text(210, 542, t['drbg'][0], 17, '#FFFFFF', 'bold', 'middle'))
    s.append(text(210, 566, t['drbg'][1], 14, '#FFFFFF', 'normal', 'middle'))
    s.append(arrow(cx, 586, cx, 655))
    s.append(box(60, 660, 220, 56, '#FFFFFF', NAVY, 2, 8))
    s.append(text(cx, 694, t['output'], 16, NAVY, 'bold', 'middle'))

    # taps from the chain to the tests
    tap_y_a, tap_y_b = 210, 290
    s.append('<circle cx="%g" cy="%g" r="5" fill="%s"/>' % (cx, tap_y_a, CYAN))
    s.append(arrow(cx, tap_y_a, 438, 170, CYAN, 2))
    s.append('<circle cx="%g" cy="%g" r="5" fill="%s"/>' % (cx, tap_y_b, RED))
    s.append(arrow(cx, tap_y_b, 438, 285, RED, 2))
    s.append(arrow(280, 362, 438, 395, NAVY, 2, '6 4'))
    s.append(arrow(360, 548, 438, 548, GREY, 2, '6 4'))
    s.append(arrow(280, 688, 438, 688, RED, 2, '6 4'))

    # tests A to E (right column)
    s += badge(440, 110, 500, 120, t['A'], CYAN)
    s += badge(440, 240, 500, 120, t['B'], RED)
    s += badge(440, 370, 500, 82, t['C'][:2] + [t['C'][2] + '  ' + t['C'][3]], NAVY)
    s += badge(440, 505, 500, 82, t['D'][:2] + [t['D'][2] + ' ' + t['D'][3]], GREY)
    s += badge(440, 645, 500, 82, t['E'][:2] + [t['E'][2] + ' ' + t['E'][3]], RED, '6 4')
    s.append('</svg>')
    return '\n'.join(s)


VALID = {
    'fr': {'top': ['libsp80090b 2.0.0', "face à l'outil du NIST SP800-90B_EntropyAssessment 1.1.8"],
           'b': [['Selftest du NIST', 'voie non IID', '11 jeux de référence', 'écart max 7,2 × 10⁻¹⁴'],
                 ['Voie IID', 'contre ea_iid', 'statistiques déterministes', 'identiques au bit près'],
                 ['Campagne des labs', 'tests 1 à 4', '22 jeux, 52 cas', 'conformes à la théorie'],
                 ["Revue du code et de l'API", 'constats de la v1.0', 'E1 à E8, R1 à R4 clos', 'M2, C2, L1 ouverts']]},
    'en': {'top': ['libsp80090b 2.0.0', 'against the NIST tool SP800-90B_EntropyAssessment 1.1.8'],
           'b': [['NIST selftest', 'non-IID track', '11 reference datasets', 'largest gap 7.2 × 10⁻¹⁴'],
                 ['IID track', 'against ea_iid', 'deterministic statistics', 'bit-for-bit identical'],
                 ['Lab campaign', 'tests 1 to 4', '22 datasets, 52 cases', 'agree with the theory'],
                 ['Code and API review', 'findings of v1.0', 'E1 to E8, R1 to R4 closed', 'M2, C2, L1 open']]},
}


def validation(lang):
    t = VALID[lang]
    W, H = 1000, 400
    s = ['<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d">' % (W, H, W, H), '<defs>']
    s.append('<marker id="h%s" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto">'
             '<path d="M0,0 L10,5 L0,10 z" fill="%s"/></marker>' % (NAVY[1:], NAVY))
    s.append('</defs><rect width="%d" height="%d" fill="#FFFFFF"/>' % (W, H))
    s.append(box(250, 20, 500, 80, NAVY, NAVY, 1.5, 10))
    s.append(text(500, 54, t['top'][0], 20, '#FFFFFF', 'bold', 'middle'))
    s.append(text(500, 80, t['top'][1], 14, '#FFFFFF', 'normal', 'middle'))
    s.append('<line x1="500" y1="100" x2="500" y2="140" stroke="%s" stroke-width="2"/>' % NAVY)
    xs = [30, 270, 510, 750]
    s.append('<line x1="%g" y1="140" x2="%g" y2="140" stroke="%s" stroke-width="2"/>' % (xs[0] + 110, xs[-1] + 110, NAVY))
    colors = [CYAN, CYAN, CYAN, NAVY]
    for x, b, c in zip(xs, t['b'], colors):
        s.append(arrow(x + 110, 140, x + 110, 178))
        s.append(box(x, 180, 220, 190, '#FFFFFF', c, 2, 10))
        s.append('<rect x="%g" y="180" width="220" height="56" rx="10" fill="%s"/>' % (x, c))
        s.append('<rect x="%g" y="220" width="220" height="16" fill="%s"/>' % (x, c))
        s.append(text(x + 110, 206, b[0], 16, '#FFFFFF', 'bold', 'middle'))
        s.append(text(x + 110, 227, b[1], 13, '#FFFFFF', 'normal', 'middle'))
        s.append(text(x + 110, 285, b[2], 15, TEXT, 'normal', 'middle'))
        s.append(text(x + 110, 325, b[3], 15, NAVY, 'bold', 'middle'))
    s.append('</svg>')
    return '\n'.join(s)


PERF = {
    'fr': {'title': 'Durée par fichier de 10⁶ échantillons (échelle logarithmique)',
           'rows': [('Lab 4 : fonction approuvée', 0.01, 0.01, 'instantané'),
                    ('Lab 2 : fichier de 1 bit', 5, 7, '5 à 7 s'),
                    ('Lab 4 : mesure de h′', 7, 38, '7 à 38 s'),
                    ('Lab 1 : fichier IID', 16, 57, '16 à 57 s'),
                    ('Lab 2 : fichier de 4 ou 8 bits', 31, 53, '31 à 53 s'),
                    ('Lab 3 : redémarrage 1 000 × 1 000', 46, 74, '46 à 74 s'),
                    ('Lab 1 : 10 000 permutations, sans bzip2', 120, 180, '2 à 3 min'),
                    ('Lab 2 : replay', 474, 474, '7 min 54 s'),
                    ('Lab 1 : compression bzip2 en échec', 2460, 3420, '41 à 57 min')],
           'ticks': ['0,01 s', '0,1 s', '1 s', '10 s', '1 min', '10 min', '1 h', '3 h']},
    'en': {'title': 'Time per file of 10⁶ samples (logarithmic scale)',
           'rows': [('Lab 4: vetted function', 0.01, 0.01, 'instantaneous'),
                    ('Lab 2: 1-bit file', 5, 7, '5 to 7 s'),
                    ('Lab 4: measuring h′', 7, 38, '7 to 38 s'),
                    ('Lab 1: IID file', 16, 57, '16 to 57 s'),
                    ('Lab 2: 4- or 8-bit file', 31, 53, '31 to 53 s'),
                    ('Lab 3: restart 1,000 × 1,000', 46, 74, '46 to 74 s'),
                    ('Lab 1: 10,000 permutations, no bzip2', 120, 180, '2 to 3 min'),
                    ('Lab 2: replay', 474, 474, '7 min 54 s'),
                    ('Lab 1: bzip2 compression failing', 2460, 3420, '41 to 57 min')],
           'ticks': ['0.01 s', '0.1 s', '1 s', '10 s', '1 min', '10 min', '1 h', '3 h']},
}


def performance(lang):
    import math
    t = PERF[lang]
    W, H = 1000, 520
    x0, x1 = 330, 900                       # axis from 0.01 s to 3 h
    lo, hi = math.log10(0.01), math.log10(3 * 3600)

    def X(sec):
        return x0 + (math.log10(sec) - lo) / (hi - lo) * (x1 - x0)
    s = ['<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d">' % (W, H, W, H)]
    s.append('<rect width="%d" height="%d" fill="#FFFFFF"/>' % (W, H))
    s.append(text(W / 2, 30, t['title'], 17, NAVY, 'bold', 'middle'))
    top, step = 60, 44
    bottom = top + step * len(t['rows'])
    for sec, lab in zip([0.01, 0.1, 1, 10, 60, 600, 3600, 10800], t['ticks']):
        x = X(sec)
        s.append('<line x1="%g" y1="%g" x2="%g" y2="%g" stroke="#D3D9DF" stroke-width="1"/>' % (x, top - 5, x, bottom))
        s.append(text(x, bottom + 22, lab, 13, GREY, 'normal', 'middle'))
    for i, (name, a, b, lab) in enumerate(t['rows']):
        y = top + i * step + 8
        c = RED if b >= 2000 else (NAVY if b >= 200 else CYAN)
        s.append(text(x0 - 12, y + 19, name, 14, TEXT, 'normal', 'end'))
        xa, xb = X(a), X(b)
        s.append('<rect x="%g" y="%g" width="%g" height="26" rx="5" fill="%s"/>' % (x0, y, max(xb - x0, 4), c))
        if b > a:
            s.append('<rect x="%g" y="%g" width="%g" height="26" rx="5" fill="#FFFFFF" fill-opacity="0.35"/>' % (x0, y, xa - x0))
        s.append(text(xb + 8, y + 19, lab, 13, NAVY, 'bold'))
    s.append('<line x1="%g" y1="%g" x2="%g" y2="%g" stroke="%s" stroke-width="1.5"/>' % (x0, top - 5, x0, bottom, NAVY))
    s.append('</svg>')
    return '\n'.join(s)


PROTO = {
    'fr': [('Modéliser', ['dossier physique', '→ H_claim, IID ou non']),
           ('Capturer', ['≥ 10⁶ échantillons bruts', 'au pire point de marche']),
           ('Choisir la voie', ['IID : argument + 3 tests', 'iid_evaluate_ex()']),
           ('Estimer H_I', ['voie non IID par défaut', 'non_iid_evaluate_ex()']),
           ('Redémarrer', ['1 000 × 1 000 échantillons', 'restart_evaluate()']),
           ('Règle du minimum', ['H = min(H_claim, H_I,', '         H_r, H_c)']),
           ('Dimensionner', ['graine, RCT, APT, marge', 'conditioning_evaluate()']),
           ('Surveiller', ['RCT et APT en continu', 'dans le produit'])],
    'en': [('Model', ['physical model', '→ H_claim, IID or not']),
           ('Capture', ['≥ 10⁶ raw samples', 'at the worst operating point']),
           ('Choose the track', ['IID: argument + 3 tests', 'iid_evaluate_ex()']),
           ('Estimate H_I', ['non-IID track by default', 'non_iid_evaluate_ex()']),
           ('Restart', ['1,000 × 1,000 samples', 'restart_evaluate()']),
           ('Minimum rule', ['H = min(H_claim, H_I,', '         H_r, H_c)']),
           ('Size', ['seed, RCT, APT, margin', 'conditioning_evaluate()']),
           ('Monitor', ['RCT and APT, continuous', 'in the product'])],
}


def protocole(lang):
    steps = PROTO[lang]
    W, H = 1000, 380
    s = ['<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d">' % (W, H, W, H), '<defs>']
    s.append('<marker id="h%s" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto">'
             '<path d="M0,0 L10,5 L0,10 z" fill="%s"/></marker>' % (NAVY[1:], NAVY))
    s.append('</defs><rect width="%d" height="%d" fill="#FFFFFF"/>' % (W, H))
    xs, bw, bh = [20, 265, 510, 755], 225, 120
    pos = [(x, 30) for x in xs] + [(x, 230) for x in reversed(xs)]   # snake: row 1 left to right, row 2 back
    for i, ((x, y), (title, lines)) in enumerate(zip(pos, steps)):
        c = NAVY if i == 5 else (RED if i == 7 else CYAN)
        s.append(box(x, y, bw, bh, '#FFFFFF', c, 2, 10))
        s.append('<circle cx="%g" cy="%g" r="15" fill="%s"/>' % (x + 26, y + 28, c))
        s.append(text(x + 26, y + 33, str(i + 1), 15, '#FFFFFF', 'bold', 'middle'))
        s.append(text(x + 50, y + 34, title, 16, c, 'bold'))
        for k, l in enumerate(lines):
            mono = l.endswith('()')
            s.append(text(x + 14, y + 72 + 24 * k, l, 13 if mono else 14, NAVY if mono else TEXT,
                          'bold' if mono else 'normal'))
    for i in range(3):                                   # row 1 arrows, then down, then row 2 arrows
        s.append(arrow(xs[i] + bw, 90, xs[i + 1] - 2, 90))
    s.append(arrow(xs[3] + bw / 2, 30 + bh, xs[3] + bw / 2, 228))
    for i in range(3, 0, -1):
        s.append(arrow(xs[i], 290, xs[i - 1] + bw + 2, 290))
    s.append('</svg>')
    return '\n'.join(s)


def main():
    for lang in ('fr', 'en'):
        out = HERE if lang == 'fr' else os.path.join(HERE, 'en')
        os.makedirs(out, exist_ok=True)
        for name, draw in (('placement', placement), ('validation', validation), ('performance', performance),
                           ('protocole', protocole)):
            svg = os.path.join(out, name + '.svg')
            open(svg, 'w', encoding='utf8').write(draw(lang))
            subprocess.run(['inkscape', svg, '--export-type=png', '--export-dpi=192',
                            '--export-filename=' + os.path.join(out, name + '.png')], check=True,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            print(os.path.join(out, name + '.png'))


if __name__ == '__main__':
    main()
