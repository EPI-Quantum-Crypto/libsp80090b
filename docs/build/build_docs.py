#!/usr/bin/env python3
"""Builds the libsp80090b publications (docx + pdf) with the house style of note v3.

    python3 docs/build/build_docs.py            # every document of docs/pub/fr/ and docs/pub/en/
    python3 docs/build/build_docs.py NT-02      # one document (both languages)
    python3 docs/build/build_docs.py NT-02_en   # one document, one language
    python3 docs/build/build_docs.py --docx     # docx only (no LibreOffice)

Sources: docs/pub/<lang>/NT-xx_<lang>.md, next to their publications (syntax in
         docs/README.md); the header field
         `langue: fr` or `langue: en` selects the labels (fr by default).
Style:   docs/charte/ (docx template taken from note v3, logo, equations;
         English equations in docs/charte/equations/en/).
Output:  docs/pub/<lang>/<REF>_libsp80090b_v<version>_<lang>.docx and .pdf

Requires Python 3 (standard library), LibreOffice and pdftotext for the PDF.
"""
import datetime
import html
import os
import re
import shutil
import subprocess
import sys
import tempfile
import struct
import zipfile

DOCS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CHARTE = os.path.join(DOCS, 'charte')
PUB = os.path.join(DOCS, 'pub')

# House colours
NAVY, CYAN, RED, GREY, TEXT = '11203A', '0FB5D6', 'B03A2E', '6B7684', '1A1A1A'
PAGE_W = 9638  # text width in DXA (A4, 1134 margins)

# Language-dependent labels
LABELS = {
    'fr': {'table': 'Tableau', 'figure': 'Figure', 'toc': 'Sommaire', 'abstract': 'RÉSUMÉ', 'keywords': 'Mots-clés : ',
           'motto': "Cryptographie appliquée et évaluation des sources d'entropie",
           'cover': ['RÉFÉRENCE', 'OBJET', 'VERSION', 'AUTEURS', 'DIFFUSION', 'RÉFÉRENTIELS', 'DOCUMENTS LIÉS', 'REMPLACE'],
           'page': 'Page '},
    'en': {'table': 'Table', 'figure': 'Figure', 'toc': 'Contents', 'abstract': 'ABSTRACT', 'keywords': 'Keywords: ',
           'motto': 'Applied cryptography and entropy source assessment',
           'cover': ['REFERENCE', 'SUBJECT', 'VERSION', 'AUTHORS', 'DISTRIBUTION', 'STANDARDS', 'RELATED DOCUMENTS', 'SUPERSEDES'],
           'page': 'Page '},
}
L = LABELS['fr']


def esc(s):
    return html.escape(s, quote=True).replace('&#x27;', '&apos;')


# ---------------------------------------------------------------- inline text

def rpr(size, color=TEXT, bold=False, italic=False, mono=False, va=None, spacing=None):
    font = 'Consolas' if mono else 'Arial'
    x = '<w:rFonts w:ascii="%s" w:cs="%s" w:eastAsia="%s" w:hAnsi="%s"/>' % ((font,) * 4)
    x += '<w:b/><w:bCs/>' if bold else '<w:b w:val="false"/><w:bCs w:val="false"/>'
    x += '<w:i/><w:iCs/>' if italic else '<w:i w:val="false"/><w:iCs w:val="false"/>'
    x += '<w:color w:val="%s"/>' % color
    if spacing:
        x += '<w:spacing w:val="%d"/>' % spacing
    x += '<w:sz w:val="%d"/><w:szCs w:val="%d"/>' % (size, size)
    if va:
        x += '<w:vertAlign w:val="%s"/>' % va
    return '<w:rPr>%s</w:rPr>' % x


def run(text, **kw):
    return '<w:r>%s<w:t xml:space="preserve">%s</w:t></w:r>' % (rpr(**kw), esc(text))


INLINE = re.compile(r'\\(.)|`([^`]*)`|(\*\*)|(\*)|(~)|(\^)')


def inline(text, size=20, color=TEXT, bold=False, bold_color=NAVY, code_color=RED):
    """Inline Markdown -> runs: **bold**, *italic*, `code`, H~subscript~, 2^superscript^."""
    out, buf = [], []
    st = {'b': bold, 'i': False, 'va': None}

    def flush():
        if buf:
            t = ''.join(buf)
            out.append(run(t, size=size, color=bold_color if st['b'] else color,
                           bold=st['b'], italic=st['i'], va=st['va']))
            buf.clear()

    pos = 0
    for m in INLINE.finditer(text):
        buf.append(text[pos:m.start()])
        pos = m.end()
        if m.group(1) is not None:
            buf.append(m.group(1))
        elif m.group(2) is not None:
            flush()
            out.append(run(m.group(2), size=size - 2, color=code_color,
                           mono=True, va=st['va']))
        else:
            flush()
            if m.group(3):
                st['b'] = not st['b']
            elif m.group(4):
                st['i'] = not st['i']
            elif m.group(5):
                st['va'] = None if st['va'] == 'subscript' else 'subscript'
            else:
                st['va'] = None if st['va'] == 'superscript' else 'superscript'
    buf.append(text[pos:])
    flush()
    return ''.join(out)


