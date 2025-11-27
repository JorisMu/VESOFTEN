# Tekentaken / API Functiebeschrijving

Deze API bevat functies om lijnen, vormen, tekst en bitmaps op het scherm te tekenen.

---

## 1. Lijn tekenen
**Functie:**  
`lijn(x, y, x2, y2, kleur, dikte)`

**Beschrijving:**  
Tekent een lijn van punt **(x, y)** naar **(x2, y2)** met de opgegeven **kleur** en **dikte**.

---

## 2. Rechthoek tekenen
**Functie:**  
`rechthoek(x_lup, y_lup, breedte, hoogte, kleur, gevuld)`

**Beschrijving:**  
Tekent een rechthoek met linkerbovenhoek **(x_lup, y_lup)**.  
- `gevuld = 1` → Rechthoek is volledig gevuld in de opgegeven kleur  
- `gevuld = 0` → Alleen een rand (1 px dik)

---

## 3. Tekst plaatsen
**Functie:**  
`tekst(x, y, kleur, tekst, fontnaam, fontgrootte, fontstijl)`

**Beschrijving:**  
Tekent tekst op positie **(x, y)**.  
Ondersteunde opties:  
- **Fontnaam:** arial, consolas  
- **Fontgrootte:** 1 of 2  
- **Fontstijl:** normaal, vet, cursief  

---

## 4. Bitmap tekenen
**Functie:**  
`bitmap(nr, x_lup, y_lup)`

**Beschrijving:**  
Tekent een vooraf ingestelde bitmap op positie **(x_lup, y_lup)**.  
Beschikbare bitmaps omvatten:  
- pijlen (4 richtingen)  
- smileys (boos, blij)

---

## 5. Scherm wissen
**Functie:**  
`clearscherm(kleur)`

**Beschrijving:**  
Maakt het volledige scherm leeg en vult het met de opgegeven kleur.

---
