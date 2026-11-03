import os
from collections import deque

W = 36
H = 28

def create_base(border_top, border_fill, floor_top, floor_fill, w=W, h=H):
    grid = [[' ' for _ in range(w)] for _ in range(h)]
    # Top ceiling (2 rows)
    for x in range(w):
        grid[0][x] = border_top
        grid[1][x] = border_fill
    # Left & Right walls (2 cols each)
    for y in range(h):
        grid[y][0] = border_fill
        grid[y][1] = border_fill
        grid[y][w - 2] = border_fill
        grid[y][w - 1] = border_fill
    # Ground floor (2 rows at bottom: h-2 and h-1)
    for x in range(2, w - 2):
        grid[h - 2][x] = floor_top
        grid[h - 1][x] = floor_fill
    return grid

def fill_rect(grid, x0, y0, x1, y1, top_tile, fill_tile=None):
    """Fill rectangle [x0..x1) x [y0..y1) with top_tile on y0 and fill_tile below."""
    if fill_tile is None:
        fill_tile = top_tile
    for y in range(y0, y1):
        for x in range(x0, x1):
            grid[y][x] = top_tile if y == y0 else fill_tile

def hline(grid, x0, x1, y, tile):
    for x in range(x0, x1):
        grid[y][x] = tile


def build_level01():
    """
    World 1: Forest (Inspired directly by Pocket Bomberman Stage 1-1 & Stage 1-3)
    - Uses authentic green grass crust ('1') + brown earth fill ('H'), framed by wood/rock ('2'/'I').
    - Uneven stepped forest floor (like Stage 1-3).
    - Iconic Stage 1-1 central-right enclosed Door Alcove (floor row 18, roof row 12, right wall cols 24-25)
      with destructible blocks ('D') sealing the alcove entrance and shortcuts.
    - Left L-shaped earth structure (from Stage 1-3) and right vertical earth column (from Stage 1-1).
    - Door at (21, 16) resting on row 18 inside the alcove.
    - Player spawn at (3, 25).
    - Enemies: ('H', 13, 23), ('H', 9, 19), ('H', 27, 7)
    """
    grid = create_base('2', 'I', '1', 'H')

    # --- Uneven stepped forest ground (Stage 1-3 bottom profile) ---
    # Left stepping ledge at row 24 (cols 5..9) connecting ground (row 26) to left structure (row 22)
    hline(grid, 5, 9, 24, 'A')
    # Raised ground plateau in center (cols 11..19, rows 24..25)
    fill_rect(grid, 11, 24, 19, 26, '1', 'H')
    # Raised ground step on right (cols 26..31, rows 24..25)
    fill_rect(grid, 26, 24, 31, 26, '1', 'H')

    # --- Left L-shaped Forest Terrace & Pillar (Stage 1-3 left structure) ---
    # Left wall step at row 22 (cols 2..6)
    fill_rect(grid, 2, 22, 6, 23, '1', 'H')
    # Vertical earth pillar (cols 6..8, rows 16..22)
    fill_rect(grid, 6, 16, 8, 22, '1', 'H')
    # Left mid ledge sticking out of pillar at row 20 & 18 (cols 2..6)
    hline(grid, 2, 6, 20, 'A')
    hline(grid, 2, 6, 18, 'A')
    # Center-left step at row 22 (cols 8..13)
    hline(grid, 8, 13, 22, 'A')
    # Terrace extending right from pillar at row 20 (cols 8..12 solid '1', col 12..13 'A' so player can jump up from row 22!)
    fill_rect(grid, 8, 20, 12, 21, '1', 'H')
    hline(grid, 12, 14, 20, 'A')
    # Steppable bridge connecting left terrace to central alcove floor at row 20 (cols 14..16)
    hline(grid, 14, 16, 20, 'C')

    # --- Central-Right Door Alcove Structure (Stage 1-1 centerpiece) ---
    # Alcove base platform at rows 18..19 (cols 15..26)
    fill_rect(grid, 15, 18, 26, 20, '1', 'H')
    # Alcove right back-wall (cols 24..26, rows 12..18)
    fill_rect(grid, 24, 12, 26, 18, '1', 'H')
    # Alcove upper roof & terrace at rows 12..13 (cols 14..24)
    fill_rect(grid, 14, 12, 24, 14, '1', 'H')
    # Upper lip hanging from roof at row 14 (cols 18..20) like Stage 1-1
    hline(grid, 18, 20, 14, 'H')
    # Destructible blocks ('D') guarding the Door alcove entrance (cols 16..18, rows 16..17)
    grid[17][16] = 'D'
    grid[16][16] = 'D'
    grid[17][18] = 'D'
    # Door sits at (21, 16) -> occupies cols 21..22, rows 16..17, resting on row 18!

    # --- Right Vertical Column & Stepping Ledges (Stage 1-1 right side) ---
    # Stepping bridge at row 22 (cols 20..28)
    hline(grid, 20, 28, 22, 'A')
    # Right vertical earth column (cols 29..31, rows 12..20)
    fill_rect(grid, 29, 12, 31, 20, '1', 'H')
    # Ledges on both sides of the right column (rows 12, 14, 16, 18, 20, 22)
    hline(grid, 31, 34, 22, 'A')
    hline(grid, 26, 29, 20, 'A')
    hline(grid, 31, 34, 20, '1')
    hline(grid, 26, 29, 18, 'A')
    hline(grid, 31, 34, 18, 'A')
    hline(grid, 26, 29, 16, 'A')
    hline(grid, 31, 34, 16, '1')
    hline(grid, 26, 29, 14, 'A')
    hline(grid, 31, 34, 14, 'A')
    hline(grid, 26, 29, 12, 'A')
    hline(grid, 31, 34, 12, 'A')

    # --- Upper Canopy Platforms (Stage 1-1 & 1-3 upper section) ---
    # Left step at row 16 (cols 2..6) and row 14 (cols 2..5)
    hline(grid, 2, 6, 16, 'A')
    hline(grid, 2, 5, 14, 'A')
    # Stepping bridge at row 16 (cols 9..15) connecting row 18 alcove to row 14 bridge
    hline(grid, 9, 15, 16, 'A')
    # Left upper island at rows 12..13 (cols 4..10)
    fill_rect(grid, 4, 12, 10, 14, '1', 'H')
    # Stepping bridge between left pillar and upper island at row 14 (cols 9..14)
    hline(grid, 9, 14, 14, 'A')
    # Step at row 10 on left (cols 3..7)
    hline(grid, 3, 7, 10, 'A')
    # High canopy terraces at row 8
    fill_rect(grid, 6, 8, 13, 9, '1', 'H')
    hline(grid, 13, 16, 10, 'C')  # Steppable bridge
    hline(grid, 16, 23, 8, 'A')   # High wooden bridge
    fill_rect(grid, 23, 8, 32, 9, '1', 'H')
    hline(grid, 26, 29, 10, 'A')

    # --- Authentic Pocket Bomberman Destructible Blocks ('D') ---
    grid[25][4] = 'D'
    grid[25][10] = 'D'
    grid[23][16] = 'D'
    grid[25][21] = 'D'
    grid[23][28] = 'D'
    grid[15][6] = 'D'
    grid[15][7] = 'D'
    grid[19][11] = 'D'
    grid[19][32] = 'D'
    grid[15][32] = 'D'
    grid[11][5] = 'D'
    grid[11][16] = 'D'
    grid[11][21] = 'D'
    grid[7][9] = 'D'
    grid[7][30] = 'D'

    return grid


