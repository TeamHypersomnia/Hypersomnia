#!/usr/bin/env python3
"""
Compares generated bot waypoints (XYZDBG gen lines, many seeds) with the author's (XYZDBG off) on one map.
Usage: analyze.py <log> <out png> [radius]
"""
import sys, math, collections
import numpy as np
from PIL import Image, ImageDraw

log, out_png = sys.argv[1], sys.argv[2]
R = float(sys.argv[3]) if len(sys.argv) > 3 else 400.0

cells, off, gen = [], [], collections.defaultdict(list)
avg_us = None

for line in open(log):
	p = line.split()
	if len(p) < 2:
		continue
	if p[1] == 'cell':
		cells.append((float(p[2]), float(p[3])))
	elif p[1] == 'off':
		off.append(dict(type=p[2], faction=int(p[3]), x=float(p[4]), y=float(p[5]), quality=float(p[6]), ahead=float(p[7])))
	elif p[1] == 'gen':
		gen[int(p[2])].append(dict(type=p[3], faction=int(p[4]), x=float(p[5]), y=float(p[6]), secs=float(p[7]), far=p[8] in ('1', 'true')))
	elif p[1] == 'avg_us':
		avg_us = int(p[2])

cells = np.array(cells)
seeds = sorted(gen)

def pts(ws, t=None):
	return np.array([(w['x'], w['y']) for w in ws if t is None or w['type'] == t]).reshape(-1, 2)

def coverage(points):
	if len(points) == 0:
		return float('nan'), float('nan')
	d = np.sqrt(((cells[:, None, :] - points[None, :, :]) ** 2).sum(-1)).min(1)
	return float(d.mean()), float(np.percentile(d, 90))

off_patrols = pts(off, 'patrol')
off_pushes = pts(off, 'push')

print(f"== {log}")
print(f"cells={len(cells)} seeds={len(seeds)} avg generation {avg_us} us")
print(f"author: patrol={len(off_patrols)} push={len(off_pushes)}")

pc = [len(pts(gen[s], 'patrol')) for s in seeds]
pu = [len(pts(gen[s], 'push')) for s in seeds]
print(f"generated: patrol mean={np.mean(pc):.1f} min={min(pc)} max={max(pc)}   push mean={np.mean(pu):.1f} min={min(pu)} max={max(pu)}")

oc = coverage(np.vstack([off_patrols, off_pushes]))
gcs = [coverage(pts(gen[s])) for s in seeds]
print(f"coverage (distance of a free cell to the nearest waypoint): author mean={oc[0]:.0f} p90={oc[1]:.0f}   generated mean={np.mean([g[0] for g in gcs]):.0f} p90={np.mean([g[1] for g in gcs]):.0f}")

# How often a generated patrol lands near each of the author's patrols.
hits = []
for i, o in enumerate(off_patrols):
	n = sum(1 for s in seeds if len(pts(gen[s], 'patrol')) and (np.sqrt(((pts(gen[s], 'patrol') - o) ** 2).sum(1)).min() <= R))
	hits.append(n / len(seeds))

for RR in (600.0, 800.0):
	hh = []
	for o in off_patrols:
		n = sum(1 for s in seeds if len(pts(gen[s], 'patrol')) and (np.sqrt(((pts(gen[s], 'patrol') - o) ** 2).sum(1)).min() <= RR))
		hh.append(n / len(seeds))
	if hh:
		print(f"author patrol spots hit within {RR:.0f}px: mean={np.mean(hh)*100:.0f}%  min={min(hh)*100:.0f}%  spots under 30%: {sum(1 for h in hh if h < 0.3)}/{len(hh)}")

if hits:
	print(f"author patrol spots hit within {R:.0f}px: mean={np.mean(hits)*100:.0f}%  min={min(hits)*100:.0f}%  spots under 30%: {sum(1 for h in hits if h < 0.3)}/{len(hits)}")
	print("   per spot: " + " ".join(f"{h*100:.0f}" for h in hits))

# Stability: how alike two seeds are - share of patrols of one seed with a patrol of the other within R.
sim = []
for a, b in zip(seeds[:-1], seeds[1:]):
	A, B = pts(gen[a], 'patrol'), pts(gen[b], 'patrol')
	if len(A) and len(B):
		sim.append(np.mean(np.sqrt(((A[:, None, :] - B[None, :, :]) ** 2).sum(-1)).min(1) <= R))
print(f"seed to seed similarity of patrols within {R:.0f}px: {np.mean(sim)*100:.0f}% (100% = always the same spots)")

secs = collections.Counter(w['secs'] for s in seeds for w in gen[s] if w['type'] == 'patrol')
far = sum(1 for s in seeds for w in gen[s] if w['type'] == 'patrol' and w['far'])
total_p = sum(pc)
print(f"camp classes (min secs): {dict(sorted(secs.items()))}  look_far {far/total_p*100:.0f}%")
print(f"author spot quality: " + " ".join(f"{o['quality']:.2f}" for o in off if o['type'] == 'patrol'))

# Picture: free cells, density of generated patrols (red), author patrols (green), author pushes (blue), generated pushes (cyan dots, faint).
lo = cells.min(0) - 300
hi = cells.max(0) + 300
scale = 900 / max(hi - lo)
size = tuple(((hi - lo) * scale).astype(int) + 1)
img = Image.new('RGB', size, (20, 20, 24))
d = ImageDraw.Draw(img, 'RGBA')

def to_px(x, y):
	return ((x - lo[0]) * scale, (y - lo[1]) * scale)

cell_px = 128 * scale / 2
for x, y in cells:
	cx, cy = to_px(x, y)
	d.rectangle([cx - cell_px, cy - cell_px, cx + cell_px, cy + cell_px], fill=(70, 70, 78, 255))

for s in seeds:
	for w in gen[s]:
		cx, cy = to_px(w['x'], w['y'])
		if w['type'] == 'patrol':
			d.ellipse([cx - 6, cy - 6, cx + 6, cy + 6], fill=(255, 60, 60, 14))
		else:
			d.ellipse([cx - 3, cy - 3, cx + 3, cy + 3], fill=(60, 230, 255, 10))

for o in off:
	cx, cy = to_px(o['x'], o['y'])
	col = (80, 255, 80, 255) if o['type'] == 'patrol' else (90, 140, 255, 255)
	d.ellipse([cx - 5, cy - 5, cx + 5, cy + 5], outline=col, width=2)

img.save(out_png)
print(f"picture: {out_png}")

import json
def hit_stats(RR):
	hh = []
	for o in off_patrols:
		n = sum(1 for s in seeds if len(pts(gen[s], 'patrol')) and (np.sqrt(((pts(gen[s], 'patrol') - o) ** 2).sum(1)).min() <= RR))
		hh.append(n / len(seeds))
	return hh

summary = dict(
	cells=len(cells), seeds=len(seeds), avg_us=avg_us,
	author_patrols=len(off_patrols), author_pushes=len(off_pushes),
	gen_patrol_mean=float(np.mean(pc)), gen_patrol_min=int(min(pc)), gen_patrol_max=int(max(pc)), gen_push_mean=float(np.mean(pu)),
	coverage_author=oc, coverage_generated=(float(np.mean([g[0] for g in gcs])), float(np.mean([g[1] for g in gcs]))),
	hits400=hit_stats(400.0), hits600=hit_stats(600.0), hits800=hit_stats(800.0),
	similarity=float(np.mean(sim)) if sim else None,
	camp_classes={str(k): v for k, v in sorted(secs.items())}, look_far=far / max(1, total_p),
)
json.dump(summary, open(out_png.replace('.png', '.json'), 'w'))
