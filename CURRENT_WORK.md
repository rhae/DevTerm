# Aktuelle Arbeit: UI- und Fensterkomponenten

Ziel: geeignete Fenster- und UI-Muster aus `Kolosal-main/include/window` und
`Kolosal-main/include/ui` in DevTerm übernehmen. Gerätekommunikation, Protokoll-
und Messwertverhalten bleiben erhalten. DevTerm bleibt vorerst bei C++11,
Win32 und OpenGL2; Komponenten werden ohne Lambdas und Coroutinen umgesetzt.

## Pakete und Fortschritt

- ✅ **Planung und Leitplanken — 100 % erledigt:** Komponentenbaum, C++11- und
  Renderer-Grenzen sowie Prüfpunkte festgehalten.
- ✅ **Fenster- und OpenGL-Lebenszyklus — 100 % erledigt:** Win32-Fenster und
  OpenGL-Kontext in eigenständige Klassen mit RAII-Verantwortung ausgelagert;
  borderless Resize/Drag, Acrylic-Fallback, Schatten, Rundungen und relevante
  DPI-, Resize-, Aktivierungs-, Paint-, Schließ- und DWM-Nachrichten integriert.
  Eine eigene Titelzeile bietet Minimieren, Maximieren/Wiederherstellen und
  Schließen.
- ✅ **Quelldatei-Namenskonvention — 100 % erledigt:** Klassendateien ohne
  künstliches `C`-Präfix benannt; Klassennamen behalten das `C`.
- ✅ **UI-Komponenten — 100 % erledigt:** Titelzeile, Menü, Verbindungsleiste,
  Arbeitsbereich/Tabs, Befehlseingabe und Statusanzeige aus `CApplication`
  in eigene Header-/Implementierungsdateien herausgelöst.
- ✅ **Anwendungsintegration und Build — 100 % erledigt:** Komponenten in
  `CApplication` verbunden, CMake ergänzt und Debug-Windows-Build erfolgreich.
- ✅ **Theme-Hintergründe — 100 % erledigt:** Helles Spectrum-Theme auf
  `#CDD6D9`, dunkles auf `#444344` gesetzt und im Theme-Test abgesichert.
- ✅ **Abnahme — 90 % erledigt:** Debug-Windows-Build, `DevTermTheme`-Test,
  Whitespace-Prüfung und GUI-Start-/Schließ-Smoke-Test erfolgreich. Die native
  Win32-Hit-Test-Zone verwendet dieselbe Titelleistenhöhe wie die gezeichnete
  Komponente. Segoe UI Bold wird bevorzugt, Rundungen an Steuerelementen sind
  entfernt. Resize, Acrylic-Darstellung unter verschiedenen DWM-Versionen,
  Fensteraktionen sowie Verbindungsabläufe benötigen noch eine manuelle
  Prüfung.

## Implementierter Verzeichnisbaum

```text
src/
  Application.h/.cpp
  CommunicationWorker.h/.cpp
  Connection.h/.cpp
  ScpiResponse.h/.cpp
  WindowsFont.h/.cpp
  main.cpp
  window/
    Window.h/.cpp
    Win32Window.h/.cpp
    OpenGLContext.h/.cpp
  ui/
    shell/
      TitleBar.h/.cpp
      MenuBar.h/.cpp
      StatusBar.h/.cpp
    terminal/
      ConnectionBar.h/.cpp
      WorkspaceTabs.h/.cpp
      CommandInput.h/.cpp
  theme/
    spectrum.h/.cpp
```

`CApplication` koordiniert Kommunikation und UI-Aktionen; Darstellungs-
Komponenten zeichnen nur ihre Oberfläche und liefern einfache Ergebnisse
zurück. OpenGL2 und das Spectrum-Theme bleiben die bestehende Renderbasis.
Quelldateien verwenden Klassennamen ohne `C`-Präfix; die C++-Klassennamen
behalten das Präfix.
Der native Fensterrahmen ist durch borderless Resize-/Drag-Hit-Testing ersetzt;
Windows-DWM erhält Acrylic, Schatten und bevorzugte Rundungen mit einer
abgerundeten Regionsform als Fallback. Wo der Backdrop verfügbar ist, wird der
ImGui-Hintergrund transparent gezeichnet.

## Abnahme

- ✅ Neue `.cpp`-Dateien sind im DevTerm-Target in `CMakeLists.txt` eingetragen.
- ✅ Keine Lambdas oder Coroutinen hinzugefügt; C++11 bleibt aktiviert.
- ✅ Debug-Windows-Build erfolgreich.
- ✅ Whitespace-Prüfung der geänderten Dateien erfolgreich (CRLF berücksichtigt).
- ✅ `DevTermTheme`-Test erfolgreich; kontrastreichere Schrift mit Bold-Font-
  Präferenz und Fallback sowie kantige Controls/Frames gegen helle
  Rundungsartefakte.
- ✅ GUI-Start-/Schließ-Smoke-Test erfolgreich; Resize, Acrylic-Darstellung
  unter verschiedenen DWM-Versionen sowie Verbindung, Protokoll und Plot
  wurden nicht manuell durchgeklickt.
