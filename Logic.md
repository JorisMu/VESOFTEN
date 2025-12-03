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
| Code | Betekenis |
|------|-----------|
| `OK` | Parameters zijn geldig, functie uitgevoerd |
| `ERROR_INVALID_PARAM` | Een parameter is ongeldig (bijv. buiten scherm) |
| `ERROR_OUT_OF_BOUNDS` | Coördinaten liggen buiten het scherm |
| `ERROR_INVALID_COLOR` | Kleur is niet gedefinieerd of onjuist |

---

## 1. Lijn tekenen
**Functie:**  
`lijn(x, y, x2, y2, kleur, dikte)`

**Parameters:**  
- `x, y, x2, y2` : coördinaten van begin- en eindpunt  
- `kleur` : gewenste kleur  
- `dikte` : lijndikte

**Return:**  
- `OK` als de lijn getekend kan worden  
- `ERROR_OUT_OF_BOUNDS` of `ERROR_INVALID_PARAM` bij fouten

**Beschrijving:**  
Controleert de parameters en tekent een lijn op het scherm als ze geldig zijn.

---

## 2. Rechthoek tekenen
**Functie:**  
`rechthoek(x_lup, y_lup, breedte, hoogte, kleur, gevuld)`

**Parameters:**  
- `x_lup, y_lup` : linkerbovenhoek  
- `breedte, hoogte` : afmetingen  
- `kleur` : kleur van rand of vulling  
- `gevuld` : 1 = gevuld, 0 = alleen rand

**Return:**  
- `OK` bij succes  
- `ERROR_INVALID_PARAM` of `ERROR_OUT_OF_BOUNDS` bij fouten

**Beschrijving:**  
Controleert de parameters en tekent een rechthoek als ze geldig zijn.

---

## 3. Tekst plaatsen
**Functie:**  
`tekst(x, y, kleur, tekst, fontnaam, fontgrootte, fontstijl)`

**Parameters:**  
- `x, y` : positie  
- `kleur` : tekstkleur  
- `tekst` : string die getoond wordt  
- `fontnaam` : Arial, Consolas  
- `fontgrootte` : 1 of 2  
- `fontstijl` : normaal, vet, cursief

**Return:**  
- `OK` bij succes  
- `ERROR_INVALID_PARAM` bij fouten (bijv. lettertype niet ondersteund)

**Beschrijving:**  
Controleert de parameters en toont tekst op het scherm als ze geldig zijn.

---

## 4. Bitmap tekenen
**Functie:**  
`bitmap(nr, x_lup, y_lup)`

**Parameters:**  
- `nr` : bitmap-nummer (bijv. pijl of smiley)  
- `x_lup, y_lup` : linkerbovenhoek van de bitmap

**Return:**  
- `OK` bij succes  
- `ERROR_INVALID_PARAM` als bitmap niet bestaat of coördinaten ongeldig zijn

**Beschrijving:**  
Controleert de parameters en tekent de bitmap op het scherm als ze geldig zijn.

---

## 5. Scherm wissen
**Functie:**  
`clearscherm(kleur)`

**Parameters:**  
- `kleur` : kleur waarmee het scherm wordt gevuld

**Return:**  
- `OK` bij succes  
- `ERROR_INVALID_COLOR` als de kleur ongeldig is

**Beschrijving:**  
Maakt het volledige scherm leeg en vult het met de opgegeven kleur als deze geldig is.