def build_level02():
    """
    World 2: Desert / Ocean Ruins (Inspired directly by Pocket Bomberman Stage 2-1)
    - Uses sand/coral crust ('3') + fill ('J'), framed by temple stone ('4'/'K').
    - Features Stage 2-1's signature layout:
      1. Left chamber with vertical flyer shaft (col 5) separated from the central basin by a tall
         vertical dividing wall (cols 11..13, rows 8..23) with a bombable lower tunnel ('D' at rows 23..25)
         and an upper pass at row 8.
      2. Central open basin with an inverted-T island platform in the middle (cols 16..22, rows 14..16).
      3. Right-hand walled square vault (cols 24..30, rows 16..24) with corridors of 'D' blocks above it.
      4. Upper-Right Door Sanctum at (30, 6) resting on row 8, protected by 'D' blocks.
    - Enemies: ('H', 18, 21), ('V', 5, 14), ('H', 21, 7)
    """
    grid = create_base('4', 'K', '3', 'J')

    # --- Left Chamber & Stepped Ruin Ledges (Stage 2-1 left room) ---
    # Keep col 5 clear from row 8 to 23 for the vertical flying enemy!
    hline(grid, 6, 10, 24, 'A')
    hline(grid, 2, 5, 22, '3')
    hline(grid, 6, 10, 20, 'A')
    fill_rect(grid, 7, 18, 11, 19, '3', 'J')
    hline(grid, 2, 5, 18, 'A')
    hline(grid, 2, 5, 16, '3')
    hline(grid, 6, 11, 16, 'A')
    hline(grid, 7, 11, 14, 'C')  # Steppable bridge on the right of col 5 shaft
    hline(grid, 2, 5, 14, 'A')
    fill_rect(grid, 2, 12, 5, 13, '3', 'J')
    hline(grid, 6, 10, 12, 'A')
    hline(grid, 6, 10, 10, 'A')
    fill_rect(grid, 7, 8, 13, 9, '3', 'J')

    # --- Tall Vertical Dividing Wall (Stage 2-1 cols 11..13) ---
    fill_rect(grid, 11, 9, 13, 23, 'J', 'J')
    # Bombable lower tunnel under the dividing wall (rows 23..25 at col 11..12)
    grid[23][11] = 'D'; grid[23][12] = 'D'
    grid[24][11] = 'D'; grid[24][12] = 'D'
    grid[25][11] = 'D'; grid[25][12] = 'D'

    # --- Central Basin & Inverted-T Island (Stage 2-1 center room) ---
    # Raised basin floor (cols 13..23, rows 22..23) with a lower tunnel beneath
    fill_rect(grid, 13, 22, 23, 23, '3', 'J')
    hline(grid, 21, 24, 24, 'A')  # Step out of lower tunnel on the right
    # Left inner lip (col 14, rows 18..22)
    fill_rect(grid, 14, 18, 15, 22, '3', 'J')
    # Stepping bridges in central basin (rows 20, 18, 16)
    hline(grid, 15, 19, 20, 'A')
    hline(grid, 15, 18, 18, 'A')
    # Iconic Inverted-T platform in the center of the room (rows 13..16, cols 16..22)
    fill_rect(grid, 18, 13, 20, 15, '3', 'J')
    fill_rect(grid, 16, 15, 22, 16, '3', 'J')
    hline(grid, 21, 24, 14, 'A')
    # Upper central roof bridge (row 8, cols 15..24)
    fill_rect(grid, 15, 8, 24, 9, '3', 'J')
    hline(grid, 13, 16, 10, 'C')  # Steppable bridge connecting left wall to center roof
    hline(grid, 20, 24, 10, 'A')
    hline(grid, 21, 24, 12, 'A')

    # --- Right Square Vault & Destructible Corridors (Stage 2-1 right section) ---
    # Walled vault box (cols 24..30, rows 16..24)
    fill_rect(grid, 24, 16, 30, 17, '3', 'J')  # Top of vault
    fill_rect(grid, 24, 23, 30, 24, '3', 'J')  # Bottom of vault
    fill_rect(grid, 24, 17, 25, 23, 'J', 'J')  # Left wall of vault
    fill_rect(grid, 29, 17, 30, 23, 'J', 'J')  # Right wall of vault
    # Row of destructible blocks right on top of the vault (like the seaweed row in Stage 2-1!)
    for x in range(25, 29):
        grid[15][x] = 'D'

    # Upper-mid terrace above the vault (rows 12..13, cols 24..30)
    fill_rect(grid, 24, 12, 30, 13, '3', 'J')
    # Another row of destructible blocks on row 11 (like Stage 2-1's second seaweed layer)
    for x in range(25, 28):
        grid[11][x] = 'D'

    # --- Upper-Right Door Sanctum (Stage 2-1 exit) ---
    # Door sits at (30, 6) resting on row 8 (cols 29..34)
    fill_rect(grid, 29, 8, 34, 9, '3', 'J')
    # Right shaft stepping ledges (cols 30..34) for climbing up/down the right side
    hline(grid, 30, 34, 24, 'A')
    hline(grid, 30, 34, 22, 'A')
    hline(grid, 30, 34, 20, 'A')
    hline(grid, 30, 34, 18, 'A')
    hline(grid, 30, 33, 16, 'C')
    hline(grid, 30, 34, 14, 'A')
    hline(grid, 30, 34, 12, '3')
    hline(grid, 26, 30, 10, 'A')
    # Destructible blocks guarding the Door platform entrance
    grid[7][29] = 'D'
    grid[6][29] = 'D'

    # Additional strategic 'D' blocks
    grid[21][3] = 'D'
    grid[17][9] = 'D'
    grid[21][16] = 'D'
    grid[21][21] = 'D'
    grid[14][17] = 'D'
    grid[7][17] = 'D'

    return grid


