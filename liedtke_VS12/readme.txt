<!____-----Steuerung:------____!>
Shooting: Space & RightMouseButton
Menü: F11
Bewegen: erst F11 dann m
<!____-----°°°°°°°°°-------____!>

HA 1b: Revision 21

HA 2: Revision 29

HA 3: Revision 34
3.3 a)
Der Vertexbuffer müsste dann so verändert werden, dass im Vertexbuffer für jedes Dreieck drei Vertex gespeichert wird, wobei dann dort ein/zwei Vertex doppelt gespeichert wird für Dreiecke, die sich berühren.

b)
Damit zwei .jpg Texturen genutzt werden können, muss systematisch alle vorkommen von g_Diffuse durch eine weitere 2DTexture ersetzt werden und genauso muss dem Shader eine weitere texture in die ressources geladen werden, sowie eine SRV Ressource.

HA 4: Revision 39
Fehler: Der Lichtvekor scheint nocht aus der falschen Richtung zu kommen, ich kann allerdings nicht entdecken warum nicht.

4.6.2
Ein Normalisieren mit dem Vektor 4 würde die Geometrische Länge verfälschen und kein Richtungsvektor der Länge 1 hervorbringen.


HA 5: Revision 48

HA 6: Revision 63

HA 7: Revision 70:
Das Gras wird noch falsch berechnet... aber ich glaube das spielt ja für die Bewertung keine Rolle ;)

HA 8: Revision 83

HA 9: Revision 96

HA 10: Revision 117
