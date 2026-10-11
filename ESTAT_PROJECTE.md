# Estat del Projecte: Pocket Bomberman 2D (FIB - VJ 2026/27)

Aquest document resumeix l'estat actual del desenvolupament del videojoc **Pocket Bomberman**, contrastat amb els requeriments de l'enunciat de la pràctica (**Part Bàsica: 6 punts** i **Polish: 4 punts**) i el pla d'implementació.

---

## 1. Taula de Progrés Global vs Requeriments del PDF

| Requeriment de l'Enunciat | Puntuació | Estat Actual | Progrés | Detalls / Tasques Pendents |
| :--- | :---: | :---: | :---: | :--- |
| **Entorn i Compilació en Linux** | - | **Completat** | 100% | `Makefile` operatiu, dependències locals (SOIL, GLEW, GLFW) i compatibilitat amb Linux sense necessitat de `sudo`. |
| **Estructura de 4 Pantalles** | Bàsica | **Completat** | 100% | 4 pantalles principals requerides (`MAIN_MENU`, `INSTRUCTIONS`, `CREDITS`, `PLAYING`) més estats auxiliars (`PAUSED`, `GAME_OVER`, `WIN`). Menú interactiu amb cursor animat, sprites originals de GBC extrets i netejats (`images/Menu/`), i renderitzat amb `BitmapText`. Tecla `ESC` gestionada correctament per a pausa i tornada al menú. |
| **Cinc Nivells i Porta de Sortida** | Bàsica | **Pendent** | 0% | Només existeix `level01.txt` del Bubble Bobble. Cal dissenyar els 5 nivells i la lògica de la porta de sortida. |
| **Moviment i Salt de Bomberman** | Bàsica | **Iniciat** | 20% | Moviment, salt i físiques de Bomberman funcionals amb càmera dinàmica. Pendent substituir `bub.png` pel sprite definitiu de Bomberman i comptador de 3 vides. |
| **Saltar sobre les Bombes** | Bàsica | **Completat** | 100% | Les bombes funcionen com a plataformes sòlides (`isSolidOnTop`). Bomberman es recolza i pot saltar des d'elles. |
| **Sistema de Bombes i Explosions** | Bàsica | **Completat** | 90% | Classe `Bomb` (alineada a la graella, animació 4 frames `Bomb On`, mecha ~2.2s) i classe `Explosion` (animació 6 frames `Boooooom`, propagació en creu 4 direccions, col·lisió amb murs, reacció en cadena i impacte a jugador). |
| **Blocs Destructibles** | Bàsica | **Pendent** | 0% | Falta adaptar `TileMap` perquè suporti rajoles destructibles (arbustos/blocs) i regeneri el VBO en trencar-los per les explosions. |
| **Objectes / Power-ups (4 tipus)** | Bàsica | **Pendent** | 0% | Cal implementar com a mínim: *Bomb Up*, *Fire Up*, *Cor/Invulnerabilitat* i *Porta de Sortida*. |
| **3 Enemics amb IA Diferent** | Bàsica | **Pendent** | 0% | Falta crear les classes d'enemics (patrulla, caçador, ràpid). |
| **Boss Final al Nivell 5** | Bàsica | **Pendent** | 0% | Falta dissenyar i programar el cap final del cinquè món. |
| **Temps Límit per Nivell** | Bàsica | **Completat** | 100% | Comptador enrere de 2 minuts (120s) per nivell. Si s'acaba el temps, es mostra la pantalla de *Time Over* amb Bomberman marejat i es pot reiniciar o anar al menú. |
| **Interfície Gràfica (HUD)** | Bàsica | **En Progrés** | 60% | Es mostren les vides del jugador i el temporitzador de compte enrere (`TIME 02:00`). |
| **Tecles de Truc (Cheats)** | Bàsica | **Pendent** | 0% | Falta implementar: `G` (God Mode), `K` (Kill enemies & obrir porta), `1`-`5` (saltar de nivell). Accés directe de desenvolupament 1-4 afegit al menú. |
| **Àudio (Música i Efectes SFX)** | Polish | **Pendent** | 0% | Falta reproductor de so per a música de fons i efectes d'explosió, salt, agafar objectes. |
| **Animacions i Game Feeling** | Polish | **En Progrés** | 45% | Sprites originals de menús de GBC, animació de bomba (`Bomb On`), explosió (`Boooooom`), cursor animat i pantalles de Game Over/Pausa autèntiques. |

---

## 2. Estat Actual dels Fitxers del Projecte

### Codi Font (`2DGame/02-Bubble/02-Bubble/`)
- **`Scene.h` / `Scene.cpp`**: 
  - Màquina d'estats completa: `MAIN_MENU`, `INSTRUCTIONS`, `CREDITS`, `PLAYING`, `PAUSED`, `GAME_OVER`, `WIN`.
  - Sistema de menús interactiu (`initMenuScreens`, `renderMainMenu`, `renderInstructions`, `renderCredits`, `renderPauseOverlay`, `renderGameOver`, `renderWin`).
  - Navegació amb fletxes o W/S, selecció amb Enter o Espai, i dreceres numèriques 1-4.
  - Pausa amb tecla `ESC` o `P`, represa amb `ESC`/`P` o retorn al menú amb `M`.
  - Detecció de mort (Game Over clàssic) i fi de temps (Time Over amb sprite de Bomberman amb ulls de rellotge).
  - Gestió de la col·lecció de bombes (`std::vector<Bomb*>`) i explosions (`std::vector<Explosion*>`).
  - Càmera dinàmica (lerp suau) seguint el jugador durant la partida.