def build_level03():
    """
    World 3: Wind / Golden Temple Tower (Inspired directly by Pocket Bomberman Stage 3-1)
    - Uses golden crust ('5') + temple brick ('L'), framed by wood/stone ('2'/'I').
    - Designed as Stage 3-1's vertical climbing tower:
      - Three tall vertical pillars with horizontal branches sticking out on alternating sides.
      - L-shaped platforms in the lower half with boulder ('D') blocks at corners.
      - Central shaft connects from row 16 -> row 14 -> row 12 (Static Guardian) -> row 10 -> row 8 (Door)!
      - Door high up at (14, 6) resting on row 8 between the central pillars!
    - Enemies: ('H', 8, 25), ('H', 24, 23), ('V', 4, 13), ('S', 16, 11)
    """
    grid = create_base('2', 'I', '5', 'L')

    # --- Lower Half: L-Shaped Temple Platforms (Stage 3-1 bottom half) ---
    # Left L-platforms (leaving col 4 clear in rows 8..19 for vertical flyer)
    hline(grid, 5, 10, 24, '5')
    fill_rect(grid, 2, 20, 4, 23, '5', 'L')
    hline(grid, 4, 9, 22, '5')
    hline(grid, 5, 9, 20, 'A')
    fill_rect(grid, 7, 17, 8, 19, '5', 'L')
    hline(grid, 6, 11, 18, '5')

    # Center L-platforms
    hline(grid, 13, 17, 24, 'A')
    fill_rect(grid, 11, 22, 18, 23, '5', 'L')
    hline(grid, 11, 15, 20, 'A')
    fill_rect(grid, 16, 19, 18, 22, '5', 'L')
    hline(grid, 18, 22, 20, '5')
    hline(grid, 12, 16, 18, 'A')
    hline(grid, 11, 16, 16, 'A')
    fill_rect(grid, 16, 16, 18, 19, '5', 'L')

    # Right L-platforms
    hline(grid, 21, 27, 24, '5')
    fill_rect(grid, 27, 22, 33, 23, '5', 'L')
    fill_rect(grid, 29, 19, 31, 22, '5', 'L')
    hline(grid, 22, 27, 20, 'A')
    hline(grid, 31, 34, 20, 'A')
    hline(grid, 25, 29, 18, 'A')
    hline(grid, 31, 34, 18, 'A')
    fill_rect(grid, 28, 16, 34, 17, '5', 'L')

    # --- Upper Half: Tall Vertical Temple Columns with Branches (Stage 3-1 top half) ---
    # Column 1 (Left-Center: col 10..11, rows 6..15)
    fill_rect(grid, 10, 6, 12, 15, '5', 'L')
    hline(grid, 5, 9, 16, 'A')     # Stepping bridge on left
    hline(grid, 6, 10, 14, 'C')    # Steppable bridge to Col 1
    hline(grid, 12, 16, 14, 'A')   # Jump-through right branch off Col 1 in central shaft!
    hline(grid, 5, 9, 12, '5')     # Mid-left ledge
    hline(grid, 6, 10, 10, 'A')    # Branch left off Col 1
    hline(grid, 2, 6, 8, '5')      # High left ledge

    # Column 2 (Center-Right: col 19..20, rows 6..16)
    fill_rect(grid, 19, 6, 21, 16, '5', 'L')
    hline(grid, 21, 26, 16, 'A')
    hline(grid, 21, 26, 14, '5')   # Long right branch (like Stage 3-1 mid branch!)
    hline(grid, 15, 19, 12, '5')   # Guardian platform in central shaft (Enemy Static at 16, 11)
    hline(grid, 12, 15, 12, 'A')   # Jump-through step next to Guardian platform
    hline(grid, 21, 25, 12, 'A')
    hline(grid, 21, 25, 10, 'A')   # Right branch off Col 2

    # Column 3 (Right: col 26..27, rows 6..12)
    fill_rect(grid, 26, 6, 28, 12, '5', 'L')
    hline(grid, 23, 26, 8, '5')
    hline(grid, 28, 32, 14, 'A')
    hline(grid, 28, 32, 12, 'C')
    hline(grid, 30, 34, 10, 'A')
    hline(grid, 30, 34, 8, '5')

    # --- Top Golden Altar Between Col 1 and Col 2 (Stage 3-1 Door position!) ---
    # Door at (14, 6) resting on row 8 (cols 14..18), with jump-through approach at cols 12..14!
    hline(grid, 12, 14, 8, 'A')
    fill_rect(grid, 14, 8, 19, 9, '5', 'L')
    hline(grid, 12, 16, 10, 'A')   # Stepping bridge from row 12 to row 8 altar

    # --- Authentic Stage 3-1 Destructible Boulders ('D') ---
    grid[21][8] = 'D'
    grid[23][5] = 'D'
    grid[21][15] = 'D'
    grid[15][17] = 'D'
    grid[21][28] = 'D'
    grid[17][6] = 'D'
    grid[13][13] = 'D'
    grid[11][6] = 'D'
    grid[13][23] = 'D'
    grid[11][28] = 'D'
    grid[7][17] = 'D'

    return grid


