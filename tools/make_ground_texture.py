"""협곡 바닥 텍스처(assets/textures/ground.bmp)를 만든다. 실행: python tools/make_ground_texture.py

갈라진 마른 흙 무늬를 코드로 그린다(외부 이미지·라이브러리 없음).
 - 셀 무늬(보로노이): 가장 가까운 두 점까지의 거리 차가 작은 곳이 갈라진 틈이 된다.
 - 값 잡음(value noise) 여러 겹: 흙의 얼룩과 자갈.
가로·세로 모두 이어지도록(타일링) 모든 격자를 텍스처 크기에서 되감는다.
다른 그림을 쓰려면 같은 이름의 24비트 BMP로 바꿔 넣으면 된다.
"""
import math
import os
import random
import struct

SIZE = 256
CELLS = 6  # 한 변에 들어가는 갈라진 조각 수.
random.seed(7)

points = [[(random.random(), random.random()) for _ in range(CELLS)] for _ in range(CELLS)]
tones = [[random.uniform(-0.06, 0.06) for _ in range(CELLS)] for _ in range(CELLS)]


def lattice(n):
    return [[random.random() for _ in range(n)] for _ in range(n)]


def value_noise(grid, u, v):
    """격자 값을 부드럽게 보간한다. u, v는 0~1이며 격자 끝에서 처음으로 되감긴다."""
    n = len(grid)
    x, y = u * n, v * n
    x0, y0 = int(x) % n, int(y) % n
    x1, y1 = (x0 + 1) % n, (y0 + 1) % n
    fx, fy = x - int(x), y - int(y)
    fx, fy = fx * fx * (3 - 2 * fx), fy * fy * (3 - 2 * fy)
    top = grid[y0][x0] * (1 - fx) + grid[y0][x1] * fx
    bottom = grid[y1][x0] * (1 - fx) + grid[y1][x1] * fx
    return top * (1 - fy) + bottom * fy


layers = [(lattice(4), 0.5), (lattice(16), 0.3), (lattice(64), 0.2)]


def cracked(u, v):
    """(틈까지의 거리, 조각 고유 색조)를 돌려준다."""
    x, y = u * CELLS, v * CELLS
    cx, cy = int(x), int(y)
    nearest, second, tone = 9.0, 9.0, 0.0
    for dy in (-1, 0, 1):
        for dx in (-1, 0, 1):
            gx, gy = cx + dx, cy + dy
            px, py = points[gy % CELLS][gx % CELLS]
            d = math.hypot(gx + px - x, gy + py - y)
            if d < nearest:
                nearest, second, tone = d, nearest, tones[gy % CELLS][gx % CELLS]
            elif d < second:
                second = d
    return second - nearest, tone


rows = []
for j in range(SIZE):
    row = bytearray()
    for i in range(SIZE):
        u, v = i / SIZE, j / SIZE
        edge, tone = cracked(u, v)
        noise = sum(value_noise(grid, u, v) * weight for grid, weight in layers)
        shade = 0.78 + (noise - 0.5) * 0.4 + tone
        shade *= 0.45 + 0.55 * min(1.0, edge / 0.07)  # 틈 가까이는 어둡게.
        r, g, b = 0.80 * shade, 0.60 * shade, 0.42 * shade  # 붉은 기가 도는 마른 흙색.
        row += bytes(int(max(0.0, min(1.0, c)) * 255) for c in (b, g, r))  # BMP는 BGR 순서.
    rows.append(bytes(row))

path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "assets", "textures", "ground.bmp")
os.makedirs(os.path.dirname(path), exist_ok=True)
pixels = b"".join(rows)  # 256×3바이트는 4의 배수라 행 채움이 필요 없다.
with open(path, "wb") as file:
    file.write(b"BM" + struct.pack("<IHHI", 54 + len(pixels), 0, 0, 54))
    file.write(struct.pack("<IiiHHIIiiII", 40, SIZE, SIZE, 1, 24, 0, len(pixels), 2835, 2835, 0, 0))
    file.write(pixels)
print("wrote", path)
