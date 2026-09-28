# Estat del Projecte: Pocket Bomberman 2D (FIB - VJ 2026/27)

Aquest document resumeix l'estat actual del desenvolupament del videojoc **Pocket Bomberman**, contrastat amb els requeriments de l'enunciat de la pràctica (**Part Bàsica: 6 punts** i **Polish: 4 punts**) i el pla d'implementació.

---

## 1. Taula de Progrés Global vs Requeriments del PDF

| Requeriment de l'Enunciat | Puntuació | Estat Actual | Progrés | Detalls / Tasques Pendents |
| :--- | :---: | :---: | :---: | :--- |
| **Entorn i Compilació en Linux** | - | **Completat** | 100% | `Makefile` operatiu, dependències locals (SOIL, GLEW, GLFW) i compatibilitat amb Linux sense necessitat de `sudo`. |
| **Estructura de 4 Pantalles** | Bàsica | **Iniciat** | 25% | `enum GameState` i navegació de tecles definides a `Scene`. Falta carregar textures i dibuixar els menús, instruccions, crèdits i pausa. |
| **Cinc Nivells i Porta de Sortida** | Bàsica | **Pendent** | 0% | Només existeix `level01.txt` del Bubble Bobble. Cal dissenyar els 5 nivells i la lògica de la porta de sortida. |
| **Moviment i Salt de Bomberman** | Bàsica | **Iniciat** | 15% | Moviment i salt heretat del Bubble Bobble (`bub.png`). Cal canviar el personatge a Bomberman, animacions pròpies i 3 vides. |
| **Saltar sobre les Bombes** | Bàsica | **Completat** | 100% | Les bombes funcionen com a plataformes sòlides (`isSolidOnTop`). Bomberman es recolza i pot saltar des d'elles. |
| **Sistema de Bombes i Explosions** | Bàsica | **Completat** | 90% | Classe `Bomb` (alineada a la graella, animació 4 frames `Bomb On`, mecha ~2.2s) i classe `Explosion` (animació 6 frames `Boooooom`, propagació en creu 4 direccions, col·lisió amb murs, reacció en cadena i impacte a jugador). |
| **Blocs Destructibles** | Bàsica | **Pendent** | 0% | Falta adaptar `TileMap` perquè suporti rajoles destructibles (arbustos/blocs) i regeneri el VBO en trencar-los per les explosions. |
| **Objectes / Power-ups (4 tipus)** | Bàsica | **Pendent** | 0% | Cal implementar com a mínim: *Bomb Up*, *Fire Up*, *Cor/Invulnerabilitat* i *Porta de Sortida*. |
| **3 Enemics amb IA Diferent** | Bàsica | **Pendent** | 0% | Falta crear les classes d'enemics (patrulla, caçador, ràpid). |
| **Boss Final al Nivell 5** | Bàsica | **Pendent** | 0% | Falta dissenyar i programar el cap final del cinquè món. |
| **Temps Límit per Nivell** | Bàsica | **Pendent** | 0% | Falta el cronòmetre de compte enrere per cada nivell. |
| **Interfície Gràfica (HUD)** | Bàsica | **Pendent** | 0% | Falta mostrar: temps restant, vides, enemics restants, bombes màximes i abast de foc. |
| **Tecles de Truc (Cheats)** | Bàsica | **Pendent** | 0% | Falta implementar: `G` (God Mode), `K` (Kill enemies & obrir porta), `1`-`5` (saltar de nivell). |
| **Àudio (Música i Efectes SFX)** | Polish | **Pendent** | 0% | Falta reproductor de so per a música de fons i efectes d'explosió, salt, agafar objectes. |
| **Animacions i Game Feeling** | Polish | **Iniciat** | 30% | Sprites de Bomberman, animació de bomba (`Bomb On`) i explosió (`Boooooom`) completament funcionals. |

---

## 2. Estat Actual dels Fitxers del Projecte

### Codi Font (`2DGame/02-Bubble/02-Bubble/`)
- **`Scene.h` / `Scene.cpp`**: 
  - Màquina d'estats (`MAIN_MENU`, `INSTRUCTIONS`, `CREDITS`, `PLAYING`, `PAUSED`, `GAME_OVER`, `WIN`).
  - Inicia directament en `PLAYING` per a desenvolupament i proves ràpides.
  - Gestiona la col·lecció de bombes (`std::vector<Bomb*>`) i explosions (`std::vector<Explosion*>`).
  - Col·locació de bombes amb `Espai` o `X` al davant del personatge (segons la direcció que mira), amb comprovació d'obstacles per si hi ha paret.
  - Implementa reacció en cadena d'explosions (detonació immediata si una flama toca una altra bomba) i dany al jugador.