def build_level04():
    """
    World 4: Fortress / Cavern (Inspired directly by Pocket Bomberman Stage 4-1 & Stage 5-1)
    - Uses volcanic/fortress stone ('6'/'M') and glowing cyan ice/crystal ('7'/'N'), framed by dark stone ('9'/'P').
    - Combines Stage 4-1's central square vault & 3-pillar upper chamber with Stage 5-1's left U-bunker
      and rows of destructible pots/crates ('D').
    - Door at (15, 8) resting on row 10 inside the upper 3-pillar fortress chamber!
    - Enemies: ('H', 18, 17), ('V', 3, 11), ('V', 32, 11), ('S', 25, 21)
    """
    grid = create_base('9', 'P', '6', 'M')

    # --- Lower-Left Stepping Ledges & U-Shaped Bunker (from Stage 5-1) ---
    hline(grid, 2, 6, 24, '6')
    hline(grid, 5, 10, 22, 'A')
    hline(grid, 7, 11, 20, 'A')
    # U-shaped bunker (cols 5..12, rows 14..18)
    fill_rect(grid, 5, 14, 12, 15, '6', 'M')   # Bunker roof
    fill_rect(grid, 5, 15, 7, 19, 'M', 'M')    # Left bunker wall
    fill_rect(grid, 10, 15, 12, 19, 'M', 'M')  # Right bunker wall
    hline(grid, 6, 8, 18, '6')                 # Left inner lip
    hline(grid, 9, 11, 18, '6')                # Right inner lip
    hline(grid, 2, 5, 20, 'A')                 # Outer left steps
    hline(grid, 2, 5, 18, 'A')
    hline(grid, 4, 6, 16, 'A')

    # --- Lower-Center Square Vault (from Stage 4-1) ---
    # Hollow square chamber in lower center (cols 14..22, rows 18..24)
    fill_rect(grid, 14, 18, 22, 19, '7', 'N')  # Glowing cyan vault roof (row 18)
    fill_rect(grid, 14, 23, 22, 24, '7', 'N')  # Vault floor (row 23)
    fill_rect(grid, 14, 19, 15, 23, 'N', 'N')  # Vault left wall
    fill_rect(grid, 21, 19, 22, 23, 'N', 'N')  # Vault right wall
    # Steppable bridge connecting left bunker to central vault roof at row 20
    hline(grid, 11, 14, 20, 'C')
    hline(grid, 11, 14, 24, 'A')

    # --- Lower-Right Pot/Crate Corridor (from Stage 5-1) ---
    hline(grid, 22, 25, 24, 'A')
    fill_rect(grid, 23, 22, 31, 23, '6', 'M')
    hline(grid, 31, 34, 24, 'A')
    hline(grid, 22, 27, 20, '6')
    # Iconic row of 4 destructible blocks on row 20/21 (like Stage 5-1's 4 brown pots!)
    for x in range(27, 31):
        grid[20][x] = 'D'
        grid[21][x] = 'D'
    hline(grid, 31, 34, 22, 'A')
    hline(grid, 31, 34, 20, '6')
    hline(grid, 29, 32, 18, 'A')

    # --- Mid-Section L-Platforms & Bridges (from Stage 4-1 & 5-1) ---
    fill_rect(grid, 17, 14, 19, 16, '6', 'M')
    hline(grid, 15, 22, 16, '6')
    hline(grid, 12, 15, 16, 'C')  # Steppable bridge
    hline(grid, 22, 26, 16, 'A')
    fill_rect(grid, 26, 12, 28, 16, '6', 'M')
    hline(grid, 26, 31, 16, '6')
    hline(grid, 28, 31, 14, 'A')
    hline(grid, 5, 10, 12, '6')
    hline(grid, 8, 12, 10, 'A')

    # --- Upper-Center 3-Pillar Door Chamber (from Stage 4-1 top section!) ---
    # Three vertical pillars at cols 12, 18, 22 (rows 6..8, with bombable 'D' gates at rows 8..9!)
    fill_rect(grid, 12, 6, 14, 8, '7', 'N')
    grid[8][12] = 'D'; grid[8][13] = 'D'
    grid[9][12] = 'D'; grid[9][13] = 'D'

    fill_rect(grid, 18, 6, 20, 8, '7', 'N')
    grid[8][18] = 'D'; grid[8][19] = 'D'
    grid[9][18] = 'D'; grid[9][19] = 'D'

    fill_rect(grid, 22, 6, 24, 10, '7', 'N')
    # Chamber floor at row 10 (cols 11..24) -> Door sits at (15, 8) resting on row 10!
    fill_rect(grid, 11, 10, 24, 11, '7', 'N')
    hline(grid, 14, 18, 12, 'A')  # Approach bridge under Door chamber
    hline(grid, 11, 15, 14, 'A')  # Step from row 14 to row 12
    hline(grid, 20, 24, 14, 'A')  # Right approach step
    hline(grid, 21, 25, 12, 'A')
    hline(grid, 24, 28, 10, 'C')  # Steppable bridge on upper right
    hline(grid, 28, 31, 10, '6')

    # --- Additional Destructible Blocks ('D') ---
    grid[25][8] = 'D'
    grid[25][12] = 'D'
    grid[17][8] = 'D'
    grid[13][7] = 'D'
    grid[13][10] = 'D'
    grid[17][16] = 'D'
    grid[17][20] = 'D'
    grid[15][24] = 'D'
    grid[9][20] = 'D'; grid[9][21] = 'D'

    return grid


