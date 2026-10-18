# Estat del Projecte: Pocket Bomberman 2D (FIB - VJ 2026/27)

Aquest document detalla l'estat actual del desenvolupament del videojoc **Pocket Bomberman** desenvolupat a `2DGame/`, contrastat punt per punt amb els requeriments de la guia de la pràctica (**FIB - Videojocs Curs 2026/27 Q1: Laboratori Exercici 2D**), incorporant l'estat real del codi a la branca `development` i el full de ruta amb les noves tasques i millores pendents.

> **Actualització (18/10/2026):** els blocs destructibles i els quatre tipus de power-up ja estan implementats als nivells. Les explosions trenquen les rajoles marcades amb `D`, regeneren la malla i fan aparèixer els power-ups en rotació (`Bomb Up`, `Fire Up`, cor i patins). La pantalla d'inici de nivell ja no avança automàticament: cal prémer Enter o Esc. El fons dels nivells torna a ser negre; es conserva tota la mecànica dels blocs. La compilació Linux amb `make -C 2DGame/02-Bubble all` s'ha verificat correctament.

---

## 1. Avaluació vs Requeriments Oficials de l'Enunciat

L'exercici s'avalua sobre 10 punts dividits en dues parts principals: **Part Bàsica (6 punts)** i **Polish (4 punts)**.

### 1.1. Part Bàsica (6 punts)

