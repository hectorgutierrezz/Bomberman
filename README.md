# Pocket Bomberman

Projecte universitari de videojoc 2D inspirat en *Pocket Bomberman*, implementat
amb C++ i OpenGL.

## Funcionalitats

- Cinc nivells de plataformes amb enemics, bombes, explosions i portes.
- Menús, instruccions, crèdits, pausa, victòria i game over en català.
- Power-ups, blocs destructibles i blocs que es trenquen en trepitjar-los.
- Cap final al cinquè nivell, música, efectes de so i mode invulnerable.
- Proves automatitzades de funcionalitats del joc.

## Maneres d'executar el joc

Pots executar el joc com a aplicació nativa amb una finestra d'escriptori o jugar
des d'un navegador amb WebAssembly.

### 1. Aplicació nativa (Makefile)

En Linux cal disposar de `g++`, `make`, `pkg-config`, GLFW 3 i les llibreries de
desenvolupament d'OpenGL. El codi font de GLEW, GLM i SOIL s'inclou a `libs/`.

Des de l'arrel del repositori, compila i executa:

```sh
make
make run
```

També pots executar les mateixes ordres des del directori del projecte,
`2DGame/02-Bubble/`. `make` compila l'executable `bomberman` i `make run` l'obre
en una finestra. Per compilar i executar les proves:

```sh
make test
```

### 2. Navegador (WebAssembly)

Per compilar localment, cal tenir instal·lat
[Emscripten](https://emscripten.org/docs/getting_started/downloads.html). Des de
l'arrel del repositori:

```sh
make web
python3 -m http.server 8000 --directory 2DGame/02-Bubble/web-build
```

Obre <http://localhost:8000> en un navegador compatible amb WebGL 2. Cal servir
els fitxers amb un servidor HTTP; no obris `index.html` directament. La compilació
web utilitza el mateix codi i recursos del joc, amb Web Audio per al so.

### Publicació a GitHub Pages

El workflow de GitHub Actions compila i publica automàticament els canvis a la
branca `main`. Per activar-ho, ves a **Settings → Pages** i selecciona **GitHub
Actions** com a font de desplegament. Després d'activar-ho i pujar els canvis a
`main`, trobaràs l'adreça del joc a **Settings → Pages** o a l'execució del
workflow de la pestanya **Actions**. També pots iniciar el workflow manualment
des d'aquesta pestanya.

Les dues versions fan servir els mateixos controls:

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
