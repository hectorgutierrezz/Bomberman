# Estat del Projecte: Pocket Bomberman 2D (FIB - VJ 2026/27)

Aquest document detalla l'estat actual del desenvolupament del videojoc **Pocket Bomberman** desenvolupat a `2DGame/`, contrastat punt per punt amb els requeriments de la guia de la pràctica (**FIB - Videojocs Curs 2026/27 Q1: Laboratori Exercici 2D**).

---

## 1. Avaluació vs Requeriments Oficials de l'Enunciat

L'exercici s'avalua sobre 10 punts dividits en dues parts principals: **Part Bàsica (6 punts)** i **Polish (4 punts)**.

### 1.1. Part Bàsica (6 punts)

| # | Requeriment de l'Enunciat (PDF) | Estat Actual | Progrés | Detall de la Implementació a `2DGame` |
| :-: | :--- | :---: | :---: | :--- |
| **1** | **Estructura bàsica de 4 pantalles**<br>*(Menú principal, jugar, instruccions, crèdits)* | **Completat** | 100% | Les 4 pantalles principals estan implementades (`MAIN_MENU`, `PLAYING`, `INSTRUCTIONS`, `CREDITS`) més estats auxiliars (`PAUSED`, `GAME_OVER`, `WIN`). Navegació amb fletxes/WASD, selecció amb Enter/Espai, dreceres 1-4 i tecla `ESC` per pausar o tornar enrere. Menús maquetats amb sprites originals de GBC (`images/Menu/`) i `BitmapText`. |
| **2** | **Cinc nivells amb eliminació d'enemics i porta de sortida** | **En Progrés** | 60% | Creades les plantilles dels 5 nivells (`levels/level01.txt` fins a `level05.txt`). Classe [Door](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Door.h) implementada amb sprite de porta oberta/tancada i detecció de col·lisió de pas de nivell (`nextLevel()`). En superar el nivell 5 s'arriba a la pantalla de victòria (`WIN`). **Pendent:** dissenyar els mapes finals amb el tileset de GBC i condicionar l'obertura de la porta a haver eliminat tots els enemics (actualment s'obre per defecte en no haver-hi enemics encara). |
| **3** | **Moviment d'esquerra a dreta i salt de Bomberman** | **Completat** | 100% | Físiques de moviment horitzontal, gravetat i salt parabòlic operatives a [Player.cpp](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp). Substituït completament l'antic sprite de Bubble Bobble per l'spritesheet oficial de Bomberman (`bomber.png` de GBC) amb animacions d'`IDLE`, `STAND_LEFT/RIGHT`, `MOVE_LEFT/RIGHT` i `FALL`. Càmera dinàmica suau (*lerp*) que segueix el personatge. |
| **4** | **Tres enemics amb comportaments diferents**<br>*(Contacte resta vida si no té el «cor»)* | **Pendent** | 0% | El sistema de dany i invulnerabilitat temporal del jugador ja existeix a `Player::hit()`, però encara no s'ha creat la jerarquia de classes d'enemics (`Enemy`) ni els 3 patrons d'IA diferenciats (patrulla terrestre, volador/perseguidor, saltador/ràpid). |
| **5** | **Perdre les 3 vides implica perdre la partida i tornar al menú** | **Completat** | 100% | **Sistema de dos comptadors (vides generals i vida de nivell):**<br>- **Vida de nivell (Cors):** 1 vida per pantalla (augmentable només amb power-ups com cor/escut). Si té >1 vida i rep dany, absorbeix el cop i parpelleja invulnerable 2s. Si perd l'últim cor, mor en l'intent.<br>- **Vides Generals (Bomberman):** 3 intents globals. Quan es perden totes les vides del nivell o s'esgota el temps, es resta 1 vida general i es reinicia la pantalla des de l'inici (spawn, mapa i temps restablerts) **mantenint els power-ups**. Quan es perden les 3 vides generals (arriba a 0), salta el Game Over / Time Over i qualsevol tecla envia directament al menú principal (`ENTER`/`ESC`/`ESPAI`/`M`). |
| **6** | **Deixar anar bombes (alineades amb tiles) que maten enemics o Bomberman** | **Completat** | 95% | La bomba s'alinea automàticament a la quadrícula (`round(pos/16)*16`). Classe [Bomb](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Bomb.h) amb animació de mecha i detonació a ~2.2s. Classe [Explosion](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Explosion.h) en creu (4 direccions) amb col·lisió contra parets, reaccions en cadena i dany a Bomberman. **Pendent:** danyar enemics i blocs destructibles. |
| **7** | **Saltar sobre les seves bombes per arribar a zones elevades** | **Completat** | 100% | Fidels a *Pocket Bomberman*, les bombes no tenen gravetat i es mantenen fixes a l'aire on es col·loquen. Disposen de superfície superior sòlida (`isSolidOnTop`), permetent a Bomberman recolzar-s'hi i usar-les com a plataformes o escales per pujar a llocs inaccessibles. |
| **8** | **Tiles destructibles (arbustos/blocs) que poden deixar anar objectes** | **Pendent** | 0% | Falta afegir identificadors de rajoles destructibles a [TileMap](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/TileMap.h), fer que les explosions les trenquin (regenerant la malla VBO) i generar objectes (*drop*) en trencar-se. |
| **9** | **Quatre objectes (com a mínim ‘Bomb Up’ i ‘Fire Up’)** | **Pendent** | 15% | Codi base preparat (`Player::getMaxBombs()` i paràmetre de radi a `Explosion`), però falta crear les entitats i textures per als 4 objectes: `Bomb Up` (+1 bomba simultània), `Fire Up` (+1 radi d'explosió), `Cor` (protecció d'1 cop/vida) i un quart objecte (com `Patins` de velocitat o `Pedra de Poder`). |
| **10** | **Temps límit per passar-se cada nivell** | **Completat** | 100% | Temporitzador enrere de 2 minuts (120s) per nivell. Si s'esgota el temps, el joc passa a la pantalla de *Time Over* amb el sprite original de Bomberman marejat amb rellotges als ulls. |
| **11** | **Boss final al nivell 5** | **Pendent** | 0% | El nivell 5 està creat a nivell de mapa, però falta dissenyar i programar el cap final del món (comportament, vida/fases i patró d'atac basat en el joc original). |
| **12** | **Interfície gràfica (HUD) amb dades mínimes requerides** | **En Progrés** | 60% | **Implementat:** Temps restant (`TIME MM:SS`, en vermell si <= 30s), Vides restants (icones de Bomberman), Nivell actual (`1-1`, `1-2`, ...) i avís de *God Mode*. <br>**Pendent segons enunciat:** comptador d'enemics restants, nombre de bombes simultànies i mida/potència de les explosions. |
| **13** | **Tecles de truc (Cheats de correcció)** | **En Progrés** | 90% | - **`G` (God Mode):** IMPLEMENTAT a [Scene.cpp](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Scene.cpp#L644-L647) (invulnerabilitat total + missatge en pantalla).<br>- **`K` (Eliminar enemics i obrir porta):** IMPLEMENTAT per obrir la porta (`door->setOpen(true)`); pendent afegir que elimini els enemics de la llista un cop existeixin.<br>- **`1`, `2`, `3`, `4`, `5` (Saltar de nivell):** IMPLEMENTAT (càrrega directa de qualsevol dels 5 nivells).<br>*(Extres de desenvolupament: `N`/`P` per saltar de nivell, `H` per afegir +1 vida interna/cor, `B` per afegir +1 Bomb Up).* |

---

### 1.2. Polish (4 punts)

| Aspecte de Polish | Estat Actual | Progrés | Descripció i Estat |
| :--- | :---: | :---: | :--- |
| **So (Música de fons i efectes especials)** | **Pendent** | 0% | Falta incorporar el motor d'àudio (ex: llibreria lleugera com miniaudio, SDL2_mixer o irrKlang) per reproduir la música dels nivells/menús i els efectes de so (explosió, col·locar bomba, salt, agafar objectes, mal). |
| **Interactivitat i Animacions de totes les entitats** | **En Progrés** | 65% | - **Fet:** Animacions de Bomberman (`bomber.png`: moviment, salt, caiguda, parpelleig, dany `HURT` i seqüència completa de mort oficial `DIE` de 7 frames), animació de metxa de la bomba (`Bomb On`, 4 frames), animació d'explosió fluida (`Boooooom`, 6 frames), cursor animat al menú, pantalla de pausa autèntica, pantalles de Game Over / Time Over i pantalla de victòria amb cavallers celebrant.<br>- **Pendent:** Animació de trencament de blocs, animacions per als 3 enemics i el Boss, transicions suaus entre pantalles. |
| **Game feeling (Fidelitat, diversió i feedback)** | **En Progrés** | 50% | - **Fet:** Plataformes sobre bombes aèries autèntiques, càmera suau centrada en el jugador, estètica pixel-art de GBC, parpelleig en rebre dany, temporitzador que alerta en vermell quan queda poc temps.<br>- **Pendent:** Feedback d'impacte (*screenshake* a les explosions, partícules de runes, efectes sonors sincronitzats). |

---

### 1.3. Requisits del Lliurament Final (Pàgina 3 del PDF)

| Requisit de Lliurament | Especificació del PDF | Estat de Preparació |
| :--- | :--- | :--- |
| **Format del fitxer** | `NomCognoms.zip` (màxim **200 MB** en total) | Pendent en finalitzar el projecte. |
| **Fitxer `info.txt`** | Noms i cognoms dels integrants, funcionalitats implementades i instruccions del joc | Pendent de redactar abans de l'entrega. |
| **Carpeta `Binari`** | Executable i tots els recursos necessaris (imatges, nivells, llibreries/dlls) llest per executar | El projecte compila netament amb `make` a Linux; falta generar el paquet autònom per al lliurament. |
| **Carpeta `Projecte`** | Projecte compilable amb tot el codi font net | Organitzat a `2DGame/02-Bubble/`. |
| **Vídeo `demo.avi`** | Vídeo de mostra d'**1 minut**, mida màxima **100 MB** (usant OBS/FRAPS i HandBrake) | Pendent de gravar i comprimir un cop integrat el polish. |

---

## 2. Mapa dels Fitxers del Projecte (`2DGame/02-Bubble/`)

### Codi Font (`02-Bubble/`)
- [Scene.h](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Scene.h) / [Scene.cpp](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Scene.cpp):
  - Màquina d'estats completa: `MAIN_MENU`, `INSTRUCTIONS`, `CREDITS`, `PLAYING`, `PAUSED`, `GAME_OVER`, `WIN`.
  - Gestió de nivells 1 a 5 (`startLevel()`, `nextLevel()`).
  - Gestió d'entitats: jugador, porta, vector de bombes i explosions.
  - Tecles de truc: `G` (God Mode), `K` (Obrir porta / eliminar enemics), `1`..`5` (salt directe de nivell).
  - HUD complet amb vides, temps i indicador de nivell.
  - Càmera dinàmica 2D amb interpolació (*lerp*).
- [Player.h](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Player.h) / [Player.cpp](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp):
  - Model i físiques de Bomberman amb spritesheet oficial `bomber.png` (17x17 px originals escalats a 32x32).
  - Animacions: `IDLE`, `STAND_LEFT/RIGHT`, `MOVE_LEFT/RIGHT`, `FALL`.
  - Sistema de 3 vides, invulnerabilitat temporal de 2 segons amb parpelleig, i God Mode.
  - Suport per a col·lisió i recolzament sobre bombes sòlides a l'aire.
- [Door.h](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Door.h) / [Door.cpp](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Door.cpp):
  - Entitat de la porta de sortida del nivell amb estats tancada i oberta.
  - Detecció de col·lisió centrada amb Bomberman per passar al següent nivell.
- [Bomb.h](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Bomb.h) / [Bomb.cpp](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Bomb.cpp):
  - Col·locació alineada a la graella (tiles de 16x16 / 32x32).
  - Animació de 4 frames amb `Bomb On (52x56).png`.
  - Comportament de plataforma sòlida (`isSolidOnTop`) sense gravetat.
- [Explosion.h](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Explosion.h) / [Explosion.cpp](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Explosion.cpp):
  - Propagació en 4 direccions amb animació fluida de 6 frames (`Boooooom (52x56).png`).
  - Detonació en cadena de bombes properes i dany al jugador.
- [TileMap.h](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/TileMap.h) / [TileMap.cpp](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/TileMap.cpp):
  - Càrrega dels fitxers de mapa i generació de geometria OpenGL (VBO/VAO).
  - Mètodes de comprovació de col·lisions sòlides.
- [BitmapText.h](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/BitmapText.h) / [BitmapText.cpp](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/BitmapText.cpp):
  - Renderització de text retro pixel-art mitjançant textura dinàmica.

### Recursos i Nivells
- **`levels/`**: Fitxers [level01.txt](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/levels/level01.txt) fins a [level05.txt](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/levels/level05.txt).
- **`images/Menu/`**: Sprites oficials transparents (RGBA): `logo.png`, `cursor.png`, `copyright.png`, `hudson.png`, `pause.png`, `game_over.png`, `time_over.png`, `world_clear.png`, `knights_celebrate.png`.
- **`images/Sprites/Original/`**: Rips complets de la ROM original de Pocket Bomberman (tilesets de mons 1-5, personatges, enemics i bosses).
- **`images/door.png`**: Sprite de la porta oberta i tancada.

---

## 3. Resum Gràfic de Progrés Global

```
[████████████░░░░░░░░] 60% Progrés Total del Projecte

  Part Bàsica (6 punts):
  - Pantalles del joc (Menú, Jugar, Inst., Crèdits) : 100% [██████████]
  - Moviment, salt i càmera de Bomberman            : 100% [██████████]
  - Bombes a l'aire i salt sobre bombes             : 100% [██████████]
  - Temps límit de nivell (120s)                    : 100% [██████████]
  - Sistema de 3 vides i Game Over                  :  95% [██████████]
  - Cinc nivells i porta de sortida                 :  60% [██████░░░░]
  - HUD bàsic (Vides, Temps, Nivell, God Mode)      :  60% [██████░░░░]
  - Tecles de truc / Cheats (G, K, 1-5)             :  85% [████████░░]
  - Blocs destructibles als mapes                   :   0% [░░░░░░░░░░]
  - 4 Objectes (Bomb Up, Fire Up, Cor, Extra)       :  15% [██░░░░░░░░]
  - 3 Enemics amb IA diferenciada                   :   0% [░░░░░░░░░░]
  - Boss final al nivell 5                          :   0% [░░░░░░░░░░]

  Polish (4 punts):
  - Interactivitat i animacions d'entitats / menús   :  50% [█████░░░░░]
  - Game Feeling i fidelitat a Pocket Bomberman      :  50% [█████░░░░░]
  - So (Música BGM i Efectes SFX)                   :   0% [░░░░░░░░░░]
```

---

## 4. Pla d'Acció Immediat per Completar els Requeriments

Per assolir la màxima qualificació (10/10), les tasques prioritàries a desenvolupar a `2DGame` són:

1. **Blocs Destructibles i Drops (`TileMap`)**:
   - Assignar identificador de tile destructible (ex: arbust de GBC).
   - En impactar una explosió, canviar el tile a buit i reconstruir el VBO.
   - Probabilitat d'instanciar un dels 4 power-ups a la posició del bloc trencat.

2. **Els 4 Power-ups / Objectes**:
   - `Bomb Up`: incrementa `player->maxBombs`.
   - `Fire Up`: incrementa el radi de les explosions de les bombes.
   - `Cor`: absorbeix un cop sense perdre vida o atorga una vida extra.
   - `Quart objecte`: velocitat de moviment (patins) o bonificació de punts/temps.

3. **Els 3 Enemics amb IA Diferent**:
   - Classe base `Enemy` amb mètodes d'actualització, renderitzat i col·lisió.
   - Enemic 1: Patrulla terrestre horitzontal (canvia de sentit en tocar paret o buit).
   - Enemic 2: Enemic volador o caçador que es mou cap a la posició de Bomberman.
   - Enemic 3: Enemic ràpid o amb patrons de salt impredictibles.
   - En eliminar tots els enemics del nivell, la `Door` passa a estat obert (`door->setOpen(true)`).
   - Completar la tecla `K` perquè buidi la llista d'enemics a més d'obrir la porta.

4. **Boss Final al Nivell 5**:
   - Sprite de gran mida, barra de vida o múltiples cops requerits per derrotar-lo, i patró d'atac especial (llançament de projectils o càrregues).

5. **Completar els elements pendents del HUD**:
   - Afegir al HUD: nombre d'enemics restants, bombes màximes simultànies i potència d'explosió.

6. **Àudio (Polish)**:
   - Integrar reproducció de música chiptune retro per a menús i nivells, així com SFX d'explosió, salt, col·locació de bomba i recollida d'objectes.

7. **Preparació del Paquet de Lliurament**:
   - Crear `info.txt`, organitzar la carpeta `Binari/` amb l'executable compilat i gravar la demo `demo.avi` d'1 minut.