# ---------------------------------------------------------------- blocks

def para(body, ppr='<w:keepNext w:val="false"/><w:jc w:val="both"/>'):
    return '<w:p><w:pPr>%s</w:pPr>%s</w:p>' % (ppr, body)


SPACER = '<w:p><w:pPr><w:spacing w:after="0" w:before="0" w:line="200" w:lineRule="exact"/></w:pPr></w:p>'
NONE_B = '<w:%s w:val="none" w:color="FFFFFF" w:sz="0"/>'


def borders(tag, top=None, left=None, bottom=None, right=None, inside_h=None, inside_v=False):
    def b(name, v):
        return ('<w:%s w:val="single" w:color="%s" w:sz="%d"/>' % (name, v[0], v[1])) if v else NONE_B % name
    x = b('top', top) + b('left', left) + b('bottom', bottom) + b('right', right)
    if tag == 'tblBorders':
        x += b('insideH', inside_h) + b('insideV', None)
    return '<w:%s>%s</w:%s>' % (tag, x, tag)


def tcmar(t, l, b, r):
    return ('<w:tcMar><w:top w:type="dxa" w:w="%d"/><w:left w:type="dxa" w:w="%d"/>'
            '<w:bottom w:type="dxa" w:w="%d"/><w:right w:type="dxa" w:w="%d"/></w:tcMar>' % (t, l, b, r))


def tbl(widths, rows, tblborders):
    grid = ''.join('<w:gridCol w:w="%d"/>' % w for w in widths)
    return ('<w:tbl><w:tblPr><w:tblW w:type="dxa" w:w="%d"/>%s<w:tblLayout w:type="fixed"/></w:tblPr>'
            '<w:tblGrid>%s</w:tblGrid>%s</w:tbl>' % (sum(widths), tblborders, grid, ''.join(rows)))


def box(paragraphs, border, fill, mar):
    """One-column box with a left rule (abstract, key point, warning, code)."""
    tcb = borders('tcBorders', left=border)
    cell = '<w:tc><w:tcPr><w:tcW w:type="dxa" w:w="%d"/>%s<w:shd w:fill="%s" w:color="auto" w:val="clear"/>%s</w:tcPr>%s</w:tc>'
    rows = []
    for fillc, marc, ps in zip(fill, mar, paragraphs):
        rows.append('<w:tr><w:trPr><w:cantSplit/></w:trPr>%s</w:tr>' % (cell % (PAGE_W, tcb, fillc, tcmar(*marc), ps)))
    return tbl([PAGE_W], rows, borders('tblBorders', left=border))


def callout(text, warn=False):
    border = ('D9572B', 24) if warn else (CYAN, 24)
    fill = 'FEF3EE' if warn else 'F0FBFE'
    ps = ''.join(para(inline(p, size=19, bold_color=RED if warn else NAVY),
                      '<w:spacing w:after="%d" w:before="0" w:line="255" w:lineRule="auto"/><w:jc w:val="both"/>'
                      % (0 if i == len(text) - 1 else 90))
                 for i, p in enumerate(text))
    return box([ps], border, [fill], [(130, 220, 130, 220)]) + SPACER


def codeblock(title, lines):
    head = para(run(title, size=16, color=NAVY, bold=True), '<w:keepNext/><w:spacing w:after="0" w:before="0"/>')
    short = len(lines) <= 40
    keep = '<w:keepNext/><w:keepLines/>' if short else ''
    body = ''.join(
        '<w:p><w:pPr>%s<w:spacing w:after="0" w:before="0" w:line="206" w:lineRule="exact"/>'
        '</w:pPr><w:r><w:rPr><w:rFonts w:ascii="Consolas" w:cs="Consolas" w:eastAsia="Consolas" w:hAnsi="Consolas"/>'
        '<w:color w:val="1A1A1A"/><w:sz w:val="16"/><w:szCs w:val="16"/></w:rPr><w:t xml:space="preserve">%s</w:t></w:r></w:p>'
        % (keep, esc(l if l.strip() else ' ')) for l in lines)
    x = box([head, body], ('8A96A3', 18), ['E6E9ED', 'F5F6F8'], [(50, 180, 50, 140), (100, 180, 110, 140)])
    if not short:  # a long block may break across pages instead of leaving an empty page
        x = x.replace('<w:trPr><w:cantSplit/></w:trPr>', '<w:trPr><w:tblHeader/></w:trPr>', 1).replace(
            '<w:trPr><w:cantSplit/></w:trPr>', '<w:trPr/>')
    return x + SPACER


