
Name: DevTerm
Intent: Ein Programm, um mit einem Textprotokoll (SCPI) via seriell oder TCP (Port 23) Geräte abzufragen oder zu konfigurerien.

Tech stack:
- clang++ (C:\Users\hae\AppData\Local\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\llvm-mingw-20260616-ucrt-x86_64\bin\clang++.exe)
- cmake (C:\Program Files\CMake\bin\cmake.exe)
- imgui ($workspace/external/imgui)
- imgui plot ($workspace/external/imgui-plot)

Layout
======
- Menü
- Symbolleiste
- Tab unten
  - tab 1: mehrzeiliges Textfeld
  - tab 2: plot
- eingabezeile
- statuszeile

Architektur
===========
- src directory-Layout
- Connection Objekt für serial oder TCP
- Compartment thread modell
  - Main thread für UI
  - Background thread für Kommunikation
- Component based UI

Coding style
============
- C++11
- Upper CamelCase für Klassen, Funktoinsnamen, Variablennamem
  - Aussnahem kurzlebige Variablen in Schleifen (i, ii, j, n, k)
- Klassen
  - beginne mit C
  - Member starten mit m_
- 2 Leerzeicheneinrückung
- Geschweifte Klammer auf leerzeile

Coding prinzipien
=================
- Keep it simple stupid
- Dont repeat your self
- Keine fancy C++ konstrukte (coroutine, lambda) -> einfach klassisch