# robot
Detta är koden för projektgrupp 8. Målet var att bygga en fungerande robot som kunde plocka upp bollar och kasta
över dem över en vajer.


# struktur
Allting kommer finnas under main, där all kod finns som ett långt script. Programmet kommer vara uppdelad i metoder 
för att kontakta dem olika komponenterna som sitter fast på arduinon som sensorer och motorer. Sen i loopen finns
det olika states som roboten kan befinna sig i, i APP_PRODUCE produceras roboten och denna startas som den ska
i RUN kör den just nu bara fram och ifall den ser en vägg inom 50 cm kommer den svänga, och throw stängs motorn av
och servomotorn kör sitt


# I mappen trail finns alla testprogram

# I sketch finns en betaversion av koden

# För att kunna använda koden måste man ladda ner https://github.com/Seeed-Studio/Grove_6Axis_Accelerometer_And_Compass_v2 till library