def col_widths(rows, spec):
    n = len(rows[0])
    if spec:
        w = [float(v) for v in spec.split(',')]
    else:
        w = []
        for j in range(n):
            lens = [len(re.sub(r'[`*~^\\]', '', r[j])) for r in rows if j < len(r)]
            w.append(max(6, min(60, max(lens))) * 0.6 + max(4, min(45, sum(lens) / len(lens))) * 0.4)
    tot = sum(w)
    ws = [int(PAGE_W * v / tot) for v in w]
    ws[-1] += PAGE_W - sum(ws)
    return ws


def table(rows, widths):
    ws = col_widths(rows, widths)
    cpp = '<w:keepNext/><w:spacing w:after="0" w:before="0" w:line="245" w:lineRule="auto"/><w:jc w:val="left"/>'
    out = []
    for i, r in enumerate(rows):
        cells = []
        for j, c in enumerate(r):
            if i == 0:
                tcpr = ('<w:tcW w:type="dxa" w:w="%d"/>%s<w:shd w:fill="E4F5FA" w:color="auto" w:val="clear"/>%s'
                        '<w:vAlign w:val="bottom"/>' % (ws[j], borders('tcBorders', top=(NAVY, 10), bottom=(NAVY, 6)),
                                                         tcmar(80, 110, 80, 110)))
                body = inline(c, size=18, bold=True)
            else:
                tcpr = '<w:tcW w:type="dxa" w:w="%d"/>%s<w:vAlign w:val="top"/>' % (ws[j], tcmar(60, 110, 60, 110))
                body = inline(c, size=18)
            cells.append('<w:tc><w:tcPr>%s</w:tcPr>%s</w:tc>' % (tcpr, para(body, cpp)))
        hdr = '<w:tblHeader/>' if i == 0 else '<w:tblHeader w:val="false"/>'
        out.append('<w:tr><w:trPr><w:cantSplit/>%s</w:trPr>%s</w:tr>' % (hdr, ''.join(cells)))
    return tbl(ws, out, borders('tblBorders', top=(NAVY, 10), bottom=(NAVY, 10), inside_h=('D3D9DF', 4))) + SPACER


def caption(num, text):
    return ('<w:p><w:pPr><w:pStyle w:val="Legende"/></w:pPr><w:r><w:rPr><w:b/><w:bCs/><w:color w:val="%s"/>'
            '<w:sz w:val="18"/><w:szCs w:val="18"/></w:rPr><w:t xml:space="preserve">%s %d. </w:t></w:r>%s</w:p>'
            % (NAVY, L['table'], num, inline(text, size=18, color=GREY)))


FIGURES = []  # (relationship id, png path) of the figures of the document being built


def figure(path, num, text):
    """A centred PNG as wide as the text, then its caption."""
    w, h = struct.unpack('>II', open(path, 'rb').read()[16:24])
    rid = 'rIdFig%d' % num
    FIGURES.append((rid, path))
    cx = PAGE_W * 635
    cy = cx * h // w
    pic = ('<w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0"><wp:extent cx="%d" cy="%d"/>'
           '<wp:effectExtent t="0" r="0" b="0" l="0"/><wp:docPr id="%d" name="figure%d" descr="%s"/>'
           '<wp:cNvGraphicFramePr><a:graphicFrameLocks xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" noChangeAspect="1"/>'
           '</wp:cNvGraphicFramePr><a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">'
           '<a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">'
           '<pic:pic xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture"><pic:nvPicPr><pic:cNvPr id="0" name="%s.png"/>'
           '<pic:cNvPicPr/></pic:nvPicPr><pic:blipFill><a:blip r:embed="%s"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>'
           '<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="%d" cy="%d"/></a:xfrm><a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
           '</pic:spPr></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing>'
           % (cx, cy, 500 + num, num, esc(text), rid, rid, cx, cy))
    img = '<w:p><w:pPr><w:keepNext/><w:spacing w:before="120" w:after="60"/><w:jc w:val="center"/></w:pPr><w:r>%s</w:r></w:p>' % pic
    cap = ('<w:p><w:pPr><w:pStyle w:val="Legende"/><w:jc w:val="center"/></w:pPr><w:r><w:rPr><w:b/><w:bCs/><w:color w:val="%s"/>'
           '<w:sz w:val="18"/><w:szCs w:val="18"/></w:rPr><w:t xml:space="preserve">%s %d. </w:t></w:r>%s</w:p>'
           % (NAVY, L['figure'], num, inline(text, size=18, color=GREY)))
    return img + cap


def reference(num, text):
    return para(run('[%s]' % num, size=18, color=CYAN, bold=True).replace('<w:i w:val="false"/><w:iCs w:val="false"/>', '')
                + '<w:r><w:tab/></w:r>' + inline(text, size=18),
                '<w:tabs><w:tab w:val="left" w:pos="520"/></w:tabs><w:spacing w:after="80" w:line="250" w:lineRule="auto"/>'
                '<w:ind w:left="520" w:hanging="520"/>')