def build_level05():
    """
    World 5: Boss Arena (Inspired by Pocket Bomberman Stage 1-5 & Stage 5-5)
    - Compact 24x12 arena.
    - Multi-tier floating platforms matching Stage 1-5 / 5-5 layout:
      - Lower-center solid block platform at row 8 (cols 10..14).
      - Two mid-height jump-through platforms on the left (cols 5..9, row 6) and right (cols 15..19, row 6).
      - Two small upper ledges at row 4 (cols 8..11 and cols 13..16).
    - Door at (20, 8) resting on row 10 floor.
    - Player spawn at (4, 9).
    - Boss spawn at (17, 3).
    """
    W5 = 24
    H5 = 12
    grid = create_base('9', 'P', '8', 'O', W5, H5)

    # Central low platform (row 8, cols 10..14) - 2 tiles above row 10 floor (like Stage 1-5 & 5-5)
    hline(grid, 10, 14, 8, '8')

    # Mid-height side platforms (row 6, cols 5..9 and 15..19) - 2 tiles above row 8
    hline(grid, 5, 9, 6, 'A')
    hline(grid, 15, 19, 6, 'A')

    # High small ledges (row 4, cols 8..11 and 13..16) - 2 tiles above row 6 (like Stage 1-5)
    hline(grid, 8, 11, 4, 'A')
    hline(grid, 13, 16, 4, 'A')

    return grid


