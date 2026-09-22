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
| **Saltar sobre les Bombes** | Bàsica | **Pendent** | 0% | Cal que les bombes funcionin com a plataformes sòlides sobre les quals Bomberman pugui recolzar-se i saltar. |
| **Sistema de Bombes i Explosions** | Bàsica | **Pendent** | 0% | Falta la classe `Bomb` (bombes estàtiques alineades a tiles) i `Explosion` (abast en creu, destrucció de blocs/enemics). |
| **Blocs Destructibles** | Bàsica | **Pendent** | 0% | Falta adaptar `TileMap` perquè suporti rajoles destructibles (arbustos/blocs) i regeneri el VBO en trencar-los. |
| **Objectes / Power-ups (4 tipus)** | Bàsica | **Pendent** | 0% | Cal implementar com a mínim: *Bomb Up*, *Fire Up*, *Cor/Invulnerabilitat* i *Porta de Sortida*. |
| **3 Enemics amb IA Diferent** | Bàsica | **Pendent** | 0% | Falta crear les classes d'enemics (patrulla, caçador, ràpid). |
| **Boss Final al Nivell 5** | Bàsica | **Pendent** | 0% | Falta dissenyar i programar el cap final del cinquè món. |
| **Temps Límit per Nivell** | Bàsica | **Pendent** | 0% | Falta el cronòmetre de compte enrere per cada nivell. |
| **Interfície Gràfica (HUD)** | Bàsica | **Pendent** | 0% | Falta mostrar: temps restant, vides, enemics restants, bombes màximes i abast de foc. |
| **Tecles de Truc (Cheats)** | Bàsica | **Pendent** | 0% | Falta implementar: `G` (God Mode), `K` (Kill enemies & obrir porta), `1`-`5` (saltar de nivell). |
| **Àudio (Música i Efectes SFX)** | Polish | **Pendent** | 0% | Falta reproductor de so per a música de fons i efectes d'explosió, salt, agafar objectes. |
| **Animacions i Game Feeling** | Polish | **Pendent** | 10% | S'han recopilat els sprites originals de GBC. Falta connectar les transicions i l'ajust fi de jugabilitat. |

---

## 2. Estat Actual dels Fitxers del Projecte

### Codi Font (`2DGame/02-Bubble/02-Bubble/`)
- **`Scene.h` / `Scene.cpp`**: 
  - Conté la màquina d'estats (`MAIN_MENU`, `INSTRUCTIONS`, `CREDITS`, `PLAYING`, `PAUSED`, `GAME_OVER`, `WIN`).
  - S'han implementat stubs segurs i s'inicia en estat `PLAYING`.
  - Falta carregar els sprites de menú i implementar `renderMainMenu()`, etc.
- **`Player.h` / `Player.cpp`**: 
  - Utilitza encara el sprite del dragó del Bubble Bobble (`bub.png`).
  - Té la física bàsica de moviment horitzontal i salt parabòlic.
- **`TileMap.h` / `TileMap.cpp`**: 
  - Carrega `level01.txt` i `blocks.png`.
  - És estàtic: no permet trencar rajoles ni actualitzar el mapa en temps real.
- **`Texture.cpp`**:
  - Alliberament de memòria corregit amb `SOIL_free_image_data`.
- **`main.cpp` / `Game.h` / `Game.cpp`**:
  - Bucle principal a 60 FPS i callbacks d'entrada amb GLFW i OpenGL.

### Recursos (`images/`, `shaders/`, `levels/`)
- **`images/Sprites/Original/`**: 
  - Gran col·lecció de sprites originals de *Pocket Bomberman* (stages món 1-5, personatges, enemics, menús color/b&w).
- **`levels/level01.txt`**: 
  - Mapa temporal de prova provinent del laboratori base.

---

## 3. Fases del Pla d'Implementació i Progrés

```
[████████░░░░░░░░░░░░] 20% Progrés Global Estimat

 Fase 1: Arquitectura de Pantalles i Menús (25%)
 Fase 2: Nivells, Mapes i Blocs Destructibles (0%)
 Fase 3: Bomberman, Moviment, Salt i Vides (15%)
 Fase 4: Sistema de Bombes i Explosions (0%)
 Fase 5: Objectes i Millores (Power-ups) (0%)
 Fase 6: Enemics (3 Tipus) i Boss Final (0%)
 Fase 7: HUD, Temps i Tecles de Truc (0%)
 Fase 8: Polish (Àudio i Game Feeling) (0%)
```

---

## 4. Pròxims Passos Immediats a Decidir

Per avançar de forma estructurada, hi ha dues opcions principals:

### 🎮 Opció 1: Core Gameplay (Recomanada)
> **Objectiu**: Transformar el personatge en Bomberman i tenir la mecànica estrella de posar bombes i saltar sobre elles.
1. **Canviar el Sprite de Player**: Substituir `bub.png` per la spritesheet de Bomberman (`images/Sprites/Original/Color/Characters/` o `images/Bomberman`) i afegir animacions (caminar, saltar, idle).
2. **Crear la classe `Bomb`**:
   - Posar bombes amb la tecla `Espacio` o `X`.
   - Les bombes queden estàtiques al centre de la rajola.
   - Bomberman pot saltar sobre les seves pròpies bombes com a bloc sòlid.
3. **Crear la classe `Explosion`**:
   - Temporitzador de 2-3 segons de la bomba.
   - Propagació en creu.

### 🖼️ Opció 2: Tancar la Fase 1 (Menús i Pantalles)
> **Objectiu**: Deixar les 4 pantalles del joc enllestides i navegables.
1. Carregar la textura de menús originals (`images/Sprites/Original/Color/Intro_Ending_Menus/Game Boy _ GBC - Pocket Bomberman - Miscellaneous - Menus (Color).png`).
2. Crear els sprites de Menú Principal, Instruccions i Crèdits.
3. Completar les funcions de renderitzat a `Scene.cpp`.

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