def heading(level, number, text, bm, first_h1):
    style = 'Heading%d' % level
    tab = {1: 560, 2: 680, 3: 800}[level]
    pbb = '<w:pageBreakBefore/>' if first_h1 else ''
    num = ''
    if number:
        num = ('<w:r><w:rPr><w:color w:val="%s"/></w:rPr><w:t xml:space="preserve">%s</w:t></w:r><w:r><w:tab/></w:r>'
               % (CYAN, number))
    t = '<w:r><w:t xml:space="preserve">%s</w:t></w:r>' % esc(text)
    bs, be = '', ''
    if bm:
        bs = '<w:bookmarkStart w:id="%d" w:name="_Toc%d"/>' % (bm, bm + 89100)
        be = '<w:bookmarkEnd w:id="%d"/>' % bm
    return ('<w:p><w:pPr><w:pStyle w:val="%s"/>%s<w:tabs><w:tab w:val="left" w:pos="%d"/></w:tabs></w:pPr>%s%s%s%s</w:p>'
            % (style, pbb, tab, bs, num, t, be))


def toc(entries, pages):
    out = ['<w:p><w:pPr><w:pageBreakBefore/><w:pBdr><w:bottom w:val="single" w:color="%s" w:sz="8" w:space="4"/>'
           '</w:pBdr><w:spacing w:after="280" w:before="0"/></w:pPr>%s</w:p>'
           % (CYAN, '<w:r><w:rPr><w:b/><w:bCs/><w:color w:val="%s"/><w:sz w:val="30"/><w:szCs w:val="30"/></w:rPr>'
                    '<w:t xml:space="preserve">%s</w:t></w:r>' % (NAVY, L['toc']))]
    for k, (level, number, text, bm) in enumerate(entries):
        anchor = '_Toc%d' % (bm + 89100)
        begin = ''
        if k == 0:
            begin = ('<w:r><w:fldChar w:fldCharType="begin"/></w:r><w:r><w:instrText xml:space="preserve"> TOC \\o "1-2" '
                     '\\h \\z \\u </w:instrText></w:r><w:r><w:fldChar w:fldCharType="separate"/></w:r>')
        num = '<w:r><w:t xml:space="preserve">%s</w:t></w:r><w:r><w:tab/></w:r>' % number if number else ''
        link = ('<w:hyperlink w:anchor="%s" w:history="1">%s<w:r><w:t xml:space="preserve">%s</w:t></w:r><w:r><w:tab/></w:r>'
                '<w:r><w:fldChar w:fldCharType="begin"/></w:r><w:r><w:instrText xml:space="preserve"> PAGEREF %s \\h </w:instrText>'
                '</w:r><w:r><w:fldChar w:fldCharType="separate"/></w:r><w:r><w:t xml:space="preserve">%s</w:t></w:r>'
                '<w:r><w:fldChar w:fldCharType="end"/></w:r></w:hyperlink>'
                % (anchor, num, esc(text), anchor, pages.get(bm, '')))
        end = '<w:r><w:fldChar w:fldCharType="end"/></w:r>' if k == len(entries) - 1 else ''
        out.append('<w:p><w:pPr><w:pStyle w:val="TOC%d"/></w:pPr>%s%s%s</w:p>' % (level, begin, link, end))
    return ''.join(out)


# ---------------------------------------------------------------- cover page

