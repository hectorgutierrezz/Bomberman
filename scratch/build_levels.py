import os

W = 36
H = 28

def create_base(top_tile, sub_tile):
    grid = [[' ' for _ in range(W)] for _ in range(H)]
    # Ceiling
    for y in range(2):
        for x in range(W):
            grid[y][x] = '8'
    # Borders
    for y in range(H):
        grid[y][0] = '8'
        grid[y][1] = '8'
        grid[y][W-2] = '8'
        grid[y][W-1] = '8'
    # Floor
    for x in range(W):
        grid[26][x] = top_tile
        grid[27][x] = sub_tile
    return grid

def grid_to_txt(grid):
    lines = ['TILEMAP', '36 28', '16 16', 'images/Map/world_tileset.png', '16 16']
    for row in grid:
        lines.append(''.join(row))
    return '\n'.join(lines) + '\n'

def build_level01():
    # World 1: Grassland
    # Door: top sanctuary (17, 5) resting on row 7
    # Player spawn: (3, 25)
    # Enemies: (11, 25) [horizontal, ground], (22, 17) [horizontal, row 18]
    grid = create_base('2', 'H')

    # Row 24: Stepping platforms (h=2 above floor)
    for x in range(5, 9): grid[24][x] = 'A'
    for x in range(27, 31): grid[24][x] = 'A'

    # Row 22: Tier 1 ledges
    for x in range(9, 17): grid[22][x] = '2'
    for x in range(19, 27): grid[22][x] = '2'

    # Row 20: Stepping bridges
    for x in range(5, 9): grid[20][x] = 'A'
    for x in range(27, 31): grid[20][x] = 'A'

    # Row 18: Tier 2 main walkway
    for x in range(3, 11): grid[18][x] = '2'
    for x in range(14, 23): grid[18][x] = 'A' # Central wooden bridge
    for x in range(25, 33): grid[18][x] = '2'

    # Row 16: Stepping bridges
    for x in range(11, 15): grid[16][x] = 'A'
    for x in range(21, 25): grid[16][x] = 'A'

    # Row 14: Tier 3 ledges
    for x in range(5, 12): grid[14][x] = '2'
    for x in range(15, 21): grid[14][x] = 'A'
    for x in range(24, 31): grid[14][x] = '2'

    # Row 12: Stepping bridges
    for x in range(11, 15): grid[12][x] = 'A'
    for x in range(21, 25): grid[12][x] = 'A'

    # Row 10: Tier 4 ledges
    for x in range(7, 13): grid[10][x] = '2'
    for x in range(23, 29): grid[10][x] = '2'

    # Row 8: Sanctuary approach bridge
    for x in range(12, 24): grid[8][x] = 'A'

    # Row 7: Top Sanctuary (Door at col 17, 18)
    for x in range(14, 22): grid[7][x] = '8'

    # Blocs destructibles ('D')
    grid[17][6] = 'D'; grid[17][29] = 'D'
    grid[21][13] = 'D'; grid[21][23] = 'D'
    grid[25][6] = 'D'; grid[25][18] = 'D'

    return grid

def build_level02():
    # World 2: Desert / Ancient Dunes
    # Door: ground floor right (30, 24) resting on row 26
    # Player spawn: (3, 25)
    # Enemies: (15, 21) [horizontal, row 22], (5, 14) [vertical], (29, 14) [vertical]
    grid = create_base('3', 'J')

    # Stepped pyramid climbing from left and descending to right
    # Row 24: Stepping ledges
    for x in range(6, 11): grid[24][x] = 'A'
    for x in range(23, 28): grid[24][x] = 'A'

    # Row 22: Tier 1 terraces
    for x in range(10, 18): grid[22][x] = '3'
    for x in range(20, 26): grid[22][x] = '3'

    # Row 20: Stepping bridges
    for x in range(7, 11): grid[20][x] = 'A'
    for x in range(24, 28): grid[20][x] = 'A'

    # Row 18: Tier 2 pyramid platforms
    for x in range(9, 16): grid[18][x] = '3'
    for x in range(18, 25): grid[18][x] = '3'

    # Row 16: Central suspension bridge
    for x in range(13, 23): grid[16][x] = 'A'

    # Row 14: Upper pyramid ledges
    for x in range(8, 14): grid[14][x] = '3'
    for x in range(22, 28): grid[14][x] = '3'

    # Row 12: High sun terraces
    for x in range(10, 16): grid[12][x] = 'A'
    for x in range(20, 26): grid[12][x] = 'A'

    # Row 10: Top pyramid altar (treasure / viewpoint)
    for x in range(14, 22): grid[10][x] = '3'

    # Decorative pillars (columns 13, 22) below row 10
    grid[11][14] = '8'; grid[11][21] = '8'

    # Blocs destructibles ('D')
    grid[17][11] = 'D'; grid[17][21] = 'D'
    grid[21][13] = 'D'; grid[21][23] = 'D'
    grid[25][8] = 'D'; grid[25][20] = 'D'

    return grid

