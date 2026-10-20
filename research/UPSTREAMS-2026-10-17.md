# BattleHeart: Quellen zur Beschleunigung des nativen SA2B-Ports

Prüfdatum: 17.10.2026. Geltungsbereich: ausschließlich Sonic Adventure 2: Battle
für GameCube als eigenständige native PS5-App. Dies ist eine Quellenprüfung,
kein integrierter oder auf der PS5 getesteter Port.

## Wichtigster neuer Fund: zusätzliche GameCube-Rekonstruktionen

[nlopez99/SA2B](https://github.com/nlopez99/SA2B) hat neben seinem zurückliegenden
Hauptbranch neun weitere Entwicklungsbranches. Der letzte,
[`pr/9-path-dust-dynamite`](https://github.com/nlopez99/SA2B/tree/pr/9-path-dust-dynamite),
steht auf `4d873648298ed3fe82f387ad5b0521d568bdeead`.

Der Vergleich mit roeming/SA2B `874a480379260dce01e8988d6ab13c0deddcee4c`
ergab 111 dort nicht enthaltene Commits und 71 neu hinzugefügte C-Quelldateien.
Das ist eine Git- und Quelltextbeobachtung, kein unabhängig bestätigtes
Matching-Ergebnis, Vollständigkeitsprozentsatz oder Spielbarkeitstest.

| Branch | Zusätzliche C-Dateien gegenüber gemeinsamem Stand |
| --- | ---: |
| `pr/1-ring-objects` | 7 |
| `pr/2-particles-1` | 15 |
| `pr/3-item-box-effects` | 23 |
| `pr/4-enemies-1` | 33 |
| `pr/5-switch-and-ai` | 39 |
| `pr/6-stage-objects-1` | 49 |
| `pr/7-stage-objects-2` | 56 |
| `pr/8-knuckles-objects` | 65 |
| `pr/9-path-dust-dynamite` | 71 |

Die Werte sind kumulativ und dürfen nicht addiert werden. Im letzten Branch
entfallen die neuen Dateien auf 40 Objekt-, 16 Effekt-, 10 Gegnerdateien sowie
fünf weitere Dateien. Stichproben für Geschosse, Item-Boxen und Zielringe
enthalten Funktionsimplementierungen. Ob sie korrekt sind, wurde hier nicht
durch einen Build gegen ein Original verifiziert.

Der Branch konfiguriert weiterhin ausschließlich US `GSNE8P`. PAL `GSNP8P`
braucht eine eigene Zuordnung von Adressen, Layouts, Revisionen und Modulen.
Für unseren Compiler helfen Funktionsgrenzen, Strukturen und Verhaltenskenntnis;
rekonstruierte Spielfunktionen werden nicht in unser öffentliches Repository kopiert.
Die vorhandene CC0-Projektdatei ist keine Freigabe fremder Spielrechte.

Alle acht öffentlichen Hauptbranches der roeming-Forks wurden verglichen:
keiner enthielt zusätzliche Commits gegenüber roeming/main. Erst die separat
gelesenen Branchlisten zeigten die oben dokumentierte zusätzliche Arbeit.
Außerdem existiert [PR 8](https://github.com/roeming/SA2B/pull/8) aus
thescoocher/SA2B, Branch `wood-container`, Revision
`db0a526fb76d39cb303405837337637a0b2b39fc`. Er betrifft weitere Rekonstruktion
des Holzcontainers. Die Matching-Angaben im PR sind Angaben des Autors.

## Weitere tatsächliche Decompilation

[Exant64/sa2dc](https://github.com/Exant64/sa2dc) rekonstruiert die US-
Dreamcast-Fassung und beschreibt Builds von Hauptprogramm, Chao und Chao-Rennen.
Das kann Hinweise auf Spiellogik und Chao-Strukturen liefern. Dreamcast-
Maschinencode, Speicheradressen und Grafikaufrufe passen nicht unmittelbar
zum GameCube. Eine klare Projektlizenz wurde bei dieser Sichtung nicht
gefunden; keine Quellenübernahme. Die im README verlangten proprietären
Dreamcast-SDKs wurden weder beschafft noch verwendet.

## Werkzeuge und native Umsetzungen

| Quelle | Konkreter Nutzen | Grenze / Lizenzstand |
| --- | --- | --- |
| [DolRecomp](https://github.com/ExpansionPak/DolRecomp) | Bereits bestehende statische Übersetzung von DOL und REL; REL-Imports und Funktionsersatzschnittstellen | GPL-3.0; CPU-Werkzeug, keine vollständige PS5-Runtime |
| [Aurora](https://github.com/encounter/aurora) | Source-Level-Anbindung für GX, Controller, Disc und Speicherkarte; Grafik über WebGPU/Dawn mit Vulkan-Backend | MIT für Aurora, weitere Lizenzen der Abhängigkeiten; ABI-Brücke und PS5-Anpassung noch zu entwickeln |
| [SA3D.Modeling](https://github.com/X-Hax/SA3D.Modeling) | GameCube-Meshes sowie Modell- und Animationsformate von SA2B | GPL-3.0-Projektdatei geprüft; C# eignet sich zunächst für PC-Konvertierung und Prüfwerkzeuge |
| [SA3D.Archival](https://github.com/X-Hax/SA3D.Archival) | PRS, PAK und GameCube-Texturarchive GVM/GVR/GVP | GPL-3.0-Projektdatei geprüft; keine Spiellogik |
| [SA3D.Texturing](https://github.com/X-Hax/SA3D.Texturing) | Texturverarbeitung als Ergänzung der Asset-Werkzeuge | GPL-3.0-Projektdatei geprüft |
| [SA3D.SA2Event](https://github.com/X-Hax/SA3D.SA2Event) | Lesen und Verarbeiten von SA2-Ereignisdaten | GPL-3.0-Projektdatei laut GitHub; genaue GC-Abdeckung noch prüfen |
| [SA Tools](https://github.com/X-Hax/sa_tools) | Viele etablierte Dateiformate und Extraktionskonfigurationen der Adventure-Spiele | Keine einheitliche Root-Lizenz von GitHub erkannt; Einzelkomponenten vor Übernahme prüfen |
| [SA2 Mod Loader](https://github.com/X-Hax/sa2-mod-loader) | Umfangreiche dokumentierte Spielstrukturen und benannte PC-Funktionen | Windows-/PC-Adressen und ABI; keine einheitliche Root-Lizenz erkannt, keine pauschale Quellenübernahme |
| [SA2 Render Fix](https://github.com/shaddatic/sa2b-render-fix) | Dokumentierte Grafikprobleme, Material-, Transparenz- und Darstellungsverhalten | PC-Mod, kein GC-Renderer; Lizenzstatus ungeklärt, zunächst Problembeschreibungen als Referenz |
| [OpenSA2](https://github.com/kittycache9/OpenSA2) | Neu gefundene native Umsetzung mit eigenem Grafik-, Audio-, Eingabe- und Systemruntime sowie Vergleichstests | GPL-3.0-only laut README; Grundlage ist die PC-Fassung von 2012, öffentliche Bootstrap-Kette unvollständig |
| [sa2-nx](https://github.com/ChanseyIsTheBest/sa2-nx) | Native SA2-Vulkan-Umsetzung, Controller- und Sprachauswahl als zusätzliche Referenz | Xbox-360-XEX/Xenos als Grundlage; braucht Xbox-Dateien, GPL-3.0/BSD/MIT je Verzeichnis |

OpenSA2 meldet selbst getestetes Gameplay, aber unvollständige Levelabdeckung.
Diese Recherche hat das nicht nachgetestet. Laut README enthält das öffentliche
Repository keine übersetzten Spielquellen und baut allein noch kein spielbares
Spiel. Weder PC- noch Xbox-Port akzeptieren unsere GameCube-ISO unmittelbar.
Aus deren Veröffentlichungspraxis wird keine rechtliche Freigabe für uns abgeleitet.

## Priorität für unsere bestehende GameCube-Vorgabe

1. Die zusätzlichen GC-Funktions- und Modulinformationen aus nlopez99/SA2B
   gegen die eigene PAL-Fassung prüfen und als private Analysegrundlage nutzen.
2. DolRecomp als vorhandenen statischen Compiler untersuchen, einschließlich
   der REL-Module. Kein eigener Neuaufbau aus dem früheren Einzelbranch-Beispiel.
3. Die Schnittstelle zwischen übersetztem Spielcode und nativen
   Grafik-/Plattformfunktionen festlegen. Aurora ist ein Kandidat; das
   Zusammenstecken mit DolRecomp ist nicht bereits implementiert oder bewiesen.
4. SA3D-PC-Werkzeuge für lokale Formatprüfung und Asset-Konvertierung nutzen,
   statt diese Formate vollständig neu zu erschließen.
5. OpenSA2, sa2-nx und PC-Moddingwissen gezielt für verbleibende Probleme lesen.

RecompCore und ModernGekko-basierte Emulator-Runtimes werden nicht zur
Laufzeitgrundlage dieses Projekts. Es gibt keinen Interpreter- oder JIT-Fallback
und kein generisches Spielauswahl-Frontend. Nicht unterstützte Übersetzung
muss sichtbar scheitern; ein plausibler Stub gilt nicht als funktionierende Lösung.

## Veröffentlichungsgrenze und Prüfstatus

Die mitgeprüften Lizenztexte betreffen die jeweiligen eigenen Komponenten.
GPL-Abhängigkeiten verlangen passende Lizenz- und Quelltextbereitstellung bei
Weitergabe; ihre Lizenz ersetzt keine Rechte am ursprünglichen Spiel.
Unklare Lizenzen werden nicht als Erlaubnis behandelt.

Öffentliche Ergebnisse dieser Recherche sind nur diese eigene Beschreibung,
Links und Revisionsmetadaten. Keine ISO, DOL, REL, Modelle, Texturen, übersetzten
Spielfunktionen oder originalen SDK-Dateien wurden in diesen Projektordner
übernommen. Der isolierte Fork-Recherchecache liegt außerhalb des Repositories.
Es wurden keine Release-Dateien veröffentlicht, PS5-Installationen geändert
oder Dritte angeschrieben.

Die geprüften Stände stehen in `upstreams-2026-10-17.json`. Zusätzlich wurden
README und Lizenz von Aurora, die Formatbeschreibung von SA3D.Modeling und
SA3D.Archival sowie die Grenzen von OpenSA2 direkt an den Quellen gelesen.
