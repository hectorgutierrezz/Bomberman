# Estat del Projecte: Pocket Bomberman 2D (FIB - VJ 2026/27)

Aquest document detalla l'estat actual del desenvolupament del videojoc **Pocket Bomberman** desenvolupat a `2DGame/`, contrastat punt per punt amb els requeriments de la guia de la pràctica (**FIB - Videojocs Curs 2026/27 Q1: Laboratori Exercici 2D**), incorporant l'estat real del codi a la branca `development` i el full de ruta amb les noves tasques i millores pendents.

---

## 1. Avaluació vs Requeriments Oficials de l'Enunciat

L'exercici s'avalua sobre 10 punts dividits en dues parts principals: **Part Bàsica (6 punts)** i **Polish (4 punts)**.

### 1.1. Part Bàsica (6 punts)

| # | Requeriment de l'Enunciat (PDF) | Estat Actual | Progrés | Detall de la Implementació a `2DGame` |
| :-: | :--- | :---: | :---: | :--- |
| **1** | **Estructura bàsica de 4 pantalles**<br>*(Menú principal, jugar, instruccions, crèdits)* | **Completat** | 100% | Les 4 pantalles principals estan implementades ([`MAIN_MENU`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Scene.h#L24), `PLAYING`, `INSTRUCTIONS`, `CREDITS`) més estats auxiliars (`PAUSED`, `GAME_OVER`, `WIN`). Navegació amb fletxes/WASD, selecció amb Enter/Espai, dreceres 1-4 i tecla `ESC` per pausar o tornar enrere. Menús maquetats amb sprites oficials de GBC (`images/Menu/`) i [BitmapText](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/BitmapText.h).<br>*(Pendent de millora: afegir **Choose Level Screen** al prémer Start Game).* |
| **2** | **Cinc nivells amb eliminació d'enemics i porta de sortida** | **Completat** | 85% | - Creades les plantilles dels 5 nivells (`levels/level01.txt` a `level05.txt`).<br>- Entitat [Door](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Door.h) implementada amb sprite tancada i oberta.<br>- **Condició d'obertura activa:** La porta comença **tancada** a cada nivell i només s'obre quan s'han eliminat tots els enemics ([`countAliveEnemies() == 0`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Scene.cpp#L208-L213)).<br>- En superar el nivell 5 s'arriba a la pantalla de victòria (`WIN`).<br>*(Pendents claus: **Redissenyar completament els 5 nivells**, aplicar el **tileset oficial de Game Boy Color** i afegir un **missatge a pantalla que avisi clarament que la porta ja està oberta**).* |
| **3** | **Moviment d'esquerra a dreta i salt de Bomberman** | **Completat** | 100% | Físiques de moviment horitzontal, gravetat amb acceleració realista ([`GRAVITY 0.4f`, `MAX_FALL_SPEED 8.0f`, `stepFall`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp#L128-L160)) i salt parabòlic a [Player.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp). Spritesheet oficial de GBC (`bomber.png`) amb animacions `IDLE`, `STAND_LEFT/RIGHT`, `MOVE_LEFT/RIGHT` i `FALL`. Càmera dinàmica suau (*lerp*) que segueix el personatge. |
| **4** | **Tres enemics amb comportaments diferents**<br>*(Contacte resta vida si no té el «cor»)* | **Completat** | 100% | **Classe [Enemy](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Enemy.h) implementada amb 3 IA i sprites diferenciats:**<br>1. **`ENEMY_HORIZONTAL` (Caminant groc):** Patrulla sobre terra/plataformes; inverteix la marxa en topar amb parets, bombes o en detectar la vora d'una plataforma ([`wouldFall`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Enemy.cpp#L185-L193)).<br>2. **`ENEMY_VERTICAL` (Peix volador):** Patrulla amunt i avall de manera suau; inverteix marxa amb sostre, terra o en superar el radi límit.<br>3. **`ENEMY_STATIC` (Globus Valcom):** Vigilant estàtic amb lleugera levitació (*bobbing*); gira la mirada cap a Bomberman, accelera l'animació si s'apropa (<96px) i **detona una explosió en morir**.<br>- **Dany al jugador:** El contacte resta 1 vida interna/cor; si no té cors addicionals, inicia la seqüència de mort. |
| **5** | **Perdre les 3 vides implica perdre la partida i tornar al menú** | **Completat** | 90% | **Sistema fidel a Pocket Bomberman amb dos nivells de vides:**<br>- **Cors de nivell (Vida interna):** 1 de base per pantalla (augmentable amb cors). En rebre dany amb >1 cor, reprodueix animació `HURT` durant 350ms i parpelleja invulnerable 2s.<br>- **Vides Generals (3 intents globals):** Si rep dany sense cors addicionals, s'executa l'animació oficial de mort de 7 frames (`DIE`, 1.2s), es resta 1 vida general i es reinicia el nivell mantenint power-ups.<br>- En perdre les 3 vides generals (0), salta el Game Over definitiu i qualsevol tecla (`ENTER`/`SPACE`/`ESC`/`M`) retorna al menú principal.<br>*(Pendent de millora: **Pantalla intermèdia en perdre una vida** que avisi de l'oportunitat restant amb comptador estil GBC abans de reiniciar).* |
| **6** | **Deixar anar bombes (alineades amb tiles) que maten enemics o Bomberman** | **Completat** | 98% | Bombes alineades a la graella (`round(pos/16)*16`). Classe [Bomb](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Bomb.h) amb metxa de 4 frames i detonació a 2.2s. Classe [Explosion](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Explosion.h) en creu (4 direccions) amb col·lisions contra parets, reaccions en cadena, dany a Bomberman i **eliminació d'enemics** ([`enemy->kill()`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Scene.cpp#L876-L885)). *(Només pendent: destruir blocs destructibles).* |
| **7** | **Saltar sobre les seves bombes per arribar a zones elevades** | **Completat** | 100% | Bombes estàtiques a l'aire fidels a *Pocket Bomberman*. Superfície superior sòlida (`isSolidForPlayer`), permetent recolzar-s'hi i usar-les com a plataformes o escales per assolir plataformes altes. |
| **8** | **Tiles destructibles (arbustos/blocs) que poden deixar anar objectes** | **Pendent** | 0% | Falta afegir identificadors de rajoles destructibles a [TileMap](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/TileMap.h), trencar-los amb les explosions i generar objectes (*drops*). |
| **9** | **Quatre objectes (com a mínim ‘Bomb Up’ i ‘Fire Up’)** | **Pendent** | 20% | Lògica interna preparada a [Player](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.h) (`maxBombs`, `internalLives`, mètodes `addMaxBomb()`, `addInternalLife()`). Pendent crear les entitats i sprites col·leccionables al mapa: `Bomb Up`, `Fire Up`, `Cor` i `Velocitat`. |
| **10** | **Temps límit per passar-se cada nivell** | **Completat** | 100% | Temporitzador enrere de 120s (2 min) per nivell. Si s'esgota, pantalla de *Time Over* amb sprite autèntic de Bomberman marejat. |
| **11** | **Boss final al nivell 5** | **Pendent** | 0% | El nivell 5 disposa d'spawn i enemics, però falta programar el cap final (múltiples punts de vida, sprite gran i patró d'atac). |
| **12** | **Interfície gràfica (HUD) amb dades mínimes requerides** | **Completat** | 80% | - **Temps restant:** `TIME MM:SS` (vermell si <= 30s).<br>- **Vides generals:** Icones de la cara de Bomberman (3 intents globals).<br>- **Cors de nivell:** Cors pixel-art renderitzats proceduralment en vermell/blanc al costat de les vides.<br>- **Nivell actual:** Format `1-1`, `1-2`, ...<br>- **Comptador d'enemics:** Mostra `E:X` en groc i `OPEN!` en verd quan no en queden.<br>*(Pendent: **Redistribució de la interfície gràfica** per millorar alineació, espaiat i claredat visual).* |
| **13** | **Tecles de truc (Cheats de correcció)** | **Completat** | 95% | - **`G`:** Activa / desactiva God Mode (invulnerabilitat total + indicador en pantalla).<br>- **`K`:** **Elimina enemics i obre la porta de sortida** (assegurar mort immediata i neteja instantània de tots els enemics actius).<br>- **`1`, `2`, `3`, `4`, `5`:** Salt directe a qualsevol dels 5 nivells.<br>- **`H`:** Afegeix +1 cor / vida de nivell (+1 HP).<br>- **`B`:** Afegeix +1 capacitat de bombes simultànies.<br>- **`N` / `P`:** Següent / anterior nivell. |

---

### 1.2. Polish (4 punts)

| Aspecte de Polish | Estat Actual | Progrés | Descripció i Estat |
| :--- | :---: | :---: | :--- |
| **So (Música de fons i efectes especials)** | **Pendent** | 0% | Falta incorporar el reproductor d'àudio (ex: `miniaudio.h`) per a música de nivells/menús i efectes sonors (bomba, explosió, salt, recollida, dany, mort). |
| **Interactivitat i Animacions de totes les entitats** | **Completat** | 80% | - **Bomberman:** Animacions de moviment, salt, caiguda, parpelleig, reacció de dolor [`HURT`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp#L97-L99) i seqüència oficial de mort de 7 frames [`DIE`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp#L101-L109).<br>- **Enemics:** Tots 3 tipus compten amb cicle de moviment fluid (`MOVE`) i animació de mort (`DIE`).<br>- **Bombes i explosions:** Animació de metxa (4 frames) i explosió en creu fluida (6 frames).<br>- **Menús i UI:** Cursor animat, pantalles de pausa, game over, time over i celebració de victòria. |
| **Game feeling (Fidelitat, diversió i feedback)** | **En Progrés** | 65% | Plataformes sobre bombes, caiguda amb gravetat realista (`stepFall`), càmera suau, estètica fidel GBC, feedback visual de dany (`HURT` + parpelleig), alerta de temps crític en vermell, porta reactiva a la neteja d'enemics. |

---

### 1.3. Requisits del Lliurament Final (Pàgina 3 del PDF)

| Requisit de Lliurament | Especificació del PDF | Estat de Preparació |
| :--- | :--- | :--- |
| **Format del fitxer** | `NomCognoms.zip` (màxim **200 MB** en total) | Pendent en finalitzar el projecte. |
| **Fitxer `info.txt`** | Noms i cognoms dels integrants, funcionalitats implementades i instruccions del joc | Pendent de redactar abans de l'entrega. |
| **Carpeta `Binari`** | Executable i tots els recursos necessaris (imatges, nivells, llibreries/dlls) llest per executar | Netejat el repositori d'arxius obsolets (`OpenGL/`, `exercicis.pdf`); llest per empaquetar l'executable. |
| **Carpeta `Projecte`** | Projecte compilable amb tot el codi font net | Organitzat exclusivament a `2DGame/02-Bubble/`. |
| **Vídeo `demo.avi`** | Vídeo de mostra d'**1 minut**, mida màxima **100 MB** (usant OBS/FRAPS i HandBrake) | Pendent de gravar i comprimir un cop integrat el so i els blocs. |

---

## 2. Mapa dels Fitxers del Projecte (`2DGame/02-Bubble/`)

### Codi Font (`02-Bubble/`)
- [Scene.h](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Scene.h) / [Scene.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Scene.cpp):
  - Màquina d'estats: `MAIN_MENU`, `INSTRUCTIONS`, `CREDITS`, `PLAYING`, `PAUSED`, `GAME_OVER`, `WIN`.
  - Càrrega i gestió de nivells 1 a 5 (`startLevel()`, `nextLevel()`).
  - Gestió integral d'entitats: jugador, porta, vector de bombes, explosions i llista d'[Enemy](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Enemy.h).
  - Condició d'obertura de porta dinàmica (`updateDoorFromEnemies()`) segons el nombre d'enemics vius.
  - HUD complet: vides generals (cares), cors de nivell (procedural), temps restant (amb alerta vermella), nivell i comptador d'enemics restants (`E:X` / `OPEN!`).
  - Càmera dinàmica 2D amb interpolació (*lerp*).
  - Tecles de truc: `G` (God Mode), `K` (elimina enemics + obre porta), `H` (+1 cor), `B` (+1 bomb up), `1`..`5`, `N`/`P`.
- [Enemy.h](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Enemy.h) / [Enemy.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Enemy.cpp):
  - Jerarquia d'enemics amb 3 variants: `ENEMY_HORIZONTAL`, `ENEMY_VERTICAL` i `ENEMY_STATIC`.
  - Físiques i comportaments: detecció de caiguda de plataformes (`wouldFall`), col·lisions amb parets i bombes, seguiment de mirada i explosió en morir per a l'enemic estàtic.
  - Sprites i animacions oficials de GBC (`MOVE` i `DIE`).
- [Player.h](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.h) / [Player.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp):
  - Model i físiques de Bomberman amb spritesheet oficial `bomber.png`.
  - Animacions completes: `IDLE`, `STAND_LEFT/RIGHT`, `MOVE_LEFT/RIGHT`, `FALL`, `HURT` i seqüència de mort `DIE` (7 frames).
  - Físiques de gravetat amb acceleració (`GRAVITY`, `MAX_FALL_SPEED`, `stepFall`).
  - Gestió de vides internes (`internalLives`) i cicle de mort (`startDying()`, `isDying()`, `isDead()`).
  - Suport per recolzar-se sobre bombes sòlides a l'aire com a plataformes.
- [Door.h](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Door.h) / [Door.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Door.cpp):
  - Entitat de la porta de sortida del nivell amb estats tancada i oberta.
  - Detecció de pas de nivell quan està oberta i Bomberman col·lisiona amb ella.
- [Bomb.h](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Bomb.h) / [Bomb.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Bomb.cpp):
  - Alineació a la graella (tiles de 16x16 / 32x32).
  - Plataforma sòlida (`isSolidForPlayer`) sense gravetat.
- [Explosion.h](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Explosion.h) / [Explosion.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Explosion.cpp):
  - Propagació en creu de 4 direccions amb animació fluida de 6 frames.
  - Detonació en cadena de bombes, dany a Bomberman i destrucció d'enemics.
- [TileMap.h](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/TileMap.h) / [TileMap.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/TileMap.cpp):
  - Càrrega de fitxers de nivell i generació de malla OpenGL.
  - Mètodes de comprovació de col·lisions sòlides (`collisionMoveLeft`, `collisionMoveRight`, `collisionMoveDown`, `collisionMoveUp`, `isSolidTile`).
- [BitmapText.h](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/BitmapText.h) / [BitmapText.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/BitmapText.cpp):
  - Renderització de text retro amb font pixel-art (inclosos caràcters especials com `+` i `:`).

### Recursos i Nivells
- **`levels/`**: Fitxers [level01.txt](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/levels/level01.txt) fins a [level05.txt](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/levels/level05.txt).
- **`images/Menu/`**: Sprites oficials transparents (RGBA): `logo.png`, `cursor.png`, `copyright.png`, `hudson.png`, `pause.png`, `game_over.png`, `time_over.png`, `world_clear.png`, `knights_celebrate.png`.
- **`images/Sprites/Original/`**: Rips complets de la ROM original de Pocket Bomberman (tilesets de mons 1-5, personatges, enemics i bosses).
- **`images/door.png`**: Sprite de la porta oberta i tancada.

---

## 3. Resum Gràfic de Progrés Global

```
[██████████████░░░░░░] 72% Progrés Total del Projecte

  Part Bàsica (6 punts):
  - Pantalles del joc (Menú, Jugar, Inst., Crèdits) : 100% [██████████]
  - Moviment, salt i càmera de Bomberman            : 100% [██████████]
  - Bombes a l'aire i salt sobre bombes             : 100% [██████████]
  - Temps límit de nivell (120s)                    : 100% [██████████]
  - 3 Enemics amb IA diferenciada                   : 100% [██████████]
  - Bombes i explosions eliminen enemics/jugador    :  98% [██████████]
  - Tecles de truc / Cheats (G, K, 1-5, H, B)       :  95% [██████████]
  - Sistema de vides i animació oficial de mort     :  90% [█████████░]
  - Cinc nivells i porta condicionada a enemics     :  85% [█████████░]
  - HUD complet (Vides, Cors, Temps, Nivell, E:X)   :  80% [████████░░]
  - 4 Objectes (Bomb Up, Fire Up, Cor, Extra)       :  20% [██░░░░░░░░]
  - Blocs destructibles als mapes                   :   0% [░░░░░░░░░░]
  - Boss final al nivell 5                          :   0% [░░░░░░░░░░]

  Polish (4 punts):
  - Interactivitat i animacions d'entitats / menús   :  80% [████████░░]
  - Game Feeling i fidelitat a Pocket Bomberman      :  65% [███████░░░]
  - So (Música BGM i Efectes SFX)                   :   0% [░░░░░░░░░░]
```

---

## 4. Llista de Funcionalitats i Millores Pendents de Desenvolupament

A continuació es detallen les tasques prioritàries pendents per completar el projecte i donar-li la màxima qualitat:

### 4.1. Noves Pantalles i Transicions de Joc
1. **Pantalla intermèdia quan es perd una vida del joc:**
   - Fidel al funcionament de *Pocket Bomberman* original: en comptes de reiniciar sobtadament la pantalla com si res, mostrar una pantalla de transició tipus *TRY AGAIN* / *READY* amb el comptador de vides restants de Bomberman (ex: `BOMBERMAN x 2`), informant al jugador que té una nova oportunitat abans de tornar al nivell.
2. **Pantalla de Selecció de Nivell (`CHOOSE LEVEL SCREEN`):**
   - En seleccionar *"1. START GAME"* al menú principal, navegar a una pantalla de selecció de nivell amb els 5 mons/fases (Nivell 1 a 5), permetent triar per quin nivell començar directament.

### 4.2. Gràfics, Tilesets i Disseny de Nivells
3. **Redissenyar els 5 Nivells (`levels/level01.txt` a `level05.txt`):**
   - Dissenyar mapes verticals i plataformes que aprofitin de veritat les mecàniques de Pocket Bomberman: plataformes desafiants, zones on sigui obligatori saltar sobre bombes col·locades a l'aire i rutes estratègiques amb enemics.
4. **Integració del Tileset oficial de Game Boy Color:**
   - Els tiles actuals són massa bàsics. Cal actualitzar el fitxer de textura del `TileMap` amb els tilesets autèntics de GBC (`images/Sprites/Original/Color/Tilesets/`) per donar l'estètica pixel-art oficial de cada món (gespa, coves, castells, etc.).

### 4.3. Millores d'Interfície (UI/UX) i Feedback
5. **Redistribuir la interfície gràfica (HUD):**
   - Polir la distribució, marges i jerarquia visual del HUD superior: alinear ordenadament les vides generals, els cors de nivell, l'indicador del nivell actual, el comptador d'enemics i el temporitzador per evitar solapaments en qualsevol amplada de càmera.
6. **Missatge a pantalla d'avís de porta oberta:**
   - Quan es derroti l'últim enemic del nivell (`countAliveEnemies() == 0`), mostrar un missatge visual prominent a pantalla (estil banner groc/verd com el de *God Mode*, ex: `DOOR OPEN! / SORTIDA OBERTA!`) per donar un feedback immediat i clar al jugador que ja pot sortir.
7. **Reforçar la tecla de truc `K` (Cheat):**
   - Assegurar que en prémer la tecla `K`, a més d'obrir la porta, s'eliminin i netegin de forma immediata tots els enemics actius del nivell.

### 4.4. Mecàniques Fonamentals del Joc (Part Bàsica)
8. **Blocs Destructibles als Mapes (`TileMap`):**
   - Assignar identificadors de tiles destructibles (arbustos/blocs tous) que es trenquin en impactar una explosió i regenerin la malla del mapa.
9. **Entitats Col·leccionables dels 4 Power-ups:**
   - Probabilitat de *drop* en trencar un bloc: `Bomb Up` (+1 bomba), `Fire Up` (+radi explosió), `Cor` (+1 vida de nivell) i `Patins` (+velocitat de moviment).
10. **Boss Final al Nivell 5:**
    - Dissenyar el cap final del món al nivell 5: sprite de grans dimensions de Pocket Bomberman, barra de vida / múltiples impactes i patrons d'atac propis.

### 4.5. Polish i Lliurament Final
11. **Motor d'Àudio (BGM i SFX):**
    - Integrar `miniaudio.h` per reproduir música chiptune retro a menús i nivells, juntament amb efectes de so per a bombes, explosions, salts, recollida d'objectes i mort.
12. **Preparació del Paquet de Lliurament:**
    - Redacció d'`info.txt`, organització de la carpeta `Binari/` i enregistrament del vídeo `demo.avi` d'1 minut.
