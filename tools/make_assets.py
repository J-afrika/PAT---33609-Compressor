"""Procedurally renders the plugin's photoreal-style bitmaps (original artwork, no third-party assets).
   python tools/make_assets.py   ->  Assets/knob.png, Assets/panel.png, Assets/screw.png"""
import numpy as np
from PIL import Image, ImageFilter
import os
os.makedirs('Assets', exist_ok=True)

def over(drgb, da, srgb, sa):
    oa = sa + da * (1 - sa)
    orgb = (srgb * sa[..., None] + drgb * (da * (1 - sa))[..., None]) / np.maximum(oa, 1e-6)[..., None]
    return orgb, oa

def norm(v):
    return v / np.linalg.norm(v)

# ------------------------------------------------------------------ knob
S, SS = 160, 3
N = S * SS
yy, xx = np.mgrid[0:N, 0:N].astype(np.float32)
c = (N - 1) / 2
dx, dy = (xx - c) / (N / 2), (yy - c) / (N / 2)
r = np.sqrt(dx * dx + dy * dy) + 1e-6
th = np.arctan2(dy, dx)
L = norm(np.array([-0.45, -0.55, 0.70], np.float32))
H = norm(L + np.array([0, 0, 1], np.float32))

rgb = np.zeros((N, N, 3), np.float32)
a = np.zeros((N, N), np.float32)

# drop shadow
sh = ((np.sqrt(dx ** 2 + (dy - 0.07) ** 2)) < 0.86).astype(np.float32) * 0.6
sh = np.asarray(Image.fromarray((sh * 255).astype(np.uint8)).filter(ImageFilter.GaussianBlur(5 * SS))) / 255.0
rgb, a = over(rgb, a, np.zeros((N, N, 3), np.float32), sh.astype(np.float32))

def shade(nx, ny, nz, base, spec_pow, spec_amt, amb=0.35, dif=0.8):
    d = np.maximum(0, nx * L[0] + ny * L[1] + nz * L[2])
    s = np.maximum(0, nx * H[0] + ny * H[1] + nz * H[2]) ** spec_pow * spec_amt
    col = base[None, None, :] * (amb + dif * d)[..., None] + s[..., None] * 255
    return np.clip(col, 0, 255)

# knurled aluminium skirt
Rs = 0.88
t = np.clip((r - 0.55) / (Rs - 0.55), 0, 1) ** 2 * 0.95
nz = np.sqrt(np.clip(1 - t * t, 0, 1))
nx, ny = dx / r * t, dy / r * t
ridge = 0.5 + 0.5 * np.cos(54 * th)
skirt = shade(nx, ny, nz, np.array([205, 207, 205], np.float32), 25, 0.35)
skirt *= (0.82 + 0.18 * ridge)[..., None]
skirt *= (1 - 0.35 * np.clip((r - 0.84) / 0.04, 0, 1))[..., None]      # dark rim edge
rgb, a = over(rgb, a, skirt, (r < Rs).astype(np.float32))

# blue lacquered cap
Rc = 0.64
rho = np.clip(r / Rc, 0, 1)
t = rho ** 3 * 0.85
nz = np.sqrt(np.clip(1 - t * t, 0, 1))
nx, ny = dx / r * t, dy / r * t
cap = shade(nx, ny, nz, np.array([30, 118, 182], np.float32), 55, 0.75, amb=0.30, dif=0.95)
cap *= (1 + 0.035 * np.cos(rho * 110))[..., None]                      # turned rings
rim = np.clip((rho - 0.92) / 0.08, 0, 1)
cap = cap * (1 - rim[..., None]) + np.array([196, 200, 203], np.float32) * rim[..., None] * (0.55 + 0.6 * np.maximum(0, nx * L[0] + ny * L[1] + 0.3))[..., None]
rgb, a = over(rgb, a, np.clip(cap, 0, 255), (r < Rc).astype(np.float32))

img = Image.fromarray(np.dstack([np.clip(rgb, 0, 255), a * 255]).astype(np.uint8), 'RGBA').resize((S, S), Image.LANCZOS)
img.save('Assets/knob.png')

# ------------------------------------------------------------------ panel texture (tileable brushed navy)
rng = np.random.default_rng(7)
n = rng.normal(0, 1, (256, 256)).astype(np.float32)
br = sum(np.roll(n, i, axis=1) for i in range(24)) / np.sqrt(24)
tex = 0.65 * br + 0.35 * n
base = np.array([33, 45, 59], np.float32)
panel = np.clip(base + tex[..., None] * np.array([5.5, 5.5, 6.5], np.float32), 0, 255)
Image.fromarray(panel.astype(np.uint8), 'RGB').save('Assets/panel.png')

# ------------------------------------------------------------------ screw
S2, SS2 = 64, 4
M = S2 * SS2
y2, x2 = np.mgrid[0:M, 0:M].astype(np.float32)
c2 = (M - 1) / 2
ex, ey = (x2 - c2) / (M / 2), (y2 - c2) / (M / 2)
rr = np.sqrt(ex * ex + ey * ey) + 1e-6
t = np.clip(rr / 0.9, 0, 1) ** 3 * 0.8
nz = np.sqrt(np.clip(1 - t * t, 0, 1))
nx, ny = ex / rr * t, ey / rr * t
d = np.maximum(0, nx * L[0] + ny * L[1] + nz * L[2])
sp = np.maximum(0, nx * H[0] + ny * H[1] + nz * H[2]) ** 40 * 0.6
col = (np.array([120, 122, 124], np.float32)[None, None, :] * (0.4 + 0.8 * d)[..., None]) + sp[..., None] * 255
slot = (np.abs(ey) < 0.09) | (np.abs(ex) < 0.09)
slot &= rr < 0.68
col[slot] *= 0.25
alpha = (rr < 0.92).astype(np.float32) * 255
screw = Image.fromarray(np.dstack([np.clip(col, 0, 255), alpha]).astype(np.uint8), 'RGBA').resize((S2, S2), Image.LANCZOS)
screw.save('Assets/screw.png')
print('assets written')