def cover(meta, resume):
    logo = ('<w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0"><wp:extent cx="590550" cy="590550"/>'
            '<wp:effectExtent t="0" r="0" b="0" l="0"/><wp:docPr id="1" name="logo" descr="Logo EPI Quantum Crypto team" title="Logo"/>'
            '<wp:cNvGraphicFramePr><a:graphicFrameLocks xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" noChangeAspect="1"/>'
            '</wp:cNvGraphicFramePr><a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">'
            '<a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">'
            '<pic:pic xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture"><pic:nvPicPr><pic:cNvPr id="0" name="" descr=""/>'
            '<pic:cNvPicPr><a:picLocks noChangeAspect="1" noChangeArrowheads="1"/></pic:cNvPicPr></pic:nvPicPr><pic:blipFill>'
            '<a:blip r:embed="rId11" cstate="none"/><a:srcRect/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>'
            '<pic:spPr bwMode="auto"><a:xfrm><a:off x="0" y="0"/><a:ext cx="590550" cy="590550"/></a:xfrm>'
            '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing>')
    nob = borders('tcBorders')
    team = ('<w:p><w:pPr><w:spacing w:after="30"/></w:pPr><w:r><w:rPr><w:b/><w:bCs/><w:color w:val="%s"/><w:sz w:val="26"/>'
            '<w:szCs w:val="26"/></w:rPr><w:t xml:space="preserve">%s</w:t></w:r></w:p><w:p><w:pPr><w:spacing w:after="0"/>'
            '</w:pPr><w:r><w:rPr><w:color w:val="%s"/><w:sz w:val="18"/><w:szCs w:val="18"/></w:rPr><w:t xml:space="preserve">%s'
            '</w:t></w:r></w:p>' % (NAVY, esc(meta['equipe']), GREY, esc(meta['equipe_devise'])))
    band = tbl([1300, 8338], [
        '<w:tr><w:tc><w:tcPr><w:tcW w:type="dxa" w:w="1300"/>%s%s<w:vAlign w:val="center"/></w:tcPr><w:p><w:r>%s</w:r></w:p></w:tc>'
        '<w:tc><w:tcPr><w:tcW w:type="dxa" w:w="8338"/>%s%s<w:vAlign w:val="center"/></w:tcPr>%s</w:tc></w:tr>'
        % (nob, tcmar(0, 0, 0, 0), logo, nob, tcmar(0, 120, 0, 0), team)], borders('tblBorders'))
    x = [band,
         '<w:p><w:pPr><w:pBdr><w:bottom w:val="single" w:color="%s" w:sz="12" w:space="1"/></w:pBdr>'
         '<w:spacing w:after="0" w:before="120"/></w:pPr></w:p>' % CYAN,
         '<w:p><w:pPr><w:spacing w:after="160" w:before="%d"/></w:pPr><w:r><w:rPr><w:b/><w:bCs/><w:color w:val="%s"/>'
         '<w:spacing w:val="30"/><w:sz w:val="19"/><w:szCs w:val="19"/></w:rPr><w:t xml:space="preserve">%s  ·  VERSION %s'
         '</w:t></w:r></w:p>' % (int(meta.get('marge_titre', 900)), CYAN, esc(meta['type'].upper()), esc(meta['version'])),
         '<w:p><w:pPr><w:spacing w:after="240" w:line="300" w:lineRule="auto"/></w:pPr><w:r><w:rPr><w:b/><w:bCs/>'
         '<w:color w:val="%s"/><w:sz w:val="50"/><w:szCs w:val="50"/></w:rPr><w:t xml:space="preserve">%s</w:t></w:r></w:p>'
         % (NAVY, esc(meta['titre'])),
         '<w:p><w:pPr><w:spacing w:after="560" w:line="300" w:lineRule="auto"/></w:pPr><w:r><w:rPr><w:color w:val="%s"/>'
         '<w:sz w:val="25"/><w:szCs w:val="25"/></w:rPr><w:t xml:space="preserve">%s</w:t></w:r></w:p>'
         % (GREY, esc(meta['sous_titre']))]
    rows = []
    for key, label in zip(['reference', 'objet', 'version_date', 'auteurs', 'diffusion', 'referentiels',
                           'documents_lies', 'remplace'], L['cover']):
        if not meta.get(key):
            continue
        rows.append('<w:tr><w:trPr><w:cantSplit/></w:trPr><w:tc><w:tcPr><w:tcW w:type="dxa" w:w="2000"/>%s</w:tcPr>'
                    '<w:p>%s</w:p></w:tc><w:tc><w:tcPr><w:tcW w:type="dxa" w:w="7638"/>%s</w:tcPr><w:p>%s</w:p></w:tc></w:tr>'
                    % (tcmar(70, 0, 70, 110),
                       '<w:r><w:rPr><w:b/><w:bCs/><w:color w:val="%s"/><w:spacing w:val="10"/><w:sz w:val="16"/>'
                       '<w:szCs w:val="16"/></w:rPr><w:t xml:space="preserve">%s</w:t></w:r>' % (GREY, label),
                       tcmar(70, 110, 70, 0), inline(meta[key], size=19)))
    x.append(tbl([2000, 7638], rows, borders('tblBorders', top=(NAVY, 8), bottom=(NAVY, 8), inside_h=('D3D9DF', 4))))
    x.append('<w:p><w:pPr><w:spacing w:after="0" w:before="0" w:line="280" w:lineRule="exact"/></w:pPr></w:p>')
    ps = para(run(L['abstract'], size=18, color=CYAN, bold=True, spacing=30), '<w:spacing w:after="100"/>')
    for p in resume:
        ps += para(inline(p, size=19), '<w:spacing w:after="90" w:before="0" w:line="255" w:lineRule="auto"/><w:jc w:val="both"/>')
    if meta.get('mots_cles'):
        ps += para(run(L['keywords'], size=17, color=NAVY, bold=True) + run(meta['mots_cles'], size=17, color=GREY),
                   '<w:spacing w:after="0" w:before="60"/><w:jc w:val="both"/>')
    x.append(box([ps], (CYAN, 24), ['F0FBFE'], [(170, 220, 170, 220)]))
    return ''.join(x)


# ---------------------------------------------------------------- Markdown parsing

