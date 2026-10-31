# decal_viz.py <xyz.txt> <standing sprite name> <out.png>
# Left: the standing sprite with the holes where they were (attachment offsets).
# Right: the lying corpse with the holes where they landed. Same colour = same hole.
import sys, re
from PIL import Image, ImageDraw
xyz, standing_name, out = sys.argv[1:]
import os
gfx = os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../../hypersomnia/content/gfx/')
S = 5
rows = []
drips = []
import math
for line in open(xyz):
    m = re.search(r'drip (ok|skip) local=\(([-\d.e]+);([-\d.e]+)\)(?: rot=([-\d.e]+) variant=(\d))?', line)
    if m:
        drips.append((m.group(1) == 'ok', float(m.group(2)), float(m.group(3)), float(m.group(4) or 0), int(m.group(5) or 0)))
        continue
    m = re.search(r'from_standing=1 flipped=(\d) .*standing=\(([-\d.e]+);([-\d.e]+)\) local=\(([-\d.e]+);([-\d.e]+)\)', line)
    if m:
        rows.append((m.group(1) == '1', *map(float, m.groups()[1:])))
st = Image.open(gfx + standing_name + '.png').convert('RGBA')
corpse_name = 'metropolis_torso_corpse'
for line in open(xyz):
    m = re.search(r'corpse=Lying corpse (\w+)((?: \w+)*) from_standing=1', line)
    if m:
        corpse_name = m.group(1) + '_torso_corpse' + ''.join('_' + w for w in m.group(2).split())
        break
ly = Image.open(gfx + corpse_name + '.png').convert('RGBA')
flipped = rows[0][0] if rows else False
if flipped:
    ly = ly.transpose(Image.FLIP_TOP_BOTTOM)
W = (st.width + ly.width + 40) * S
H = (max(st.height, ly.height) + 20) * S
img = Image.new('RGBA', (W, H), (60, 60, 60, 255))
sx, sy = 10 * S, 10 * S
lx, ly0 = (st.width + 30) * S, 10 * S
img.paste(st.resize((st.width * S, st.height * S), Image.NEAREST), (sx, sy), st.resize((st.width * S, st.height * S), Image.NEAREST))
img.paste(ly.resize((ly.width * S, ly.height * S), Image.NEAREST), (lx, ly0), ly.resize((ly.width * S, ly.height * S), Image.NEAREST))
d = ImageDraw.Draw(img)
cols = [(255,0,0),(0,255,0),(0,160,255),(255,255,0),(255,0,255),(0,255,255),(255,128,0),(255,255,255),(128,255,128),(255,128,192),(160,160,255),(200,100,0),(0,128,0),(128,0,255)]
scx, scy = sx + st.width * S / 2, sy + st.height * S / 2
lcx, lcy = lx + ly.width * S / 2, ly0 + ly.height * S / 2
for i, (_, x, y, a, b) in enumerate(rows):
    c = cols[i % len(cols)]
    d.ellipse([scx + x*S - 6, scy + y*S - 6, scx + x*S + 6, scy + y*S + 6], outline=c, width=3)
    d.text((scx + x*S + 7, scy + y*S - 6), str(i), fill=c)
    d.ellipse([lcx + a*S - 6, lcy + b*S - 6, lcx + a*S + 6, lcy + b*S + 6], outline=c, width=3)
    d.text((lcx + a*S + 7, lcy + b*S - 6), str(i), fill=c)
d.text((10, 2), f'{xyz} {corpse_name}  flipped={flipped}', fill=(255,255,255))
for ok, a, b, rot, var in drips:
    if not ok:
        d.line([lcx + a*S - 5, lcy + b*S - 5, lcx + a*S + 5, lcy + b*S + 5], fill=(255,255,255), width=2)
        continue
    dr = Image.open(gfx + f'character_gunshot_drip_{var+1}.png').convert('RGBA')
    dr = dr.resize((dr.width*S, dr.height*S), Image.NEAREST).rotate(rot, expand=True, resample=Image.NEAREST)
    h = Image.open(gfx + f'character_gunshot_drip_{var+1}.png').height
    r = math.radians(-rot)
    ox, oy = 0, (h/2 - 0.5)
    cx = lcx + (a + ox*math.cos(r) - oy*math.sin(r))*S
    cy = lcy + (b + ox*math.sin(r) + oy*math.cos(r))*S
    img.alpha_composite(dr, (int(cx - dr.width/2), int(cy - dr.height/2)))
d.text((10, 14), f'drips ok={sum(1 for x in drips if x[0])} skip={sum(1 for x in drips if not x[0])}', fill=(255,255,255))
img.save(out)
