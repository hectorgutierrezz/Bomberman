# Estat del Projecte: Pocket Bomberman 2D (FIB - VJ 2026/27)

Aquest document detalla l'estat actual del desenvolupament del videojoc **Pocket Bomberman** desenvolupat a `2DGame/`, contrastat punt per punt amb els requeriments de la guia de la pràctica (**FIB - Videojocs Curs 2026/27 Q1: Laboratori Exercici 2D**), incorporant l'estat real del codi a la branca `development` i el full de ruta amb les noves tasques i millores pendents.

> **Actualització (31/10/2026):**
> - **Cap Final GBC Autèntic i Nivell 5 Completat:**
>   - **Sprite i textura oficial de 32x32 px:** Integrada la gòrgola demoníaca de *Pocket Bomberman* GBC a [`images/boss.png`](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/images/boss.png) amb filtratge `GL_NEAREST` pixel-perfect i `GL_CLAMP_TO_EDGE`, mantenint les proporcions idònies respecte a Bomberman (16x16) i a l'arena de combat.
>   - **Arena de combat compacta i neta (24x12 rajoles):** Reestructurat [`levels/level05.txt`](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/levels/level05.txt) com una arena de combat neta (384x192 px) sense blocs destructibles `D`, amb plataformes sòlides equilibrades per al duel.
>   - **Mecàniques de combat i cicles d'IA:**
>     - Persecució 2D suau amb aleteig i levitació (`speedX = 0.65f`, `speedY = 0.45f`, significativament més lent que Bomberman a 1.35f per permetre maniobres).
>     - Cicle de combat calibrat: 3s de persecució (`BOSS_CHASE`) $\to$ atac de raig (`BOSS_SHOOT`) o invocació (`BOSS_SUMMON`) $\to$ 1s de pausa/descans (`BOSS_PAUSE`).
>     - **Tret de raigs energètics dirigits:** Dispara projectils d'energia dirigits directament cap a la posició del jugador (a mig camí de la persecució i en finalitzar-la), amb detecció de col·lisió contra parets i dany a Bomberman.
>     - **Invocació ocasional d'esbirros:** Cada 15 segons, el cap pot invocar enemics base (`ENEMY_HORIZONTAL` o `ENEMY_VERTICAL`, limitats a un màxim de 2 simultanis).
>     - **Seguretat d'invulnerabilitat:** Quan el cap rep dany per una bomba i parpelleja invulnerable (1,5s), el contacte directe amb Bomberman **no** li resta vida, protegint el jugador de morts injustes immediatament després d'encertar un atac.
>     - **HUD i Mort:** Indicador dinàmic de vides del cap (`B3` $\to$ `B2` $\to$ `B1` $\to$ `00`), múltiples explosions consecutives en ser derrotat i obertura de la porta cap a la victòria final (`WIN`).
> - **Actualització anterior (28/10/2026):** Fons temàtics autèntics per als 5 mons, il·lustracions pixel-art oficials, ajust mil·limètric de càmera, motor d'àudio integrat amb miniaudio.h (BGM/SFX).
> - **Verificació de compilació:** 0 errors i 0 avisos a Linux (`make -C 2DGame/02-Bubble all`).

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
| **9** | **Quatre objectes (com a mínim ‘Bomb Up’ i ‘Fire Up’)** | **Completat** | 100% | Les entitats recollibles són `Bomb Up`, `Fire Up`, cor i patins. Cada bloc trencat genera un tipus en rotació; recollir-lo aplica l'efecte al jugador. **Escala reajustada a 16x16 px**, encaixant perfectament a la casella del tile i proporcional a Bomberman i als enemics. |
| **10** | **Temps límit per passar-se cada nivell** | **Completat** | 100% | Temporitzador enrere de 120s (2 min) per nivell. Si s'esgota, pantalla de *Time Over* amb sprite autèntic de Bomberman marejat. |
| **11** | **Boss final al nivell 5** | **Completat** | 100% | **Dissenyat i implementat el Cap Final autèntic de Pocket Bomberman GBC a [`Boss.h`](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Boss.h) / [`Boss.cpp`](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Boss.cpp) i l'arena de combat compacta de 24x12 a [`level05.txt`](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/levels/level05.txt).**<br>- **Sprite fidel de 32x32:** Cap gòrgola demoníaca oficial de Game Boy Color a `images/boss.png` amb animacions d'aleteig, atac i dany/derrota.<br>- **3 vides i invulnerabilitat segura:** Rep dany per explosions de bombes amb parpelleig; mentre és invulnerable, el contacte directe no danya Bomberman.<br>- **Persecució dinàmica i pauses:** Moviment aeri suau (3s de persecució i 1s de pausa) a velocitat inferior al jugador per permetre maniobres i trampes de bombes.<br>- **Atacs a distància i invocació:** Dispara raigs energètics dirigits cap a Bomberman i invoca esbirros bàsics de suport cada 15s.<br>- **Integració a l'HUD:** Indicador dinàmic de vides del cap (`B3` $\to$ `B2` $\to$ `B1` $\to$ `00`).<br>- **Transició i victòria:** En ser derrotat genera una seqüència d'explosions consecutives i obre la porta final cap a la pantalla de victòria (`WIN`). |
| **12** | **Interfície gràfica (HUD) amb dades mínimes requerides** | **Completat** | 100% | **Barra inferior fidel a Pocket Bomberman de Game Boy Color:** Franja sòlida pastel/rosada a la part inferior (`CAM_VIEW_W = 208px`) amb vora fosca superior.<br>- **Temps restant:** Icona de cronòmetre (8x8) + `MM:SS` (vermell d'alerta si <= 30s).<br>- **Enemics restants / Porta:** Icona de porta + comptador `02` (verd quan s'obre la porta).<br>- **Estadi / Target:** Indicador `T1`..`T5` idèntic a la ROM original.<br>- **Mida d'explosions:** Icona de foc + radi actual de bomba (power-up *Fire Up*).<br>- **Bombes simultànies:** Icona de bomba GBC + capacitat màxima de bombes (power-up *Bomb Up*).<br>- **Vides del nivell (cors):** Icona de cor pixel-art + comptador de vides de nivell (sempre visible: `1` de base, augmentable amb power-ups de cor).<br>- **Vides globals:** Icona oficial de la cara de Bomberman + comptador de vides globals (3, 2, 1) situada a l'extrem dret fidel a la ROM de GBC. |
| **13** | **Tecles de truc (Cheats de correcció)** | **Completat** | 95% | La pantalla **TRUCS**, separada de les instruccions i accessible des del menú principal, mostra els codis disponibles durant la partida: **`G`** activa/desactiva la invulnerabilitat; **`K`** elimina els enemics i obre la porta; **`H`** afegeix un cor; **`B`** augmenta la capacitat de bombes; **`F`** augmenta el radi de l'explosió; **`S`** augmenta la velocitat; **`N`** passa al nivell següent; **`1`–`5`** seleccionen directament un nivell. (`P` i `ESC` posen el joc en pausa.) |

