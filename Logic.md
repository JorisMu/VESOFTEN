# Tekentaken API – Volledige Documentatie

**Versie:** 1.0  
**Auteur:** Max  
**Doel:** Deze API biedt functies om lijnen, rechthoeken, tekst en bitmaps op het scherm te tekenen, inclusief parametercontrole en foutafhandeling.

---

## Algemene informatie
- De API controleert alle parameters voordat deze naar de I/O-layer worden gestuurd.  
- Ongeldige parameters genereren een foutmelding naar de front layer.  
- Alle coördinaten zijn gebaseerd op een scherm van breedte X en hoogte Y.  
- Kleurwaarden moeten binnen het ondersteunde kleurenspectrum liggen.  

---

## Foutcodes
| Code | Nummer |
|------|-----------|
| `OK` | 0 |
| `ERROR_INVALID_COLOR` | 1 |
| `ERROR_INVALID_PARAM_THICKNESS` | 2 |
| `ERROR_INVALID_PARAM_SIZE` | 3 |
| `ERROR_INVALID_PARAM_FILLED` | 4 |
| `ERROR_INVALID_PARAM_FONTNAME` | 5 |
| `ERROR_INVALID_PARAM_FONTSIZE` | 6 |
| `ERROR_INVALID_PARAM_FONSTYLE` | 7 |
| `ERROR_INVALID_PARAM` | 8 |
| `ERROR_OUT_OF_BOUNDS` | 9 |

---

## 🖌 Teken Commando's

### `lijn`
* **Functie:** `Resultaat lijn(int x, int y, int x2, int y2, char kleur[20], int dikte)`
* **Variabelen:**
    * `x`, `y`: Startpunt.
    * `x2`, `y2`: Eindpunt.
    * `kleur`: Naam van de kleur.
    * `dikte`: Dikte in pixels.
* **Voorbeeld:** `lijn(0, 0, 50, 50, "rood", 1);`

### `rechthoek`
* **Functie:** `Resultaat rechthoek(int x_lup, int y_lup, int breedte, int hoogte, char kleur[20], int gevuld)`
* **Variabelen:**
    * `x_lup`, `y_lup`: Linker-bovenhoek positie.
    * `breedte`, `hoogte`: Afmetingen van de rechthoek.
    * `kleur`: Naam van de kleur.
    * `gevuld`: Moet `0` zijn (andere waarden geven momenteel een error).
* **Voorbeeld:** `rechthoek(10, 10, 100, 50, "blauw", 0);`

### `tekst`
* **Functie:** `Resultaat tekst(int x, int y, char kleur[20], const char *tekst, const char *fontnaam, int fontgrootte, const char *fontstijl)`
* **Variabelen:**
    * `x`, `y`: Positie op het scherm.
    * `kleur`: Naam van de kleur.
    * `tekst`: De string met de tekstinhoud.
    * `fontnaam`: "arial" of "consolas".
    * `fontgrootte`: Moet `1` of `2` zijn.
    * `fontstijl`: "normaal", "vet" of "cursief".
* **Voorbeeld:** `tekst(20, 20, "wit", "Hallo", "arial", 1, "normaal");`

### `cirkel`
* **Functie:** `Resultaat cirkel(int x, int y, int radius, char kleur[20])`
* **Variabelen:**
    * `x`, `y`: Middelpunt van de cirkel.
    * `radius`: De straal van de cirkel.
    * `kleur`: Naam van de kleur.
* **Voorbeeld:** `cirkel(150, 150, 30, "geel");`

### `figuur`
* **Functie:** `Resultaat figuur(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5, char kleur[20])`
* **Variabelen:**
    * `x1, y1` t/m `x5, y5`: Vijf afzonderlijke coördinatenpunten.
    * `kleur`: Naam van de kleur.
* **Voorbeeld:** `figuur(0,0, 10,0, 10,10, 0,10, 5,5, "groen");`

---

## 🖼 Overige Commando's

### `bitmap`
* **Functie:** `Resultaat bitmap(int nr, int x_lup, int y_lup)`
* **Variabelen:**
    * `nr`: Bitmap ID (0-3 voor pijlen, 4-5 voor smileys).
    * `x_lup`, `y_lup`: Positie op het scherm.
* **Voorbeeld:** `bitmap(5, 50, 50);`

### `clearscherm`
* **Functie:** `Resultaat clearscherm(char kleur[20])`
* **Variabele:**
    * `kleur`: De kleur waarmee het hele scherm gevuld wordt.
* **Voorbeeld:** `clearscherm("zwart");`

### `wacht`
* **Functie:** `Resultaat wacht(int msecs)`
* **Variabele:**
    * `msecs`: Aantal milliseconden om te wachten (mag niet negatief zijn).
* **Voorbeeld:** `wacht(500);`

### `herhaal`
* **Functie:** `Resultaat herhaal(int aantal, int hoevaak)`
* **Variabelen:**
    * `aantal`: Aantal voorgaande commando's om te herhalen.
    * `hoevaak`: Hoe vaak deze reeks herhaald moet worden.
* **Voorbeeld:** `herhaal(2, 10);`