

# DevTerm

Ein erster Prototyp fuer die Abfrage und Konfiguration von SCPI-Geraeten ueber
TCP oder eine serielle Verbindung.

## Funktionen

- Verbindung zu einem TCP-Host (Standardport 23) oder einem COM-Port
- SCPI-Befehle senden und eingehende Zeilen im Protokoll verfolgen
- Numerische Einzelantworten in einem Live-Plot anzeigen
- Kommunikation in einem Hintergrund-Thread, damit die Oberflaeche bedienbar bleibt
- Helles, Spectrum-inspiriertes ImGui-Theme; der Plot uebernimmt die UI-Farben
- Weisse Eingabefelder, Combobox weißer Hintergrund

Serielle Verbindungen verwenden 8 Datenbits, keine Paritaet und ein Stopbit.

## Bauen (Windows)

Voraussetzungen: clang++ aus LLVM-MinGW, CMake, Ninja und initialisierte Git-Submodule.

```powershell
git submodule update --init --recursive
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++
cmake --build build
```

Die Anwendung liegt anschliessend unter `build/DevTerm.exe`.
Das Spectrum-Theme verwendet die ImGui-Standard-Widgets und die Windows-Schrift
Segoe UI. Falls sie nicht geladen werden kann, meldet DevTerm dies und verwendet
den eingebetteten ImGui-Standardfont. Eine abweichend konfigurierte Windows-
Systemschrift wird derzeit nicht automatisch uebernommen.

## Debuggen in VS Code

Die empfohlenen Extensions **LLDB DAP** und **clangd** installieren. LLVM-MinGW
(`clang++`, `clangd` und `lldb-dap`), CMake und Ninja muessen im PATH von VS Code
verfuegbar sein. Mit F5 startet `DevTerm: Debug`: Die Aufgabe konfiguriert und
baut automatisch `build\debug` mit Debug-Symbolen und erzeugt dort
`compile_commands.json`. Fuer clangd kann der erste Configure-Schritt auch mit
dem Standard-Build-Task (Strg+Umschalt+B) gestartet werden.

Die C/C++-Extension kann parallel fuer ihre Debug- und Projektfunktionen
installiert sein. Ihr IntelliSense ist standardmaessig abgeschaltet, um doppelte
Diagnosen von clangd zu vermeiden. Wer stattdessen C/C++-IntelliSense verwenden
moechte, deaktiviert clangd und setzt `C_Cpp.intelliSenseEngine` in den
Arbeitsbereichseinstellungen auf `default`. Die vorhandene
`c_cpp_properties.json` verwendet dieselbe Compile-Datenbank.

## SCPI-TCP-Simulator

Der Python-Simulator benoetigt nur die Python-Standardbibliothek und startet
standardmaessig auf `127.0.0.1:23`:

```powershell
python tools\scpi_server.py
```

Bei Bedarf koennen Host und Port angepasst werden, z. B.:

```powershell
python tools\scpi_server.py --host 127.0.0.1 --port 5025 --echo
```

In DevTerm denselben Host und Port einstellen und verbinden. Der Simulator
akzeptiert je eine ASCII-Zeile mit LF oder CRLF pro Abfrage, unabhaengig von
Gross-/Kleinschreibung. Der Syntax ist `*BEFEHL ?` (das Leerzeichen vor `?`
ist optional); Antworten enden mit LF. Mit `--echo` sendet der Server vor
jeder Antwort die unveraenderte Anfrage als eigene Zeile.

| Anfrage | Simulierte Antwort |
| --- | --- |
| `*DEV ?` | `:DEV DevTerm SCPI Simulator` |
| `*VER ?` | `:VER 10.234` |
| `*SER ?` | `:SER SIM000001` |
| `*MEAS:CURR ?` | `:MEAS:CURR <Wert>` (A) |
| `*MEAS:VOLT ?` | `:MEAS:VOLT <Wert>` (V) |
| `*MEAS:POW ?` | `:MEAS:POW <Wert>` (W) |

Die Messwerte sind zeitabhaengige Sinus-Testwerte mit unterschiedlichen
Amplituden; Leistung wird nicht aus Strom und Spannung berechnet. Unbekannte
Befehle liefern `:ERROR(0x2004): Unknown SCPI-Kommand.`. Der Simulator ist fuer lokale Tests gedacht und
bietet keine Authentifizierung; nur bei Bedarf an andere Adressen binden.
Beenden mit Strg+C. Tests ausfuehren mit:

```powershell
python -m unittest discover -s tests -v
ctest --test-dir build --output-on-failure
```

Fuer den C++-Kommunikationstest zuerst den Simulator auf Port 5025 starten,
anschliessend `build\DevTermThemeTest.exe 5025` ausfuehren. Der Test sendet
`*VER ?` und `*MEAS:VOLT ?` und prueft die Antworten des Simulators.