---

### 1.2. Polish (4 punts)

| Aspecte de Polish | Estat Actual | Progrés | Descripció i Estat |
| :--- | :---: | :---: | :--- |
| **So (Música de fons i efectes especials)** | **Completat** | 100% | Incorporat el reproductor d'àudio `miniaudio.h` mitjançant [`SoundManager`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/SoundManager.h). Música integrada: menú de títol, intro de món, nivells bosc (1-4), combat boss (5), continuació/try again, victòria/ending, stage clear, time over i game over. Efectes sonors (SFX): posar bomba, explosió, agafar power-up, mort de Bomberman, pausa, passos horitzontals i salt vertical. |
| **Interactivitat i Animacions de totes les entitats** | **Completat** | 80% | - **Bomberman:** Animacions de moviment, salt, caiguda, parpelleig, reacció de dolor [`HURT`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp#L97-L99) i seqüència oficial de mort de 7 frames [`DIE`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/Player.cpp#L101-L109).<br>- **Enemics:** Tots 3 tipus compten amb cicle de moviment fluid (`MOVE`) i animació de mort (`DIE`).<br>- **Bombes i explosions:** Animació de metxa (4 frames) i explosió en creu fluida (6 frames).<br>- **Menús i UI:** Cursor animat, pantalles de pausa, game over, time over i celebració de victòria. Avisos de god mode «MODE DEU ON/OFF» a mida reduïda perquè càpiguen a la pantalla. |
| **Game feeling i Disseny Visual (Fidelitat, fons i feedback)** | **Completat** | 90% | Plataformes sobre bombes, caiguda amb gravetat realista (`stepFall`), càmera suau, alerta de temps crític en vermell, porta reactiva a la neteja d'enemics.<br>- **Fons temàtics autèntics per als 5 mons:** Integrats patrons de textura *seamless* fidels a la ROM de GBC (`images/Map/backgrounds/bg_world1.png` a `bg_world5.png`) renderitzats al darrere del TileMap i adaptats dinàmicament a la mida de cada nivell, eliminant el fons negre.<br>- **Il·lustracions pixel-art oficials:** Afegides a la pantalla d'inici de nivell (`STAGE_INTRO`) les escenes il·lustrades de 128x96 px per a cadascun dels 5 mons (`images/Map/illustrations/world1_art.png` a `world5_art.png`). |

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
- [Boss.h](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Boss.h) / [Boss.cpp](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Boss.cpp):
  - Entitat del Cap Final (Gòrgola Demoníaca de 32x32 px) del Nivell 5.
  - Màquina d'estats: `BOSS_IDLE`, `BOSS_CHASE`, `BOSS_PAUSE`, `BOSS_SHOOT`, `BOSS_SUMMON`, `BOSS_HURT`, `BOSS_DYING`.
  - Moviment flotant i persecució suau cap a Bomberman, amb orientació horitzontal i aleteig.
  - Tret de raigs energètics dirigits cap al jugador (`BossRay`) amb col·lisió contra el mapa i dany al jugador.
  - Invocació periòdica d'esbirros (`ENEMY_HORIZONTAL` o `ENEMY_VERTICAL`) cada 15s.
  - Gestió de 3 vides, parpelleig d'invulnerabilitat (sense dany per contacte durant aquest període) i seqüència final d'explosions consecutives.
- [SoundManager.h](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/SoundManager.h) / [SoundManager.cpp](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/SoundManager.cpp):
  - Motor d'àudio autònom basat en `miniaudio.h` per a la reproducció simultània de BGM i SFX amb suport Linux/WSL2 (PulseAudio) i Windows.

### Recursos i Nivells
- **`levels/`**: Fitxers [level01.txt](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/levels/level01.txt) fins a [level05.txt](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/levels/level05.txt).
- **`images/boss.png`**: Textura d'alta fidelitat amb els sprites de la gòrgola demoníaca de 32x32 i els projectils/raigs de 16x16.
- **`images/Menu/`**: Sprites oficials transparents (RGBA): `logo.png`, `cursor.png`, `copyright.png`, `hudson.png`, `pause.png`, `game_over.png`, `time_over.png`, `world_clear.png`, `knights_celebrate.png`.
- **`images/Sprites/Original/`**: Rips complets de la ROM original de Pocket Bomberman (tilesets de mons 1-5, personatges, enemics i bosses).
- **`images/door.png`**: Sprite de la porta oberta i tancada.
- **`images/powerups.png`**: Sprites dels quatre objectes recollibles.

---

## 3. Resum Gràfic de Progrés Global

```
[████████████████████] 98% Progrés Total del Projecte (estimació actualitzada)

  Part Bàsica (6 punts):
  - Pantalles del joc (Menú, Jugar, Inst., Trucs, Crèdits): 100% [██████████]
  - Moviment, salt i càmera de Bomberman            : 100% [██████████]
  - Bombes a l'aire i salt sobre bombes             : 100% [██████████]
  - Temps límit de nivell (120s)                    : 100% [██████████]
  - 3 Enemics amb IA diferenciada                   : 100% [██████████]
  - Tecles de truc / Cheats (G, K, 1-5, H, B, F, S, N): 95% [██████████]
  - Sistema de vides i pantalla TRY_AGAIN           : 100% [██████████]
  - Cinc nivells i porta condicionada a enemics     : 100% [██████████]
  - HUD complet estil GBC (Temps, Porta, T, F, B, Vides): 100% [██████████]
  - 4 Objectes (Bomb Up, Fire Up, Cor, Patins)       : 100% [██████████]
  - Blocs destructibles als mapes                   : 100% [██████████]
  - Boss final al nivell 5                          : 100% [██████████]

  Polish (4 punts):
  - Tileset oficial GBC i estètica gràfica retro    : 100% [██████████]
  - Fons temàtics autèntics i il·lustracions de món : 100% [██████████]
  - Interactivitat i animacions d'entitats / menús   :  90% [█████████░]
  - Game Feeling i fidelitat a Pocket Bomberman      :  95% [█████████░]
  - So (Música BGM i Efectes SFX integrats)         : 100% [██████████]
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
9. **(COMPLETAT) Interfície gràfica inferior autèntica de Game Boy Color:**
   - Implementada la barra inferior retro de Pocket Bomberman (`CAM_VIEW_W = 208px`, alçada 14px) amb fons pastís rosat `#F8A8B8` i vora fosca superior.
   - Inclou els 5 requeriments oficials del PDF distribuïts en una sola fila neta:
     1. Cronòmetre pixel-art + temps restant (`MM:SS`, alerta vermella $\le$ 30s).
     2. Icona de porta + comptador d'enemics `02`, `01` (verd en obrir-se la porta).
     3. Indicador de fase `T1` a `T5` fidel a la ROM original.
     4. Icona de foc + radi d'explosió (power-up *Fire Up*).
     5. Icona de bomba + bombes simultànies màximes (power-up *Bomb Up*).
     6. Icona de la cara de Bomberman + vides restants globals (més cors de nivell).
### 4.4. Mecàniques Fonamentals del Joc (Part Bàsica)
6. **(COMPLETAT) Blocs destructibles i power-ups:**
   - Rajoles `D` als nivells, destrucció per explosió, actualització de malla i aparició rotativa dels quatre power-ups: `Bomb Up`, `Fire Up`, cor i patins. Mida reestructurada a escala nativa 16x16 px amb flotació suau de $\pm 1.5$ px i col·lisió precisa d'un bloc.
7. **(COMPLETAT) Boss Final al Nivell 5:**
    - Dissenyat i implementat el Cap Final autèntic de Game Boy Color a [`Boss.h`](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Boss.h) / [`Boss.cpp`](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/Boss.cpp): 3 vides, sprite oficial de gòrgola demoníaca de 32x32 px a [`images/boss.png`](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/images/boss.png), màquina d'estats (`BOSS_CHASE`, `BOSS_PAUSE`, `BOSS_SHOOT`, `BOSS_SUMMON`, `BOSS_HURT`, `BOSS_DYING`), tret de raigs energètics dirigits cap al jugador, convocatòria periòdica d'esbirros bàsics (cada 15s, màx. 2), col·lisions amb explosions, no-contacte letal durant la invulnerabilitat (1,5s de seguretat per al jugador) i obertura de la porta final de victòria en morir. Arena redissenyada a mida compacta de 24x12 rajoles (384x192 px) lliure de blocs destructibles a [`levels/level05.txt`](file:///home/toret/Desktop/Stuff/Bomberman/2DGame/02-Bubble/02-Bubble/levels/level05.txt).

### 4.5. Polish i Lliurament Final
9. **(COMPLETAT) Motor d'Àudio Integral (BGM i SFX):**
    - Integració de la llibreria d'àudio `miniaudio.h` (v0.11+) mitjançant la classe [`SoundManager`](file:///c:/Users/jan10/Desktop/VJ/2D/Bomberman/2DGame/02-Bubble/02-Bubble/SoundManager.h).
    - Compatibilitat tant amb Windows (DirectSound/WASAPI) com amb Linux/WSL2 (suport PulseAudio nadiu apuntant al servidor de so `/mnt/wslg/PulseServer` amb paquets `libpulse0` i `libasound2-plugins`).
    - **Banda sonora oficial completa (9 temes):** Menú principal (`MUSIC_TITLE`), transició de món (`MUSIC_WORLD_INTRO`), bosc nivells 1-4 (`MUSIC_STAGE_FOREST`), combat de boss (`MUSIC_BOSS_BATTLE`), continuació (`MUSIC_CONTINUE`), victòria (`MUSIC_ENDING`), nivell superat (`MUSIC_STAGE_CLEAR`), temps exhaurit (`MUSIC_TIME_OVER`) i derrota (`MUSIC_GAME_OVER`).
    - **Efectes sonors (7 SFX balancejats):** Plantar bomba (`SFX_PLACE_BOMB`), explosió reforçada (`SFX_EXPLOSION`, volum balancejat al 220%), agafar power-up (`SFX_PICKUP`), mort de Bomberman (`SFX_DEATH`), pausa (`SFX_PAUSE`), passos atenuats (`SFX_STEP_H`, normalitzats al 22% de pic amb `scratch/adjust_steps.py`) i salt vertical (`SFX_STEP_V`, normalitzat al 40%).
10. **(COMPLETAT) Fons temàtics autèntics i il·lustracions retro per als 5 mons:**
    - Generació de textures *seamless* de 32x32 px basades en els patrons de fons originals de Game Boy Color (`images/Map/backgrounds/bg_world1.png` a `bg_world5.png`), renderitzats al darrere del TileMap i adaptats a la mida de cada nivell amb `GL_REPEAT` i `GL_NEAREST`.
    - Integració de les il·lustracions oficials de 128x96 px (`world1_art.png` a `world5_art.png`) escalades a la pantalla de transició `STAGE_INTRO`.
    - Ajust de precisió als límits de càmera (`camX >= SCREEN_X`, `camY >= SCREEN_Y`), evitant que es vegi cap franja negra exterior al voltant del mapa.
11. **Preparació del Paquet de Lliurament:**
    - Redacció d'`info.txt`, organització de la carpeta `Binari/` i enregistrament del vídeo `demo.avi` d'1 minut.