| # | Requeriment de l'Enunciat (PDF) | Estat Actual | Progrés | Detall de la Implementació a `2DGame` |
| :-: | :--- | :---: | :---: | :--- |
| **1** | **Estructura bàsica de 4 pantalles**<br>*(Menú principal, jugar, instruccions, crèdits)* | **Completat** | 100% | Les 4 pantalles principals estan implementades ([`MAIN_MENU`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Scene.h#L24), `PLAYING`, `INSTRUCTIONS`, `CREDITS`) més les pantalles auxiliars `CHEATS`, `PAUSED`, `GAME_OVER` i `WIN`. El menú, les instruccions, els controls i els missatges de joc estan en català. La font bitmap no admet diacrítics, de manera que els textos visibles s'escriuen sense accents. Navegació amb fletxes/WASD, selecció amb Enter/Espai i accés a les pantalles amb les opcions numerades 1-5. Menús maquetats amb sprites oficials de GBC (`images/Menu/`) i [BitmapText](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/BitmapText.h).<br>*(Pendent de millora: afegir **Choose Level Screen** en iniciar una partida).* |
| **2** | **Cinc nivells amb eliminació d'enemics i porta de sortida** | **Completat** | 100% | - Creades les plantilles dels 5 nivells (`levels/level01.txt` a `level05.txt`) amb **disseny vertical autèntic de plataformes per a Pocket Bomberman**.<br>- **Integrat el Tileset oficial de Game Boy Color** ([`images/Map/world_tileset.png`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/images/Map/world_tileset.png)) amb temàtiques visuals per a cada món (gespa/bosc, desert/sorra, temple daurat, magma volcànic i gel/fortalesa).<br>- Entitat [Door](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Door.h) implementada amb sprite tancada i oberta.<br>- **Condició d'obertura activa:** La porta comença **tancada** a cada nivell i només s'obre quan s'han eliminat tots els enemics ([`countAliveEnemies() == 0`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Scene.cpp#L208-L213)). Quan s'obre, es mostra durant 2,5 segons el missatge «PORTA OBERTA!».<br>- En superar el nivell 5 s'arriba a la pantalla de victòria (`WIN`). |
| **3** | **Moviment d'esquerra a dreta i salt de Bomberman** | **Completat** | 100% | Físiques de moviment horitzontal, gravetat amb acceleració realista ([`GRAVITY 0.4f`, `MAX_FALL_SPEED 8.0f`, `stepFall`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp#L128-L160)) i salt parabòlic a [Player.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp). Spritesheet oficial de GBC (`bomber.png`) amb animacions `IDLE`, `STAND_LEFT/RIGHT`, `MOVE_LEFT/RIGHT` i `FALL`. Càmera dinàmica suau (*lerp*) que segueix el personatge. |
| **4** | **Tres enemics amb comportaments diferents**<br>*(Contacte resta vida si no té el «cor»)* | **Completat** | 100% | **Classe [Enemy](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Enemy.h) implementada amb 3 IA i sprites diferenciats:**<br>1. **`ENEMY_HORIZONTAL` (Caminant groc):** Patrulla sobre terra/plataformes; inverteix la marxa en topar amb parets, bombes o en detectar la vora d'una plataforma ([`wouldFall`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Enemy.cpp#L185-L193)).<br>2. **`ENEMY_VERTICAL` (Peix volador):** Patrulla amunt i avall de manera suau; inverteix marxa amb sostre, terra o en superar el radi límit.<br>3. **`ENEMY_STATIC` (Globus Valcom):** Vigilant estàtic amb lleugera levitació (*bobbing*); gira la mirada cap a Bomberman, accelera l'animació si s'apropa (<96px) i **detona una explosió en morir**.<br>- **Dany al jugador:** El contacte resta 1 vida interna/cor; si no té cors addicionals, inicia la seqüència de mort. |
| **5** | **Perdre les 3 vides implica perdre la partida i tornar al menú** | **Completat** | 100% | **Sistema fidel a Pocket Bomberman amb dos nivells de vides:**<br>- **Cors de nivell (Vida interna):** 1 de base per pantalla (augmentable amb cors). En rebre dany amb >1 cor, reprodueix animació `HURT` durant 350ms i parpelleja invulnerable 2s.<br>- **Vides Generals (3 intents globals):** Si rep dany sense cors addicionals, s'executa l'animació oficial de mort de 7 frames (`DIE`, 1.2s) i es resta 1 vida general.<br>- **Pantalla intermèdia `TRY_AGAIN` IMPLEMENTADA:** En comptes de reiniciar sobtadament, s'activa la pantalla de transició estil GBC amb textos en català, sprite oficial de la cara de Bomberman i comptador de vides restants (`x 2`). La pantalla espera indefinidament fins que es reprèn amb Enter/Espai/X/Z o un clic, o es torna al menú amb ESC/M.<br>- En perdre les 3 vides generals (0), salta el Game Over definitiu i qualsevol tecla retorna al menú principal. |
| **6** | **Deixar anar bombes (alineades amb tiles) que maten enemics o Bomberman** | **Completat** | 100% | Bombes alineades a la graella (`round(pos/16)*16`). Classe [Bomb](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Bomb.h) amb metxa de 4 frames i detonació a 2.2s. Classe [Explosion](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Explosion.h) en creu (4 direccions), amb col·lisions contra parets, reaccions en cadena, dany a Bomberman, eliminació d'enemics i destrucció de blocs tous amb actualització de la malla. |
| **7** | **Saltar sobre les seves bombes per arribar a zones elevades** | **Completat** | 100% | Bombes estàtiques a l'aire fidels a *Pocket Bomberman*. Superfície superior sòlida (`isSolidForPlayer`), permetent recolzar-s'hi i usar-les com a plataformes o escales per assolir plataformes altes. |
| **8** | **Tiles destructibles (arbustos/blocs) que poden deixar anar objectes** | **Completat** | 100% | El caràcter `D` identifica blocs destructibles als cinc nivells. Les explosions s'aturen en arribar-hi, eliminen el bloc i regeneren la malla de [TileMap](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/TileMap.h); en trencar-lo, apareix un power-up. |
| **9** | **Quatre objectes (com a mínim ‘Bomb Up’ i ‘Fire Up’)** | **Completat** | 100% | Les entitats recollibles són `Bomb Up`, `Fire Up`, cor i patins. Cada bloc trencat genera un tipus en rotació; recollir-lo aplica l'efecte al jugador. |
| **10** | **Temps límit per passar-se cada nivell** | **Completat** | 100% | Temporitzador enrere de 120s (2 min) per nivell. Si s'esgota, pantalla de *Time Over* amb sprite autèntic de Bomberman marejat. |
| **11** | **Boss final al nivell 5** | **Pendent** | 0% | El nivell 5 disposa d'spawn i enemics, però falta programar el cap final (múltiples punts de vida, sprite gran i patró d'atac). |
| **12** | **Interfície gràfica (HUD) amb dades mínimes requerides** | **Completat** | 80% | - **Temps restant:** `TIME MM:SS` (vermell si <= 30s).<br>- **Vides generals:** Icones de la cara de Bomberman (3 intents globals).<br>- **Cors de nivell:** Cors pixel-art renderitzats proceduralment en vermell/blanc al costat de les vides.<br>- **Nivell actual:** Format `1-1`, `1-2`, ...<br>- **Comptador d'enemics:** Mostra `E:X` en groc i `OPEN!` en verd quan no en queden.<br>Redistribució aplicada per mantenir els elements llegibles a l'amplada de la càmera. |
| **13** | **Tecles de truc (Cheats de correcció)** | **Completat** | 95% | La pantalla **TRUCS**, separada de les instruccions i accessible des del menú principal, mostra els codis disponibles durant la partida: **`G`** activa/desactiva la invulnerabilitat; **`K`** elimina els enemics i obre la porta; **`H`** afegeix un cor; **`B`** augmenta la capacitat de bombes; **`F`** augmenta el radi de l'explosió; **`S`** augmenta la velocitat; **`N`** passa al nivell següent; **`1`–`5`** seleccionen directament un nivell. (`P` i `ESC` posen el joc en pausa.) |

---

### 1.2. Polish (4 punts)

| Aspecte de Polish | Estat Actual | Progrés | Descripció i Estat |
| :--- | :---: | :---: | :--- |
| **So (Música de fons i efectes especials)** | **Pendent** | 0% | Falta incorporar el reproductor d'àudio (ex: `miniaudio.h`) per a música de nivells/menús i efectes sonors (bomba, explosió, salt, recollida, dany, mort). |
| **Interactivitat i Animacions de totes les entitats** | **Completat** | 80% | - **Bomberman:** Animacions de moviment, salt, caiguda, parpelleig, reacció de dolor [`HURT`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp#L97-L99) i seqüència oficial de mort de 7 frames [`DIE`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp#L101-L109).<br>- **Enemics:** Tots 3 tipus compten amb cicle de moviment fluid (`MOVE`) i animació de mort (`DIE`).<br>- **Bombes i explosions:** Animació de metxa (4 frames) i explosió en creu fluida (6 frames).<br>- **Menús i UI:** Cursor animat, pantalles de pausa, game over, time over i celebració de victòria. Avisos de god mode «MODE DEU ON/OFF» a mida reduïda perquè càpiguen a la pantalla. |
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
  - Destrucció de blocs `D` per explosions, regeneració de la malla i generació rotativa dels quatre power-ups.
  - Condició d'obertura de porta dinàmica (`updateDoorFromEnemies()`) segons el nombre d'enemics vius.
  - HUD complet: vides generals (cares), cors de nivell (procedural), temps restant (amb alerta vermella), nivell i comptador d'enemics restants (`E:X` / `OPEN!`).
  - Càmera dinàmica 2D amb interpolació (*lerp*).
  - Textos del menú, les instruccions i els missatges de joc en català (sense diacrítics per les limitacions de la font bitmap).
  - Tecles de truc: `G` (invulnerabilitat), `K` (elimina enemics i obre la porta), `H` (+1 cor), `B` (+bombes), `F` (+radi d'explosió), `S` (+velocitat), `1`..`5` (selecció de nivell) i `N` (nivell següent). `P` posa el joc en pausa.
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
  - Identificació i destrucció de rajoles `D`, amb reconstrucció de la malla després de cada explosió.
- [BitmapText.h](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/BitmapText.h) / [BitmapText.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/BitmapText.cpp):
  - Renderització de text retro amb font pixel-art (inclosos caràcters especials com `+` i `:`).

### Recursos i Nivells
- **`levels/`**: Fitxers [level01.txt](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/levels/level01.txt) fins a [level05.txt](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/levels/level05.txt).
- **`images/Menu/`**: Sprites oficials transparents (RGBA): `logo.png`, `cursor.png`, `copyright.png`, `hudson.png`, `pause.png`, `game_over.png`, `time_over.png`, `world_clear.png`, `knights_celebrate.png`.
- **`images/Sprites/Original/`**: Rips complets de la ROM original de Pocket Bomberman (tilesets de mons 1-5, personatges, enemics i bosses).
- **`images/door.png`**: Sprite de la porta oberta i tancada.
- **`images/powerups.png`**: Sprites dels quatre objectes recollibles.

---

## 3. Resum Gràfic de Progrés Global

```
[████████████████░░░░] 82% Progrés Total del Projecte (estimació actualitzada)

  Part Bàsica (6 punts):
  - Pantalles del joc (Menú, Jugar, Inst., Trucs, Crèdits): 100% [██████████]
  - Moviment, salt i càmera de Bomberman            : 100% [██████████]
  - Bombes a l'aire i salt sobre bombes             : 100% [██████████]
  - Temps límit de nivell (120s)                    : 100% [██████████]
  - 3 Enemics amb IA diferenciada                   : 100% [██████████]
  - Tecles de truc / Cheats (G, K, 1-5, H, B, F, S, N): 95% [██████████]
  - Sistema de vides i pantalla TRY_AGAIN           : 100% [██████████]
  - Cinc nivells i porta condicionada a enemics     : 100% [██████████]
  - HUD complet (Vides, Cors, Temps, Nivell, E:X)   :  80% [████████░░]
  - 4 Objectes (Bomb Up, Fire Up, Cor, Patins)       : 100% [██████████]
  - Blocs destructibles als mapes                   : 100% [██████████]
  - Boss final al nivell 5                          :   0% [░░░░░░░░░░]

  Polish (4 punts):
  - Tileset oficial GBC i estètica gràfica retro    :  95% [██████████]
  - Interactivitat i animacions d'entitats / menús   :  80% [████████░░]
  - Game Feeling i fidelitat a Pocket Bomberman      :  80% [████████░░]
  - So (Música BGM i Efectes SFX)                   :   0% [░░░░░░░░░░]
```

---

## 4. Llista de Funcionalitats i Millores Pendents de Desenvolupament

A continuació es detallen les tasques prioritàries pendents per completar el projecte i donar-li la màxima qualitat:

### 4.1. Noves Pantalles i Transicions de Joc
1. **(COMPLETAT) Pantalla intermèdia quan es perd una vida del joc (`TRY_AGAIN`):**
   - Fidel al model de *Pocket Bomberman*: en morir tenint vides generals restants, es neteja l'escenari i s'activa la pantalla intermèdia amb textos en català (*«TORNA-HO A PROVAR!»*, *«TENS UNA ALTRA OPORTUNITAT!»*, *«PREM ENTER O ESPAI PER CONTINUAR»*), sprite oficial de la cara de Bomberman carregat des de `bomber.png` amb el comptador de vides restants (`x 2`), i **espera indefinida** fins que el jugador prem qualsevol tecla d'acció (Enter, Espai, X, Z) o fa clic de ratolí (amb opció d'anar al menú amb ESC o M). La font bitmap no admet accents.
2. **(COMPLETAT) Mini pantalla d'inici de cada nivell (`STAGE_INTRO`):**
   - Transició estil arcade a l'inici de cada fase: mostra *«NIVELL 1-X»*, avís *«PREPARAT!»*, icona de Bomberman amb les vides restants i indicació parpellejant *«PREM ENTER O ESC»*. El nivell només comença quan es prem Enter o Esc; no hi ha inici automàtic.
3. **(COMPLETAT) Correcció de l'arrencada del joc (Inici directe al Menú Principal):**
   - Corregit l'estat d'arrencada a [Scene.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Scene.cpp): en carregar el joc s'inicialitzen els recursos en segon pla i es fixa directament `MAIN_MENU`, evitant saltar directament al primer nivell. En prémer «JUGAR», s'activa la pantalla `STAGE_INTRO` del nivell 1.
4. **Pantalla de Selecció de Nivell (`CHOOSE LEVEL SCREEN`):**
   - En seleccionar *"1. START GAME"* al menú principal, navegar a una pantalla de selecció de nivell amb els 5 mons/fases (Nivell 1 a 5), permetent triar per quin nivell començar directament.

### 4.2. Gràfics, Tilesets i Disseny de Nivells
5. **(COMPLETAT) Redissenyar els 5 Nivells (`levels/level01.txt` a `level05.txt`) adaptats a l'escala 16x16:**
   - **Plataformes escalonades accessibles:** Les plataformes s'han reestructurat en desnivells exactes de 2 rajoles (32 px) d'altura entre terrasses consecutives. Amb l'alçada de salt de 44 px, qualsevol plataforma es pot escalar de forma fluida i sense bloquejos.
   - **Eliminació d'estrangulaments i murs cecs:** S'han suprimit els grans murs sòlids divisoris centrals que partien les fases en dos o atrapaven el jugador en carrerons sense sortida en deixar una bomba.
   - **Passadissos i terrasses espaioses:** Les zones transitables tenen un mínim de 2 a 3 rajoles d'alçada d'aire (32-48 px, el doble o triple que el personatge de 16x16) i una amplada generosa (de 4 a 10 blocs) per permetre córrer, esquivar enemics i retirar-se amb seguretat en plantar bombes.
   - **Ponts permeables de fusta (`'A'`):** Integrats estratègicament per connectar les terrasses, permetent tant enfilar-se des de sota com davallar prement Avall.
   - **Script de generació net:** Centralitzat a [`scratch/build_levels.py`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/scratch/build_levels.py) per mantenir una distribució matemàticament coherent entre tots 5 mons temàtics.
6. **(COMPLETAT) Reequilibrat de velocitats del joc i mides d'entitats (16x16):**
   - **Velocitat del personatge (`moveSpeed`):** Rebaixada de 2.0f a **1.35f px/frame** (~81 px/s, uns 12 frames per creuar un bloc de 16 px). El moviment és calmat, precís, retro i completament controlable en la càmera de 208 px.
   - **Power-up de patins:** Increment calibrat a `+0.25f` fins a un màxim de `2.1f` (evitant velocitats supersòniques incontrolables).
   - **Física del salt i caiguda:** Alçada de salt fixada a **44 px** (~2.75 blocs, ideal per pujar graons de 32 px), arc suau de 45 frames (`JUMP_ANGLE_STEP = 4`), gravetat ajustada a `0.22f` i velocitat màxima de caiguda reduïda a `3.5f` px/frame (eliminant la caiguda en picat sobtada).
   - **Velocitat dels enemics:** Moviment horitzontal dels enemics terrestres calibrat a **22 px/s** (abans 35) i vertical a **18 px/s** (abans 30), donant temps suficient al jugador per llegir les seves rutes, posar bombes i fugir.
   - **Suavitzat de càmera:** Càmera amb lerp suau a `0.08f` (abans 0.12f) per a un desplaçament cinematogràfic i còmode.
   - **Posicionament i escala de les bombes:** Base de la bomba reposant a la cara superior del terra (`y + 16`) i centrada al bloc (`x + 8`), en pixel-art 1:1 natiu.
7. **(COMPLETAT) Integració del Tileset oficial de Game Boy Color:**
   - Resolt el conflicte de mida de bloc (`blockSize 32` vs `tileSize 16`), eliminant el solapament visual entre rajoles.
   - Migrat del placeholder `blocks.png` a [`images/Map/world_tileset.png`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/images/Map/world_tileset.png), amb codificació alfanumèrica completa a [TileMap.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/TileMap.cpp) per admetre fins a 62 rajoles diferents de Game Boy Color per a cada món temàtic (món 1 herba/terra, món 2 desert, món 3 temple daurat, món 4 magma volcànic, món 5 gel).
8. **(COMPLETAT) Variació de quantitat i tipologia d'enemics per nivell:**
   - La quantitat i tipus d'enemics varia segons la fase: Nivell 1 (2 enemics terrestres, introductori sense voladors), Nivell 2 (3 enemics amb voladors en pous verticals), Nivell 3 (4 enemics amb globus estàtic), Nivell 4 (4 enemics volcànics amb 2 voladors) i Nivell 5 (5 enemics protegint la porta final).

### 4.3. Millores d'Interfície (UI/UX) i Feedback
9. **(COMPLETAT) Redistribució de la interfície gràfica (HUD):**
   - Adaptat el text de la font bitmap a escala 1 (8x8 px) per a la càmera de 208px: distribució neta a la barra superior amb les 3 vides generals (16x16), cors de nivell, indicador de fase central `1-X`, comptador d'enemics `E:X` i temporitzador `02:00` sense cap solapament.
### 4.4. Mecàniques Fonamentals del Joc (Part Bàsica)
6. **(COMPLETAT) Blocs destructibles i power-ups:**
   - Rajoles `D` als nivells, destrucció per explosió, actualització de malla i aparició rotativa dels quatre power-ups: `Bomb Up`, `Fire Up`, cor i patins.
7. **Boss Final al Nivell 5:**
    - Dissenyar el cap final del món al nivell 5: sprite de grans dimensions de Pocket Bomberman, barra de vida / múltiples impactes i patrons d'atac propis.

### 4.5. Polish i Lliurament Final
9. **Motor d'Àudio (BGM i SFX):**
    - Integrar `miniaudio.h` per reproduir música chiptune retro a menús i nivells, juntament amb efectes de so per a bombes, explosions, salts, recollida d'objectes i mort.
10. **Preparació del Paquet de Lliurament:**
    - Redacció d'`info.txt`, organització de la carpeta `Binari/` i enregistrament del vídeo `demo.avi` d'1 minut.