def parse(path):
    lines = open(path, encoding='utf8').read().split('\n')
    meta, i = {}, 0
    if lines[0].strip() == '---':
        i = 1
        while lines[i].strip() != '---':
            if ':' in lines[i] and not lines[i].startswith(' '):
                k, v = lines[i].split(':', 1)
                meta[k.strip()] = v.strip()
            i += 1
        i += 1
    blocks = []
    resume = []
    while i < len(lines):
        l = lines[i]
        s = l.strip()
        if not s or s.startswith('<!--'):
            if s.startswith('<!--'):
                while '-->' not in lines[i]:
                    i += 1
            i += 1
            continue
        if s == '::: resume':
            i += 1
            p = []
            while lines[i].strip() != ':::':
                if lines[i].strip():
                    p.append(lines[i].strip())
                elif p:
                    resume.append(' '.join(p)); p = []
                i += 1
            if p:
                resume.append(' '.join(p))
            i += 1
            continue
        m = re.match(r'::: eq (\S+)', s)
        if m:
            blocks.append(('eq', m.group(1))); i += 1; continue
        m = re.match(r'::: fig (\S+)\s+(.*?)\s*(\{#([\w:-]+)\})?$', s)
        if m:
            blocks.append(('fig', m.group(1), m.group(2), m.group(4))); i += 1; continue
        if s == '\\newpage':
            blocks.append(('newpage',)); i += 1; continue
        m = re.match(r'(#{1,3}) (.*?)\s*(\{([^}]*)\})?$', s)
        if m:
            attrs = m.group(4) or ''
            label = re.search(r'#(\S+)', attrs)
            blocks.append(('h', len(m.group(1)), m.group(2), '-' in attrs.split(), label.group(1) if label else None))
            i += 1; continue
        if s.startswith('```'):
            title = s[3:].strip()
            i += 1
            code = []
            while not lines[i].strip().startswith('```'):
                code.append(lines[i].rstrip()); i += 1
            blocks.append(('code', title, code)); i += 1; continue
        m = re.match(r'Table: (.*?)\s*(\{([^}]*)\})?$', s)
        if m:
            attrs = m.group(3) or ''
            label = re.search(r'#(\S+)', attrs)
            widths = re.search(r'widths=([\d.,]+)', attrs)
            i += 1
            while not lines[i].strip():
                i += 1
            rows = []
            while i < len(lines) and lines[i].strip().startswith('|'):
                cells = [c.strip() for c in re.split(r'(?<!\\)\|', lines[i].strip())[1:-1]]
                if not all(re.fullmatch(r':?-+:?', c) for c in cells):
                    rows.append([c.replace('\\|', '|') for c in cells])
                i += 1
            blocks.append(('table', m.group(1), rows, widths.group(1) if widths else None,
                           label.group(1) if label else None))
            continue
        if s.startswith('>'):
            warn = s.startswith('>!')
            ps, p = [], []
            while i < len(lines) and lines[i].strip().startswith('>'):
                t = re.sub(r'^>!?\s?', '', lines[i].strip())
                if t:
                    p.append(t)
                elif p:
                    ps.append(' '.join(p)); p = []
                i += 1
            if p:
                ps.append(' '.join(p))
            blocks.append(('callout', ps, warn)); continue
        m = re.match(r'(\d+\.|-) ', s)
        if m:
            kind = 'ol' if m.group(1) != '-' else 'ul'
            items = []
            while i < len(lines) and lines[i].strip():
                t = lines[i].strip()
                if re.match(r'(\d+\.|-) ', t):
                    items.append(re.sub(r'^(\d+\.|-) ', '', t))
                else:
                    items[-1] += ' ' + t
                i += 1
            blocks.append((kind, items)); continue
        m = re.match(r'\[(\d+)\] (.*)', s)
        if m:
            blocks.append(('ref', m.group(1), m.group(2))); i += 1; continue
        p = []
        while i < len(lines) and lines[i].strip() and not re.match(r'(#{1,3} |```|Table: |>|::: |\\newpage)', lines[i].strip()):
            p.append(lines[i].strip()); i += 1
        blocks.append(('p', ' '.join(p)))
    return meta, resume, blocks


# ---------------------------------------------------------------- assembly