def build_level03():
    # World 3: Golden Temple
    # Door: top sanctuary (17, 5) resting on row 7
    # Player spawn: (3, 25)
    # Enemies: (8, 25) [walker, floor], (23, 21) [walker, row 22],
    #          (28, 14) [vertical flier], (17, 11) [static balloon]
    grid = create_base('4', 'K')

    # Stepping stones row 24
    for x in range(4, 9): grid[24][x] = 'A'
    for x in range(26, 31): grid[24][x] = 'A'

    # Tier 1 row 22
    for x in range(8, 16): grid[22][x] = '4'
    for x in range(20, 28): grid[22][x] = '4'

    # Tier 2 stepping row 20
    for x in range(5, 9): grid[20][x] = 'A'
    for x in range(26, 30): grid[20][x] = 'A'

    # Tier 2 row 18
    for x in range(4, 11): grid[18][x] = '4'
    for x in range(13, 23): grid[18][x] = 'A' # Temple bridge
    for x in range(24, 32): grid[18][x] = '4'

    # Tier 3 stepping row 16
    for x in range(9, 13): grid[16][x] = 'A'
    for x in range(22, 26): grid[16][x] = 'A'

    # Tier 3 row 14
    for x in range(6, 12): grid[14][x] = '4'
    for x in range(23, 27): grid[14][x] = '4'

    # Static guardian platform row 12 (Guardian sits at 17, 11, resting on row 12)
    for x in range(15, 21): grid[12][x] = '4'

    # Tier 4 row 10
    for x in range(8, 14): grid[10][x] = 'A'
    for x in range(21, 27): grid[10][x] = 'A'

    # Row 8: Sanctuary approach
    for x in range(12, 24): grid[8][x] = 'A'

    # Row 7: Top Golden Altar (Door at 17, 5)
    for x in range(14, 22): grid[7][x] = '4'

    # Blocs destructibles ('D')
    grid[17][7] = 'D'; grid[17][28] = 'D'
    grid[21][11] = 'D'; grid[21][24] = 'D'
    grid[25][6] = 'D'; grid[25][18] = 'D'

    return grid

def build_level04():
    # World 4: Volcanic Cavern / Magma
    # Door: ground floor right (30, 24) resting on row 26
    # Player spawn: (3, 25)
    # Enemies: (16, 21) [walker, row 22], (6, 14) [vertical],
    #          (29, 14) [vertical], (24, 25) [static, ground]
    grid = create_base('5', 'L')

    # Stepping row 24
    for x in range(5, 9): grid[24][x] = 'A'
    for x in range(19, 23): grid[24][x] = 'A'

    # Tier 1 row 22
    for x in range(8, 18): grid[22][x] = '5'
    for x in range(22, 28): grid[22][x] = '5'

    # Stepping row 20
    for x in range(9, 13): grid[20][x] = 'A'
    for x in range(23, 27): grid[20][x] = 'A'

    # Tier 2 row 18
    for x in range(11, 24): grid[18][x] = '5'

    # Stepping row 16
    for x in range(7, 12): grid[16][x] = 'A'
    for x in range(23, 28): grid[16][x] = 'A'

    # Tier 3 row 14
    for x in range(9, 16): grid[14][x] = '5'
    for x in range(20, 27): grid[14][x] = '5'

    # Stepping row 12
    for x in range(13, 23): grid[12][x] = 'A'

    # Tier 4 volcanic peak row 10
    for x in range(14, 22): grid[10][x] = '5'

    # Blocs destructibles ('D')
    grid[17][16] = 'D'; grid[17][20] = 'D'
    grid[21][12] = 'D'; grid[21][25] = 'D'
    grid[25][7] = 'D'; grid[25][26] = 'D'

    return grid

def build_level05():
    # World 5: Boss Arena - compact 24x12 horizontal arena
    # Door: right side (20, 8) resting on row 10 floor
    # Player spawn: (4, 9)
    # Boss spawn: (17, 4)
    W5 = 24
    H5 = 12
    grid = [[' ' for _ in range(W5)] for _ in range(H5)]
    # Sostre
    for y in range(2):
        for x in range(W5):
            grid[y][x] = '8'
    # Murs laterals
    for y in range(H5):
        grid[y][0] = '8'
        grid[y][1] = '8'
        grid[y][W5-2] = '8'
        grid[y][W5-1] = '8'
    # Terra sòlid
    for x in range(W5):
        grid[10][x] = '6'
        grid[11][x] = 'M'
    # Plataformes
    # Plataforma central superior a la fila 5
    for x in range(9, 15): grid[5][x] = '6'
    # Plataformes laterals a la fila 7
    for x in range(4, 8): grid[7][x] = 'A'
    for x in range(16, 20): grid[7][x] = 'A'
    # Sense blocs destructibles a l'arena del cap
    return grid

def main():
    levels = [
        ('level01.txt', build_level01(), 36, 28),
        ('level02.txt', build_level02(), 36, 28),
        ('level03.txt', build_level03(), 36, 28),
        ('level04.txt', build_level04(), 36, 28),
        ('level05.txt', build_level05(), 24, 12),
    ]
    
    out_dir = '2DGame/02-Bubble/02-Bubble/levels'
    for filename, grid, w, h in levels:
        path = os.path.join(out_dir, filename)
        lines = ['TILEMAP', f'{w} {h}', '16 16', 'images/Map/world_tileset.png', '16 16']
        for row in grid:
            lines.append(''.join(row))
        content = '\n'.join(lines) + '\n'
        with open(path, 'w', newline='\n') as f:
            f.write(content)
        print(f'Wrote {path} ({len(grid)} rows, {len(grid[0])} cols)')

if __name__ == '__main__':
    main()
