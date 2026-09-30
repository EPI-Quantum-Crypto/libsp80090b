# libsp80090b documentation

*Version française : [README.fr.md](README.fr.md)*

Three publications, each in French and in English, one house style, one generator. Each note has a
single Markdown source per language, stored with its publications in `pub/fr/` or `pub/en/`; the
`.docx` and `.pdf` are generated from it and never edited by hand.

## The publications

| Ref. | Document | Audience | Content | Updated |
|---|---|---|---|---|
| **NT-01** | Technical note: method and good practice | Crypto engineers, managers | Why measure, the four tests, conformity decision, good practice, hybrid generator | Rarely (standard, method) |
| **NT-02** | User guide: API and labs | Developers, bench operators | Installation, C API, precautions, data, lab syntax, integration code, verification | At each change of the API or of the labs |
| **NT-03** | Good practice guide | Assessors, crypto engineers | The 18 lab cases, restart and conditioning results, their interpretation, qualification protocol, common mistakes, tool limitations | When the lab results change |

The library `README.md` (and `README.fr.md` in French) is the entry point: it summarises
and refers here. `CHANGELOG.md` (`CHANGELOG.fr.md`) records the versions of the library and
of the documents.

## Directory tree

```text
docs/
├── README.md        this file: rules and generation; README.fr.md: French version
├── charte/          docx template of note v3 (styles, headers, footers), logo,
│                    equations (equations/ in French, equations/en/ in English)
├── build/           build_docs.py: Markdown → docx (house style) → pdf
├── pub/             one note = its source and its publications, one directory per language:
│                    fr/ and en/: <REF>_<fr|en>.md   ← write only here
│                                 <REF>_libsp80090b_v<version>_<fr|en>.docx / .pdf
└── archives/        the two previous versions of each note, as .pdf and .docx
```

## Generating

```bash
make docs                                  # from SP800-90B_lib/: the six publications
python3 docs/build/build_docs.py NT-02     # one document, both languages
python3 docs/build/build_docs.py NT-02_en  # one document, one language
python3 docs/build/build_docs.py --docx    # docx only, without LibreOffice
```

Requirements: Python 3 (standard library only), LibreOffice and `pdftotext`
(poppler-utils) for the PDF. Allow about 2 minutes per document: LibreOffice converts
twice to fill in the page numbers of the table of contents.

## Management rules

1. **One source per document and per language.** Edit `pub/<lang>/<REF>_<lang>.md`, then regenerate.
   An edit made in Word is lost at the next generation. `make docs` only publishes the final
   sources `NT-xx_fr.md` and `NT-xx_en.md`; any other `.md` file in `pub/` (draft, working
   version) is skipped and should not stay there.
2. **Both languages together.** Any change to `<REF>_fr.md` is carried over to `<REF>_en.md`
   in the same version, and conversely: same version number, same sections, same labels
   (`{#sec:…}`, `{#tab:…}`), same tables and same figures.
3. **Code is in English.** In both languages, code blocks (comments included) and program
   messages stay in English, like the library code.
4. **Each piece of information in one place only.**
   - The method and the decision rules: NT-01.
   - Commands, options, parameters and code: NT-02.
   - Figures (results, run times) and defects: NT-03.
   The other documents refer to it ("see NT-03") instead of copying.
5. **Figures come from the result files.** The tables of NT-03 are updated from
   the `.res` files of `lab/results/<lab>/` and the output of `selftest/selftest`,
   never from memory.
6. **Version of each document.**
   - Number x.y in the Markdown header (`version`), which gives the published file name.
   - Minor correction: y + 1. Change of content or structure: x + 1.
   - One row in the "Revision history" table at each version.
   - The `remplace` field states what the version supersedes.
   - Move the `.pdf` and `.docx` of the previous version from `pub/` to `archives/`, which only
     keeps the last two versions of each note; the `.md` source is not archived.
7. **NT-03 follows the library.** At each library version: repeat the verification
   (NT-02, "Verifying an installation"), update NT-03, and set the fixed actions to "Closed".
8. **Distribution notice** identical on the three documents of a language (`diffusion`, `pied`).
9. **Review before publication.** Open the PDF and check the cover, the table of contents
   and the long tables.

## Source syntax

Header between two `---` lines: `reference`, `type`, `version`, `version_date`, `langue`
(`fr` or `en`: labels "Tableau / Table", "Sommaire / Contents", cover, equations), `titre`,
`titre_court` (page header), `sous_titre`, `objet`, `auteurs`, `diffusion`, `pied`
(page footer), `referentiels`, `documents_lies`, `remplace`, `mots_cles`,
and `marge_titre` (optional, space before the cover title, 900 by default).

| Element | Syntax |
|---|---|
| Cover abstract | block between `::: resume` and `:::` |
| Numbered headings | `# Heading`, `## Subheading`; `### ` unnumbered |
| Unnumbered heading | `# Revision history {-}` |
| Cross-reference label | `# Heading {#sec:name}`; in the text, `{@sec:name}` gives "4.2" |
| Table | line `Table: Caption. {#tab:name widths=20,80}` then a `\| … \|` table; `{@tab:name}` gives its number |
| Bold, italic, code | `**bold**`, `*italic*`, `` `code` `` |
| Subscript, superscript | `H~I~`, `2^−20^`; `\*`, `\~`, `\^` for the character itself |
| Numbered, bulleted list | `1. …`, `- …` |
| Code block | ```` ```Block title ```` … ```` ``` ```` |
| "Key point" box | lines starting with `> ` |
| "Warning" box | lines starting with `>! ` |
| House equation | `::: eq min-entropie` (files of `charte/equations/`, or `charte/equations/en/` for `langue: en`; numbered automatically) |
| Bibliographic reference | `[1] Authors. *Title*. Publisher, year.` |
| Page break | `\newpage` |

## House style

`charte/template/` is the docx package of note v3.0 without its content: styles (Arial,
navy headings `11203A`, cyan accent `0FB5D6`, red code `B03A2E`, grey `6B7684`), header
and footer, numbering, logo (`word/media/`, copy in `charte/logo.png`). The generator
inserts the content, the short title of the header, the type and version, and the
distribution notice of the footer. To change the house style (logo, colours), edit
`charte/` and the constants at the top of `build/build_docs.py`, then regenerate the six
publications.