def verify_level(name, grid, player_pos, door_pos, enemies):
    h = len(grid)
    w = len(grid[0])
    px, py = player_pos
    assert grid[py][px] == ' ', f"{name}: Player spawn ({px},{py}) not empty: {grid[py][px]}"
    assert grid[py + 1][px] != ' ', f"{name}: Player spawn ({px},{py}) has no ground below!"

    dx, dy = door_pos
    for oy in range(2):
        for ox in range(2):
            assert grid[dy + oy][dx + ox] == ' ', f"{name}: Door cell ({dx+ox},{dy+oy}) not empty: {grid[dy+oy][dx+ox]}"
    for ox in range(2):
        assert grid[dy + 2][dx + ox] not in (' ', 'D', 'C'), f"{name}: Door has no solid permanent ground at ({dx+ox},{dy+2})"

    for etype, ex, ey in enemies:
        assert grid[ey][ex] == ' ', f"{name}: Enemy ({etype}) at ({ex},{ey}) not empty: {grid[ey][ex]}"
        if etype in ('H', 'S'):
            assert grid[ey + 1][ex] not in (' ', 'D', 'C'), f"{name}: Ground enemy ({etype}) at ({ex},{ey}) has no solid permanent floor!"
        elif etype == 'V':
            clear_count = sum(1 for yy in range(max(2, ey - 3), min(h - 2, ey + 4)) if grid[yy][ex] == ' ')
            assert clear_count >= 5, f"{name}: Vertical enemy at ({ex},{ey}) has insufficient vertical clearance ({clear_count})"


def main():
    l1 = build_level01()
    l2 = build_level02()
    l3 = build_level03()
    l4 = build_level04()
    l5 = build_level05()

    verify_level('level01', l1, (3, 25), (21, 16), [('H', 13, 23), ('H', 9, 19), ('H', 27, 7)])
    verify_level('level02', l2, (3, 25), (30, 6),  [('H', 18, 21), ('V', 5, 14), ('H', 21, 7)])
    verify_level('level03', l3, (3, 25), (14, 6),  [('H', 8, 25), ('H', 24, 23), ('V', 4, 13), ('S', 16, 11)])
    verify_level('level04', l4, (3, 25), (15, 8),  [('H', 18, 17), ('V', 3, 11), ('V', 32, 11), ('S', 25, 21)])
    verify_level('level05', l5, (4, 9),  (20, 8),  [])

    levels = [
        ('level01.txt', l1, 36, 28),
        ('level02.txt', l2, 36, 28),
        ('level03.txt', l3, 36, 28),
        ('level04.txt', l4, 36, 28),
        ('level05.txt', l5, 24, 12),
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
        print(f'Wrote {path} ({len(grid)} rows, {len(grid[0])} cols) - VERIFIED OK')

if __name__ == '__main__':
    main()