def render(meta, resume, blocks, pages):
    # first pass: numbers of headings, tables and labels
    labels, entries = {}, []
    h1 = h2 = tabn = fign = 0
    bm = 900
    numbered = []
    for b in blocks:
        if b[0] == 'h':
            level, text, unnum, label = b[1:]
            number = ''
            if level == 1 and not unnum:
                h1 += 1; h2 = 0; number = str(h1)
            elif level == 2 and not unnum:
                h2 += 1; number = '%d.%d' % (h1, h2)
            mark = None
            if level <= 2:
                mark = bm; bm += 1
                entries.append((level, number, text, mark))
            numbered.append((number, mark))
            if label:
                labels[label] = number
        elif b[0] == 'table':
            tabn += 1
            if b[4]:
                labels[b[4]] = str(tabn)
        elif b[0] == 'fig':
            fign += 1
            if b[3]:
                labels[b[3]] = str(fign)

    def refs(t):
        def sub(m):
            if m.group(1) not in labels:
                sys.exit('unknown label: %s' % m.group(1))
            return labels[m.group(1)]
        return re.sub(r'\{@([\w:-]+)\}', sub, t)

    body = [cover(meta, [refs(p) for p in resume]), toc(entries, pages)]
    nums = iter(numbered)
    tabn, eqn, fign, first, list_id = 0, 0, 0, True, 100
    FIGURES.clear()
    for b in blocks:
        k = b[0]
        if k == 'h':
            number, mark = next(nums)
            body.append(heading(b[1], number, b[2], mark, first and b[1] == 1))
            if b[1] == 1:
                first = False
        elif k == 'p':
            body.append(para(inline(refs(b[1]))))
        elif k in ('ol', 'ul'):
            list_id += 1
            numbering_ids.append((list_id, 2 if k == 'ol' else 1))
            for it in b[1]:
                body.append(para(inline(refs(it)),
                                 '<w:pStyle w:val="ListParagraph"/><w:numPr><w:ilvl w:val="0"/><w:numId w:val="%d"/></w:numPr>'
                                 '<w:spacing w:after="70"/><w:jc w:val="both"/>' % list_id))
        elif k == 'table':
            tabn += 1
            body.append(caption(tabn, refs(b[1])))
            body.append(table([[refs(c) for c in r] for r in b[2]], b[3]))
        elif k == 'code':
            body.append(codeblock(b[1], b[2]))
        elif k == 'callout':
            body.append(callout([refs(p) for p in b[1]], b[2]))
        elif k == 'ref':
            body.append(reference(b[1], refs(b[2])))
        elif k == 'eq':
            eqdir = os.path.join(CHARTE, 'equations', 'en') if meta.get('langue') == 'en' else os.path.join(CHARTE, 'equations')
            x = open(os.path.join(eqdir, b[1] + '.xml'), encoding='utf8').read()

            def renum(m):
                nonlocal eqn
                eqn += 1
                return '>(%d)</w:t>' % eqn
            body.append(re.sub(r'>\((\d+)\)</w:t>', renum, x))
        elif k == 'fig':
            fign += 1
            figdir = os.path.join(CHARTE, 'figures', 'en') if meta.get('langue') == 'en' else os.path.join(CHARTE, 'figures')
            body.append(figure(os.path.join(figdir, b[1] + '.png'), fign, refs(b[2])))
        elif k == 'newpage':
            body.append('<w:p><w:r><w:br w:type="page"/></w:r></w:p>')
    sect = ('<w:sectPr><w:headerReference w:type="default" r:id="rId7"/><w:headerReference w:type="first" r:id="rId8"/>'
            '<w:footerReference w:type="default" r:id="rId9"/><w:footerReference w:type="first" r:id="rId10"/>'
            '<w:pgSz w:w="11906" w:h="16838" w:orient="portrait"/><w:pgMar w:top="1250" w:right="1134" w:bottom="1150" '
            'w:left="1134" w:header="560" w:footer="520" w:gutter="0"/><w:pgNumType/><w:titlePg/><w:docGrid w:linePitch="360"/></w:sectPr>')
    head = open(os.path.join(CHARTE, 'document_head.xml'), encoding='utf8').read()
    return head + '<w:body>' + ''.join(body) + sect + '</w:body></w:document>', entries


numbering_ids = []


def numbering_xml():
    x = open(os.path.join(CHARTE, 'template', 'word', 'numbering.xml'), encoding='utf8').read()
    x = re.sub(r'<w:num w:numId=.*?</w:num>', '', x, flags=re.S)
    nums = ''.join('<w:num w:numId="%d"><w:abstractNumId w:val="%d"/><w:lvlOverride w:ilvl="0"><w:startOverride w:val="1"/>'
                   '</w:lvlOverride></w:num>' % n for n in numbering_ids)
    return x.replace('</w:numbering>', nums + '</w:numbering>')


