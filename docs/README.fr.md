# Documentation de libsp80090b

*English version: [README.md](README.md)*

Trois publications, chacune en français et en anglais, une charte, un générateur. Chaque note a
une seule source Markdown par langue, rangée avec ses publications dans `pub/fr/` ou `pub/en/` ; le
`.docx` et le `.pdf` sont produits à partir d'elle et ne se retouchent jamais à la main.

## Les publications

| Réf. | Document | Public | Contenu | Mise à jour |
|---|---|---|---|---|
| **NT-01** | Note technique : méthode et bonnes pratiques | Ingénieurs crypto, responsables | Pourquoi mesurer, les quatre tests, décision de conformité, bonnes pratiques, générateur hybride | Rarement (norme, méthode) |
| **NT-02** | Guide d'utilisation : API et labs | Développeurs, opérateurs de banc | Installation, API C, précautions, données, syntaxe des labs, code d'intégration, vérification | À chaque changement de l'API ou des labs |
| **NT-03** | Guide de bonnes pratiques | Évaluateurs, ingénieurs crypto | Les 18 cas des labs, redémarrage et conditionnement, leur interprétation, protocole de qualification, erreurs fréquentes, limites de l'outil | Quand les résultats des labs changent |

Le `README.fr.md` (et `README.md` en anglais) de la bibliothèque est le point d'entrée : il résume
et renvoie ici. Le `CHANGELOG.fr.md` (`CHANGELOG.md`) trace les versions de la bibliothèque et des documents.

## Arborescence

```text
docs/
├── README.md        règles et génération (anglais) ; README.fr.md : ce fichier
├── charte/          gabarit docx de la note v3 (styles, en-têtes, pieds de page), logo,
│                    équations (equations/ en français, equations/en/ en anglais)
├── build/           build_docs.py : Markdown → docx (charte) → pdf
├── pub/             une note = sa source et ses publications, un répertoire par langue :
│                    fr/ et en/ : <REF>_<fr|en>.md   ← on n'écrit qu'ici
│                                 <REF>_libsp80090b_v<version>_<fr|en>.docx / .pdf
└── archives/        les deux versions précédentes de chaque note, en .pdf et en .docx
```

## Générer

```bash
make docs                                  # from SP800-90B_lib/: the six publications
python3 docs/build/build_docs.py NT-02     # one document, both languages
python3 docs/build/build_docs.py NT-02_en  # one document, one language
python3 docs/build/build_docs.py --docx    # docx only, without LibreOffice
```

Dépendances : Python 3 (bibliothèque standard seulement), LibreOffice et `pdftotext`
(poppler-utils) pour le PDF. Compter environ 2 minutes par document : LibreOffice
convertit deux fois pour remplir les numéros de page du sommaire.

## Règles de gestion

1. **Une source par document et par langue.** On modifie `pub/<langue>/<REF>_<langue>.md`, puis on régénère.
   Une retouche faite dans Word est perdue à la génération suivante. `make docs` ne publie que
   les sources finales `NT-xx_fr.md` et `NT-xx_en.md` ; tout autre fichier `.md` de `pub/`
   (brouillon, version de travail) est ignoré et n'a pas à y rester.
2. **Les deux langues ensemble.** Toute modification de `<REF>_fr.md` est reportée dans
   `<REF>_en.md` dans la même version, et inversement : même numéro de version, mêmes sections,
   mêmes étiquettes (`{#sec:…}`, `{#tab:…}`), mêmes tableaux et mêmes chiffres.
3. **Le code est en anglais.** Dans les deux langues, les blocs de code (commentaires compris)
   et les messages des programmes restent en anglais, comme le code de la bibliothèque.
4. **Chaque information à un seul endroit.**
   - La méthode et les règles de décision : NT-01.
   - Les commandes, options, paramètres et le code : NT-02.
   - Les chiffres (résultats, durées) et les défauts : NT-03.
   Les autres documents y renvoient (« voir NT-03 ») au lieu de recopier.
5. **Les chiffres viennent des fichiers de résultats.** Les tableaux de NT-03 se mettent à
   jour depuis les fichiers `.res` de `lab/results/<lab>/` et la sortie de
   `selftest/selftest`, jamais de mémoire.
6. **Version de chaque document.**
   - Numéro x.y dans l'en-tête du Markdown (`version`), qui donne le nom du fichier publié.
   - Correction mineure : y + 1. Changement de contenu ou de structure : x + 1.
   - Une ligne dans le tableau « Historique des révisions » à chaque version.
   - Le champ `remplace` indique ce que la version remplace.
   - Déplacer le `.pdf` et le `.docx` de la version précédente de `pub/` vers `archives/`, qui ne
     garde que les deux dernières versions de chaque note ; la source `.md` n'est pas archivée.
7. **NT-03 suit la bibliothèque.** À chaque version de la bibliothèque : refaire la
   vérification (NT-02, « Vérifier une installation »), mettre à jour NT-03, et passer à
   « Clos » les actions corrigées.
8. **Mention de diffusion** identique sur les trois documents d'une même langue (`diffusion`, `pied`).
9. **Relecture avant publication.** Ouvrir le PDF et vérifier la couverture, le sommaire
   et les tableaux longs.

## Syntaxe des sources

En-tête entre deux lignes `---` : `reference`, `type`, `version`, `version_date`, `langue`
(`fr` ou `en` : libellés « Tableau / Table », « Sommaire / Contents », couverture, équations), `titre`,
`titre_court` (en-tête de page), `sous_titre`, `objet`, `auteurs`, `diffusion`, `pied`
(pied de page), `referentiels`, `documents_lies`, `remplace`, `mots_cles`,
et `marge_titre` (facultatif, espace avant le titre de couverture, 900 par défaut).

| Élément | Syntaxe |
|---|---|
| Résumé de couverture | bloc entre `::: resume` et `:::` |
| Titres numérotés | `# Titre`, `## Sous-titre` ; `### ` non numéroté |
| Titre non numéroté | `# Historique des révisions {-}` |
| Étiquette de renvoi | `# Titre {#sec:nom}` ; dans le texte, `{@sec:nom}` donne « 4.2 » |
| Tableau | ligne `Table: Légende. {#tab:nom widths=20,80}` puis un tableau `\| … \|` ; `{@tab:nom}` donne son numéro |
| Gras, italique, code | `**gras**`, `*italique*`, `` `code` `` |
| Indice, exposant | `H~I~`, `2^−20^` ; `\*`, `\~`, `\^` pour le caractère lui-même |
| Liste numérotée, à puces | `1. …`, `- …` |
| Bloc de code | ```` ```Titre du bloc ```` … ```` ``` ```` |
| Encadré « à retenir » | lignes commençant par `> ` |
| Encadré « attention » | lignes commençant par `>! ` |
| Équation de la charte | `::: eq min-entropie` (fichiers de `charte/equations/`, ou `charte/equations/en/` pour `langue: en` ; numérotées automatiquement) |
| Référence bibliographique | `[1] Auteurs. *Titre*. Éditeur, année.` |
| Saut de page | `\newpage` |

## La charte

`charte/template/` est le paquet docx de la note v3.0 sans son contenu : styles (Arial,
titres bleu nuit `11203A`, accent cyan `0FB5D6`, code rouge `B03A2E`, gris `6B7684`),
en-tête et pied de page, numérotation, logo (`word/media/`, copie dans `charte/logo.png`).
Le générateur y insère le contenu, le titre court de l'en-tête, le type et la version,
et la mention de diffusion du pied de page. Pour changer la charte (logo, couleurs), modifier
`charte/` et les constantes en tête de `build/build_docs.py`, puis régénérer les six publications.
