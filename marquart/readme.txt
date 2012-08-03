rev 2: assignment 0 (hello world)
rev 26: assignment 1 bzw. 1b (terrain-generator)
rev 26: assignment 2 (texture- und ptf-generator)
rev 28: assignment 2 mit etwas sch�nerem Code, aber schon mit Vorbereitung zu Assignment 3
rev 29: assignment 3
rev 35: assignment 4
rev 45: assignment 5
rev 51: assignment 6
rev 54: assignment 7 (completely uses resource-builder)
rev 64: assignment 8 + Bonus A
rev 66: assignment 9 + Bonus B (J: left cannon, L: right cannon -> see game.cfg)
rev 78: assignment 10 + Bonus C

---------------------------------------
Kommondozeilen-Argumente f�r TerrainGenerator.exe:
-r <Aufl�sung> 
Optional:
-p <Pfad f�r Ausgabe (Standard: terrain_height.raw)>
-v <Startwert f�r Displacement (Standard: Zuf�llig zwischen 0 - 2^16-1)> 
-d <Startwert f�r Displacement-Faktor pro Iteration (Standard: 0.6)>
-e Lineare Verminderung des Displacements (Standard: Exponentiell)
-f <Filter-Iterationen>.<Filter-Gr��e (1-9)> (Standard: aus)

---------------------------------------
Fragen zu Assignment 3:
- Der Vertex-Buffer muss die einzelnen Vertices in der Reihenfolge der Dreieckspunkte beinhalten.
  Einzelne Vertices kommen somit auch mehrfach vor.
- Per DXUTFindDXSDKMediaFileCch(..) wird nach der Texturdatei anhand des �bergebenen Pfades gesucht.
  Wenn diese gefunden wird, wird aus dieser per D3DX11CreateShaderResourceViewFromFile(..) eine Shader-Resource-View
  erstellt, die dann letztendlich per g_DiffuseEV->SetResource(..) an den Shader �bergeben werden kann, womit diese
  dann angezeigt wird.

---------------------------------------
Fragen zu Assignment 4:
- Vektor gedreht um die x-Achse, also kommt das Licht von rechts unten
- 4. Komponente des Vektors wird nicht gebraucht und w�rde die Normalisierung beeinflussen, was zu falschen (zu kleinen)
  Werten der anderen Komponenten f�hren w�rde; bei der sp�teren Lichtberechung h�tten die xyz-Komponenten nicht mehr die
  L�nge eins, was evtl. zu Fehlern f�hrt