- **`Game.h` / `Game.cpp`**:
  - Bucle principal i callbacks de GLFW.
  - Desacoblat el tancament brusc de la tecla `ESC`: ara `ESC` es delega a la `Scene` (per pausar, tornar enrere dels menús o sortir ordenadament mitjançant `Game::instance().exit()`).
- **`BitmapText.h` / `BitmapText.cpp`**:
  - Renderitzador de text pixel art retro basat en spritesheet (`images/hud_font.png`).
  - Millorat el mapeig de glifs (`glyphFor`) per admetre caràcters de puntuació: `.`, `!`, `/`, `(`, `)`, `>`, `<`, `[`, `]`.
- **`Bomb.h` / `Bomb.cpp`**:
  - Utilitza el spritesheet `images/Bomberman/Bomb On (52x56).png` (4 frames, 208x56 px) amb textura compartida a memòria.
  - Temporitzador de mecha de 2.2 segons abans de la detonació.
  - Permet recolzar-se i saltar a sobre (`collisionDown`, `isSolidOnTop`) i actuar com a mur lateral.
- **`Explosion.h` / `Explosion.cpp`**:
  - Utilitza el spritesheet `images/Bomberman/Boooooom (52x56).png` (6 frames, 312x56 px).
  - Propagació en creu en 4 direccions amb col·lisió contra rajoles de paret sòlida.
  - Detecció de col·lisions (`checkCollision`) amb el jugador i altres bombes per reacció en cadena.
- **`Player.h` / `Player.cpp`**: 
  - Moviment horitzontal, salt parabòlic i recolzament sobre bombes sòlides.
  - Mètode `hit()` per reiniciar posició al rebre l'impacte d'una explosió.
- **`TileMap.h` / `TileMap.cpp`**: 
  - Carrega `level01.txt` i `blocks.png`.
  - Mètodes de consulta de sòlids: `isSolidTile(pos)` i `isTileSolid(tileX, tileY)`.

### Recursos (`images/`, `shaders/`, `levels/`)
- **`images/Menu/` (Nous Sprites Extrets i Processats amb Canal Alfa RGBA)**:
  - `logo.png` (111x64): Logotip original oficial "Pocket Bomberman" amb l'explosió.
  - `cursor.png` (46x56): Cap de Bomberman usat com a cursor de selecció al menú principal.
  - `copyright.png` (159x15): Avís de copyright oficial Hudson Soft / Nintendo.
  - `hudson.png` (147x144): Emblema/abella de Hudson Soft per als crèdits.
  - `pause.png` (160x167): Bomberman dormint sota l'arbre de pomes amb el cartell "PAUSE".
  - `game_over.png` (141x55): Tipografia arcade 3D de "GAME OVER".
  - `time_over.png` (158x156): Bomberman marejat amb rellotges als ulls per fi de temps ("TIME OVER").
  - `world_clear.png` (144x24): Rètol daurat de "WORLD CLEAR!!".
  - `knights_celebrate.png` (176x44): Cavallers celebrant per a la pantalla de victòria.
- **`images/Bomberman/`**:
  - `Bomb On (52x56).png`: Spritesheet animat de la bomba activa.
  - `Boooooom (52x56).png`: Spritesheet animat de la detonació/explosió.
- **`images/Sprites/Original/`**: 
  - Col·lecció completa de sprites extrets de la ROM de *Pocket Bomberman* (stages món 1-5, personatges, enemics, menús).

---

## 3. Fases del Pla d'Implementació i Progrés

```
[██████████████████░░] 45% Progrés Global Estimat

 Fase 1: Arquitectura de Pantalles i Menús (100% - COMPLETAT)
 Fase 2: Nivells, Mapes i Blocs Destructibles (0%)
 Fase 3: Bomberman, Moviment, Salt i Vides (20%)
 Fase 4: Sistema de Bombes i Explosions (90%)
 Fase 5: Objectes i Millores (Power-ups) (0%)
 Fase 6: Enemics (3 Tipus) i Boss Final (0%)
 Fase 7: HUD, Temps i Tecles de Truc (50%)
 Fase 8: Polish (Àudio i Game Feeling) (30%)
```

---

## 4. Pròxims Passos Immediats a Decidir

1. **Blocs Destructibles al `TileMap` (Fase 2)**:
   - Adaptar `TileMap` per identificar blocs destructibles (rajoles que es poden trencar).
   - Quan una explosió toca una rajola destructible, canviar-la a buida (aire) i regenerar el VBO de la capa de tiles.
2. **Power-ups i Objectes (Fase 5)**:
   - *Bomb Up*: augmenta el nombre màxim de bombes simultànies.
   - *Fire Up*: augmenta l'abast de les flames de la bomba.
   - *Cor / Vida*: vida extra o immunitat temporal.
   - *Porta de sortida*: apareix o s'obre en complir les condicions del nivell.
3. **Enemics (Fase 6)**:
   - Crear classe base `Enemy` i 3 classes derivades amb diferents comportaments (patrulla simple, caçador/volador, enemic ràpid).

---

## 5. Guia Ràpida de Compilació i Execució (Linux)

Des del directori del projecte:
```bash
# Compilar tot el projecte
make

# Compilar i executar immediatament
make run

# Netejar fitxers de compilació
make clean
```
O directament des de `2DGame/02-Bubble`:
```bash
cd 2DGame/02-Bubble
make
./bomberman
```
