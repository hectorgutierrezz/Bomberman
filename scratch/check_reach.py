import os
from collections import deque

def is_solid_or_standable(ch):
    return ch not in (' ', 'D')

def is_passable_upward(ch):
    return ch in (' ', 'D', 'A', 'B')

def get_reachable(grid, start_pos):
    h = len(grid)
    w = len(grid[0])

    def standable(x, y):
        if x < 2 or x >= w - 2 or y < 2 or y >= h - 1:
            return False
        if grid[y][x] not in (' ', 'D'):
            return False
        return is_solid_or_standable(grid[y + 1][x])

    sx, sy = start_pos
    if not standable(sx, sy):
        return set()

    visited = set([(sx, sy)])
    q = deque([(sx, sy)])

    while q:
        x, y = q.popleft()
        for nx in (x - 1, x + 1):
            if standable(nx, y) and (nx, y) not in visited:
                visited.add((nx, y))
                q.append((nx, y))

        for nx in (x - 1, x, x + 1):
            if 2 <= nx < w - 2:
                can_drop = False
                if nx == x and grid[y + 1][x] in ('A', 'B', 'C'):
                    can_drop = True
                elif nx != x and grid[y][nx] in (' ', 'D'):
                    can_drop = True
                if can_drop:
                    for ny in range(y + 1, h - 1):
                        if grid[ny][nx] not in (' ', 'D', 'A', 'B'):
                            break
                        if standable(nx, ny):
                            if (nx, ny) not in visited:
                                visited.add((nx, ny))
                                q.append((nx, ny))
                            break

        for dy, max_dx in [(-2, 2), (-1, 3), (0, 3), (1, 3), (2, 4)]:
            ny = y + dy
            if ny < 2 or ny >= h - 1:
                continue
            if dy < 0:
                head_ok = True
                for cy in range(ny, y):
                    if not is_passable_upward(grid[cy][x]):
                        head_ok = False
                        break
                if not head_ok:
                    continue

            for dx in range(-max_dx, max_dx + 1):
                nx = x + dx
                if standable(nx, ny) and (nx, ny) not in visited:
                    step = 1 if nx >= x else -1
                    blocked = False
                    for mx in range(x, nx + step, step):
                        if mx == nx and grid[ny][mx] in (' ', 'D'):
                            continue
                        if not is_passable_upward(grid[ny][mx]):
                            blocked = True
                            break
                    if not blocked:
                        visited.add((nx, ny))
                        q.append((nx, ny))
    return visited

def check_reachability(name, grid, start_pos, targets):
    visited = get_reachable(grid, start_pos)
    missing = []
    for label, tx, ty in targets:
        if (tx, ty) not in visited:
            missing.append((label, tx, ty))
    if missing:
        print(f"  [FAIL] {name}: Missing targets {missing}")
        return False

    # Check for softlocks (any reachable tile that cannot get back to start_pos or Door)
    door_pos = (targets[0][1], targets[0][2])
    softlocks = []
    for tile in visited:
        from_tile = get_reachable(grid, tile)
        if door_pos not in from_tile:
            softlocks.append(tile)
    if softlocks:
        print(f"  [SOFTLOCK WARNING] {name}: {len(softlocks)} tiles cannot reach Door: {softlocks[:8]}")
        return False

    print(f"  [REACHABILITY & ZERO-SOFTLOCK OK] {name}: {len(visited)} standable tiles, all {len(targets)} targets & 0 softlocks!")
    return True
