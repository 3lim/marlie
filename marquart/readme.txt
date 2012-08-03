rev 2: assignment 0 (hello world)
rev 26: assignment 1 bzw. 1b (terrain-generator)
rev 26: assignment 2 (texture- und ptf-generator)
rev 28: assignment 2 mit etwas schönerem Code, aber schon mit Vorbereitung zu Assignment 3
rev 29: assignment 3
rev 35: assignment 4
rev 45: assignment 5
rev 51: assignment 6
rev 54: assignment 7 (completely uses resource-builder)
rev 64: assignment 8 + Bonus A
rev 66: assignment 9 + Bonus B (J: left cannon, L: right cannon -> see game.cfg)
rev 78: assignment 10 + Bonus C

---------------------------------------
Kommondozeilen-Argumente für TerrainGenerator.exe:
-r <Auflösung> 
Optional:
-p <Pfad für Ausgabe (Standard: terrain_height.raw)>
-v <Startwert für Displacement (Standard: Zufällig zwischen 0 - 2^16-1)> 
-d <Startwert für Displacement-Faktor pro Iteration (Standard: 0.6)>
-e Lineare Verminderung des Displacements (Standard: Exponentiell)
-f <Filter-Iterationen>.<Filter-Größe (1-9)> (Standard: aus)

---------------------------------------
Fragen zu Assignment 3:
- Der Vertex-Buffer muss die einzelnen Vertices in der Reihenfolge der Dreieckspunkte beinhalten.
  Einzelne Vertices kommen somit auch mehrfach vor.
- Per DXUTFindDXSDKMediaFileCch(..) wird nach der Texturdatei anhand des übergebenen Pfades gesucht.
  Wenn diese gefunden wird, wird aus dieser per D3DX11CreateShaderResourceViewFromFile(..) eine Shader-Resource-View
  erstellt, die dann letztendlich per g_DiffuseEV->SetResource(..) an den Shader übergeben werden kann, womit diese
  dann angezeigt wird.

---------------------------------------
Fragen zu Assignment 4:
- Vektor gedreht um die x-Achse, also kommt das Licht von rechts unten
- 4. Komponente des Vektors wird nicht gebraucht und würde die Normalisierung beeinflussen, was zu falschen (zu kleinen)
  Werten der anderen Komponenten führen würde; bei der späteren Lichtberechung hätten die xyz-Komponenten nicht mehr die
  Länge eins, was evtl. zu Fehlern führt