def write_docx(meta, document, out):
    tpl = os.path.join(CHARTE, 'template')
    now = datetime.datetime.now(datetime.timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ')
    label = '%s v%s' % (meta['type'], meta['version'])
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        for root, _, files in os.walk(tpl):
            for f in sorted(files):
                p = os.path.join(root, f)
                arc = os.path.relpath(p, tpl)
                data = open(p, 'rb').read()
                if arc == os.path.join('word', 'header1.xml'):
                    x = data.decode('utf8')
                    x = re.sub(r'(  ·  )[^<]*(</w:t>)', r'\g<1>%s\g<2>' % esc(meta['titre_court']), x, count=1)
                    x = re.sub(r'(<w:tab/><w:t xml:space="preserve">)[^<]*', r'\g<1>%s' % esc(label), x)
                    data = x.encode('utf8')
                elif arc == os.path.join('word', 'footer1.xml'):
                    x = data.decode('utf8').replace('Note technique interne — diffusion restreinte', esc(meta['pied']))
                    data = x.encode('utf8')
                elif arc == os.path.join('word', '_rels', 'document.xml.rels'):
                    x = data.decode('utf8')
                    rels = ''.join('<Relationship Id="%s" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
                                   'relationships/image" Target="media/%s.png"/>' % (rid, rid) for rid, _ in FIGURES)
                    data = x.replace('</Relationships>', rels + '</Relationships>').encode('utf8')
                elif arc == os.path.join('word', 'numbering.xml'):
                    data = numbering_xml().encode('utf8')
                elif arc == os.path.join('docProps', 'core.xml'):
                    x = data.decode('utf8')
                    for tag, val in [('dc:title', meta['titre'] + ' — v' + meta['version']),
                                     ('dc:subject', 'libsp80090b — %s' % label.lower()),
                                     ('cp:keywords', meta.get('mots_cles', '').replace(' ;', ';')),
                                     ('dc:description', '%s — %s' % (meta['reference'], label))]:
                        x = re.sub(r'<%s>.*?</%s>' % (tag, tag), '<%s>%s</%s>' % (tag, esc(val), tag), x)
                    x = re.sub(r'(<dcterms:(created|modified)[^>]*>)[^<]*', r'\g<1>' + now, x)
                    data = x.encode('utf8')
                z.writestr(arc, data)
        z.writestr('word/document.xml', document)
        for rid, path in FIGURES:
            z.write(path, 'word/media/%s.png' % rid)


def to_pdf(docx, outdir):
    subprocess.run(['libreoffice', '--headless', '--convert-to',
                    'pdf:writer_pdf_Export:{"UseTaggedPDF":{"type":"boolean","value":"true"}}',
                    '--outdir', outdir, docx], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return os.path.join(outdir, os.path.splitext(os.path.basename(docx))[0] + '.pdf')


def page_numbers(pdf, entries):
    txt = subprocess.run(['pdftotext', '-layout', pdf, '-'], capture_output=True, text=True, check=True).stdout
    pages = txt.split('\f')
    found, start = {}, 1  # the cover page holds no heading
    for level, number, text, bm in entries:
        pat = re.compile(r'^\s*%s\s*%s' % (re.escape(number), re.escape(text[:30])))
        for p in range(start, len(pages)):
            # a table-of-contents line ends with leader dots: skip it
            if any(pat.match(l) and '...' not in l for l in pages[p].split('\n')):
                found[bm] = str(p + 1)
                start = p
                break
    return found


def build(src, want_pdf):
    global L
    meta, resume, blocks = parse(src)
    lang = meta.get('langue', 'fr')
    if lang not in LABELS:
        sys.exit('%s: unknown language %s' % (src, lang))
    L = LABELS[lang]
    meta.setdefault('equipe', 'EPI Quantum Crypto team')
    meta.setdefault('equipe_devise', L['motto'])
    name = '%s_libsp80090b_v%s_%s' % (meta['reference'], meta['version'], lang)
    pub = os.path.join(PUB, lang)                     # one directory per language
    os.makedirs(pub, exist_ok=True)
    out = os.path.join(pub, name + '.docx')
    numbering_ids.clear()
    document, entries = render(meta, resume, blocks, {})
    write_docx(meta, document, out)
    if want_pdf:
        tmp = tempfile.mkdtemp(dir=os.path.join(DOCS, 'build'))
        try:
            pages = page_numbers(to_pdf(out, tmp), entries)
            missing = [e[2] for e in entries if e[3] not in pages]
            if missing:
                print('  warning: page not found for', missing)
            numbering_ids.clear()
            document, _ = render(meta, resume, blocks, pages)
            write_docx(meta, document, out)
            pdf = to_pdf(out, tmp)
            shutil.move(pdf, os.path.join(pub, name + '.pdf'))
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
    print('%s -> docs/pub/%s/%s.{docx%s}' % (os.path.basename(src), lang, name, ',pdf' if want_pdf else ''))


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    want_pdf = '--docx' not in sys.argv
    # only the final sources are published: NT-xx_fr.md and NT-xx_en.md; drafts are skipped
    srcs, drafts = [], []
    for lang in sorted(LABELS):
        d = os.path.join(PUB, lang)
        for f in sorted(os.listdir(d)) if os.path.isdir(d) else []:
            if f.endswith('.md'):
                (srcs if re.fullmatch(r'NT-\d\d_%s\.md' % lang, f) else drafts).append(os.path.join(d, f))
    for f in drafts:
        print('skipped (not a final source): %s' % os.path.relpath(f, DOCS))
    if args:
        srcs = [f for f in srcs if any(os.path.basename(f).startswith(a) for a in args)]
    if not srcs:
        sys.exit('no source found in docs/pub/fr/ or docs/pub/en/')
    for f in srcs:
        build(f, want_pdf)


if __name__ == '__main__':
    main()
