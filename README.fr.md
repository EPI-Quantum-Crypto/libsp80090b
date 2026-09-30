# libsp80090b — évaluation d'entropie NIST SP 800-90B en bibliothèque C

*English version: [README.md](README.md)*

Portage en C99 de l'outil de référence du NIST
[SP800-90B_EntropyAssessment](https://github.com/usnistgov/SP800-90B_EntropyAssessment) 1.1.8,
sous forme de bibliothèque qui évalue un **tampon en mémoire** : aucun fichier à écrire,
aucun exécutable à lancer, aucune sortie texte à analyser.

Elle mesure la **min-entropie** d'une source de bruit brut, en bits par échantillon, avec
les quatre tests de NIST SP 800-90B. Cette valeur fixe le nombre d'échantillons à collecter
pour une graine de DRBG, les seuils des tests de santé RCT et APT, et la déclaration
d'entropie d'un dossier ESV.

## Documentation

Chaque document existe en français (`_fr`) et en anglais (`_en`).

| Réf. | Document | Pour qui | Fichiers |
|---|---|---|---|
| **NT-01** | Note technique : méthode, les quatre tests, bonnes pratiques, générateur hybride | Ingénieurs crypto, responsables | [`fr`](docs/pub/fr/NT-01_libsp80090b_v5.1_fr.pdf) · [`en`](docs/pub/en/NT-01_libsp80090b_v5.1_en.pdf) |
| **NT-02** | Guide d'utilisation : installation, API C, labs, code d'intégration | Développeurs, opérateurs de banc | [`fr`](docs/pub/fr/NT-02_libsp80090b_v3.1_fr.pdf) · [`en`](docs/pub/en/NT-02_libsp80090b_v3.1_en.pdf) |
| **NT-03** | Guide de bonnes pratiques : l'évaluation d'entropie en pratique à travers les 18 cas des labs, redémarrage, conditionnement, protocole de qualification | Évaluateurs, ingénieurs crypto | [`fr`](docs/pub/fr/NT-03_libsp80090b_v4.1_fr.pdf) · [`en`](docs/pub/en/NT-03_libsp80090b_v4.1_en.pdf) |

Sources, charte et règles de mise à jour : [`docs/README.fr.md`](docs/README.fr.md).
Changements : [`CHANGELOG.fr.md`](CHANGELOG.fr.md).

## Conformité

| Test (SP 800-90B) | Fonction | Face à l'outil du NIST (`make check`, NT-02) |
|---|---|---|
| 1. Hypothèse IID (§5, §6.3.1) | `sp80090b_iid_evaluate_ex()` | Toutes les valeurs déterministes identiques au bit près à `ea_iid` ; mêmes verdicts des permutations |
| 2. Estimateurs non IID (§6.3) | `sp80090b_non_iid_evaluate_ex()` | Identique à `ea_non_iid` (écart max. 7,2 × 10⁻¹⁴ avec les sorties de référence du NIST), 2 à 3 fois plus rapide |
| 3. Redémarrage (§3.1.4) | `sp80090b_restart_evaluate()` | Mêmes H_r, H_c et verdicts que `ea_restart` ; seuil simulé à ±1 près |
| 4. Conditionnement (§3.1.5) | `sp80090b_conditioning_evaluate()` | Mêmes valeurs que `ea_conditioning` à 1 ulp de `long double` près |

L'estimation statistique contrôle le modèle de la source, elle ne le remplace pas
(NT-01, « Modéliser, puis mesurer »).

## Démarrage rapide

```bash
sudo apt-get install -y build-essential libdivsufsort-dev libbz2-dev openssl gawk
make                                   # libraries, labs, test programs
./test_selftest                        # self-test against NIST values
make check                             # NIST selftest (11 reference datasets) + test_selftest
```

```c
#include "sp80090b.h"

/* buf: at least 1,000,000 RAW samples, one per byte */
sp80090b_options_t o = sp80090b_default_options();
sp80090b_result_t r;

o.bits_per_symbol = 8;
if (sp80090b_non_iid_evaluate_ex(buf, len, &o, &r) != SP80090B_OK || r.warnings)
    return -1;
printf("H_I = %.5f bits per sample\n", r.h_assessed);
```

```bash
gcc -std=c99 -O2 -fopenmp main.c -I./include libsp80090b.a \
    -ldivsufsort -ldivsufsort64 -lbz2 -lm -o my_app
```

Les quatre tests se lancent aussi avec les labs : `lab/gen_data.sh` construit 24 jeux, des
bonnes sources aux défauts typiques (saturation, dérive, rejeu, sortie de DRBG...), et
`lab/lab1.sh` à `lab/lab4.sh` leur appliquent les tests 1 à 4, une ligne par cas. Les
programmes `lab/lab_iid`, `lab_non_iid`, `lab_restart` et `lab_conditioning` reprennent la
ligne de commande des outils du NIST. Syntaxe et exemples dans NT-02, résultats et leur interprétation dans NT-03.

## Règles d'usage

1. Évaluer le **bruit brut**, jamais `/dev/urandom`, `getrandom()` ni une sortie hachée : ils obtiennent près de 8 bits quelle que soit la source.
2. Au moins **1 000 000 d'échantillons**, collectés dans les conditions d'emploi (sinon `SP80090B_WARN_FEW_SAMPLES`).
3. La voie IID seulement pour une source **revendiquée et justifiée** IID par son concepteur ; sinon la voie non IID.
4. Retenir le **minimum** : `h_assessed`, puis `min(H_r, H_c, H_I)` après le test de redémarrage, et jamais plus que la revendication de conception.
5. Dimensionner la graine avec cette valeur : ⌈384 / H⌉ échantillons bruts à l'instanciation et ⌈256 / H⌉ au réensemencement pour une force de 256 bits.
6. Pour un dossier FIPS 140-3 (ESV), les données sont évaluées par le NIST ; la bibliothèque sert au développement, à la qualification et aux campagnes de mesure.

## Arborescence

```text
SP800-90B_lib/
├── README.md, README.fr.md, CHANGELOG.md, CHANGELOG.fr.md
├── Makefile                  make, make lib, make lab, make check, make docs, make clean
├── include/sp80090b.h        public API: the only header to include
├── src/                      sp80090b_api.c and test programs
├── lab/                      gen_data.sh, lab1.sh .. lab4.sh, data/, results/, lab_* programs
├── bin/                      NIST reference datasets
├── selftest/                 copy of the NIST selftest, compared with its reference outputs
└── docs/                     NT-01, NT-02, NT-03 (fr, en): sources, style, publications
```
