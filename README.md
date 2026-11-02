# Pocket Bomberman

Projecte universitari de videojoc 2D inspirat en *Pocket Bomberman*, implementat
amb C++ i OpenGL.

## Funcionalitats

- Cinc nivells de plataformes amb enemics, bombes, explosions i portes.
- Menús, instruccions, crèdits, pausa, victòria i game over en català.
- Power-ups, blocs destructibles i blocs que es trenquen en trepitjar-los.
- Cap final al cinquè nivell, música, efectes de so i mode invulnerable.
- Proves automatitzades de funcionalitats del joc.

## Requisits

En Linux cal disposar de `g++`, `make`, `pkg-config`, GLFW 3 i les llibreries de
desenvolupament d'OpenGL. El codi font de GLEW, GLM i SOIL s'inclou a `libs/`.

## Compilar i executar

Des de l'arrel del repositori:

```sh
make -C 2DGame/02-Bubble
make -C 2DGame/02-Bubble run
```

Per executar les proves:

```sh
make -C 2DGame/02-Bubble test
```

## Controls

- **Fletxes / WASD:** moure's i navegar pels menús.
- **Espai:** saltar o confirmar una opció.
- **B:** col·locar una bomba.
- **P / Esc:** pausar o reprendre la partida.
- **G:** activar o desactivar el mode invulnerable.

Els controls addicionals i les instruccions del joc també són accessibles des
del menú principal.

## Estructura

- `2DGame/02-Bubble/02-Bubble/`: codi font principal.
- `2DGame/02-Bubble/02-Bubble/levels/`: mapes dels nivells.
- `2DGame/02-Bubble/02-Bubble/images/` i `sound/`: recursos del joc.
- `tests/`: proves automatitzades.
- [`ESTAT_PROJECTE.md`](ESTAT_PROJECTE.md): estat i detalls del projecte.
