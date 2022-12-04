## Funktionen:

| Name                          | Beschreibung        | Imp. | Get. | Notes                                                                                                                                                            |
|-------------------------------|---------------------|------|------|------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Addition                      |                     | ja   | nein |                                                                                                                                                                  |
| Negation                      |                     | ja   | nein |                                                                                                                                                                  |
| Subtraktion                   |                     | ja   | nein |                                                                                                                                                                  |
| Shift Left                    |                     | ja   | nein |                                                                                                                                                                  |
| Get-HI                        |                     | ja   | nein | Note: Es muss nicht unbedingt die ganze bignum kopiert werden, meistens sollte es reichen einen pointer auf die 2. hälfte des original memorys zeigen zu lassen. |
| Get-Lo                        |                     | ja   | nein | s.o.                                                                                                                                                             |
| Multiplikation (Karazuba)     |                     | ja   |      |                                                                                                                                                                  |
| Multiplikation (Alternative?) |                     |      |      | zur Diskussion                                                                                                                                                   |
| Shift Right                   |                     | ja   | nein | noch anständige Test cases ausdenken                                                                                                                             |
| Division (Newton-Raphson)     |                     |      |      |                                                                                                                                                                  |
| Division (Schulmethode?)      |                     |      |      | zur Diskussion                                                                                                                                                   |
| Free                          |                     | ja   | n.N. |                                                                                                                                                                  |
| == Operation                  | bignumA ==? bignumB | ja   |      |                                                                                                                                                                  |
| Print as Hex                  |                     | ja   |      | Wenn implementiert, in main.c in main() nutzen mit lokal variable                                                                                                |
| Print as Dec                  |                     |      |      | Wenn implementiert, in main.c in main() nutzen mit lokal variable                                                                                                |
| Command-Line interpretation   | in main             | ja   | ?    | Methoden fehlen (PrintasHex, PrintasDec, etc.)  <br/>                                                                                                            |
| Subtraktion2()                | TO DELETE           | ja   | ja   | Funktioniert nicht für Zahlen ab 64 Bit, Borrow Implementation vielleicht nützlich                                                                               |
| getReciprocal(a, k)           | rechnet reciprocal  | 1/2  | nein | Gleiches Problem wie newtonDivision(),viele Fs tauchen auf nach Sub- vielleicht sogar richtig, Interpretationsfehler                                             |
| toDecimal(a)                  | To decimal          | nein | nein | Einfach durch hochste 10-er Potenz dividieren und Parsen. Wie für Nachkommastellen                                                                               |
| goldschmidtDiv()              | Division            | ja    | 1/2 | Funktioniert nur Für einzel Operationen, etwas falsch mit Speicher vom Pointer               |
| 
---
**Legende**:
Imp. = Vollständig Implementiert ; Get. = Vollständig getestet
---

## Theorie:

* Herausfinden wie Schnelle Exponentiation funktioniert

## Notizen:

* Alle Nummern sind Big Endian (Stellen kleinster relevanz zuerst)
* BigNum size beschreibt die größe in 32bit einheiten
    * Size ist immer auf 64 bit aligned -> d.h. size % 2 == 0