<!____-----Steuerung:------____!>
Shooting: Space & RightMouseButton
Men�: F11
Bewegen: erst F11 dann m
<!____-----���������-------____!>

HA 1b: Revision 21

HA 2: Revision 29

HA 3: Revision 34
3.3 a)
Der Vertexbuffer m�sste dann so ver�ndert werden, dass im Vertexbuffer f�r jedes Dreieck drei Vertex gespeichert wird, wobei dann dort ein/zwei Vertex doppelt gespeichert wird f�r Dreiecke, die sich ber�hren.

b)
Damit zwei .jpg Texturen genutzt werden k�nnen, muss systematisch alle vorkommen von g_Diffuse durch eine weitere 2DTexture ersetzt werden und genauso muss dem Shader eine weitere texture in die ressources geladen werden, sowie eine SRV Ressource.

HA 4: Revision 39
Fehler: Der Lichtvekor scheint nocht aus der falschen Richtung zu kommen, ich kann allerdings nicht entdecken warum nicht.

4.6.2
Ein Normalisieren mit dem Vektor 4 w�rde die Geometrische L�nge verf�lschen und kein Richtungsvektor der L�nge 1 hervorbringen.


HA 5: Revision 48

HA 6: Revision 63

HA 7: Revision 70:
Das Gras wird noch falsch berechnet... aber ich glaube das spielt ja f�r die Bewertung keine Rolle ;)

HA 8: Revision 83

HA 9: Revision 96

HA 10: Revision 117