- **`Bomb.h` / `Bomb.cpp`**:
  - Utilitza el spritesheet `images/Bomberman/Bomb On (52x56).png` (4 frames, 208x56 px) amb textura compartida a memòria.
  - Mida i offset calculats per centrar la bomba al bloc de 32x32 i recolzar-la a terra.
  - Temporitzador de mecha de 2.2 segons abans de la detonació.
  - Sistema de solidesa física dinàmica: permeable mentre el jugador hi és a sobre o se'n solapa al posar-la, i completament sòlida en separar-se'n.
  - Permet recolzar-se i saltar a sobre (`collisionDown`, `isSolidOnTop`) i actuar com a mur lateral (`collisionLeft`, `collisionRight`).
- **`Explosion.h` / `Explosion.cpp`**:
  - Utilitza el spritesheet `images/Bomberman/Boooooom (52x56).png` (6 frames, 312x56 px).
  - Propagació en creu en 4 direccions (amunt, avall, esquerra, dreta) a partir del centre de la bomba.
  - Detecció d'obstacles mitjançant `TileMap::isSolidTile` per aturar la flama contra parets sòlides.
  - Detecció de col·lisions (`checkCollision`) amb el jugador i altres bombes per reacció en cadena.
- **`Player.h` / `Player.cpp`**: 
  - Utilitza el sprite de Bomberman (`images/Sprites/Original/Color/Characters/bomber.png`).
  - Moviment horitzontal, salt parabòlic i recolzament sobre bombes sòlides.
  - Mètode `hit()` per reiniciar posició al rebre l'impacte d'una explosió.
- **`Sprite.h` / `Sprite.cpp`**:
  - Corregida inicialització de `texCoordDispl`, `currentAnimation`, `currentKeyframe` i `timeAnimation` per evitar comportament indefinit.
- **`TileMap.h` / `TileMap.cpp`**: 
  - Carrega `level01.txt` i `blocks.png`.
  - S'hi han afegit mètodes de consulta de sòlids: `isSolidTile(pos)` i `isTileSolid(tileX, tileY)`.
- **`Texture.cpp`**:
  - Alliberament de memòria corregit amb `SOIL_free_image_data`.
- **`main.cpp` / `Game.h` / `Game.cpp`**:
  - Bucle principal a 60 FPS i callbacks d'entrada amb GLFW i OpenGL.

### Recursos (`images/`, `shaders/`, `levels/`)
- **`images/Bomberman/`**:
  - `Bomb On (52x56).png`: Spritesheet animat de la bomba activa.
  - `Boooooom (52x56).png`: Spritesheet animat de la detonació/explosió.
- **`images/Sprites/Original/`**: 
  - Col·lecció de sprites originals de *Pocket Bomberman* (stages món 1-5, personatges, enemics, menús).
- **`levels/level01.txt`**: 
  - Mapa temporal de prova provinent del laboratori base.

---

## 3. Fases del Pla d'Implementació i Progrés

```
[██████████████░░░░░░] 35% Progrés Global Estimat

 Fase 1: Arquitectura de Pantalles i Menús (25%)
 Fase 2: Nivells, Mapes i Blocs Destructibles (0%)
 Fase 3: Bomberman, Moviment, Salt i Vides (30%)
 Fase 4: Sistema de Bombes i Explosions (90%)
 Fase 5: Objectes i Millores (Power-ups) (0%)
 Fase 6: Enemics (3 Tipus) i Boss Final (0%)
 Fase 7: HUD, Temps i Tecles de Truc (0%)
 Fase 8: Polish (Àudio i Game Feeling) (15%)
```

---

## 4. Pròxims Passos Immediats a Decidir

1. **Blocs Destructibles (Fase 2)**:
   - Adaptar `TileMap` perquè les rajoles destructibles (ex: tipus '2' o arbustos) es puguin destruir quan una explosió les toqui.
   - Regenerar el VBO de `TileMap` en destruir un bloc.
2. **Power-ups i Objectes (Fase 5)**:
   - *Bomb Up* (augmentar límit de bombes simultànies).
   - *Fire Up* (augmentar el rang de la creu de la classe `Explosion`).
   - *Porta de sortida*.
3. **Enemics (Fase 6)**:
   - Crear la jerarquia d'enemics amb moviment i detecció de danys per explosió.

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
