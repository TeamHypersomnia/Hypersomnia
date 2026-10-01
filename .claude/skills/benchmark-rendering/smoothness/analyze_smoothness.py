"""
Frame pacing / judder analysis of the CSVs dumped by smoothness.patch.

Usage: python3 analyze_smoothness.py head=run1.csv head=run2.csv v300=run3.csv ...
Runs with the same name are summarized together. Only the steady part of walking is analyzed
(0.7 s after pressing the keys until 0.2 s before releasing them).
"""
import csv, sys, numpy as np

def load(path):
    r = list(csv.DictReader(open(path)))
    cols = {k: np.array([float(x[k]) for x in r]) for k in r[0].keys()}
    return cols

def pct(a, ps=(50, 99, 99.9)):
    return " ".join(f"p{p}={np.percentile(a, p):.3f}" for p in ps) + f" max={a.max():.3f}"

def analyze(path, verbose=True):
    c = load(path)
    off = c['t_perform_begin'] - c['gl_now_at_perform_begin'] / 1e9
    off = np.median(off)
    gpu_end = c['gpu_end'] / 1e9 + off
    gpu_begin = c['gpu_begin'] / 1e9 + off
    valid = (c['gpu_end'] > 0) & (c['t_swap_end'] > 0)
    disp = np.maximum(gpu_end, c['t_swap_end'])

    w = (c['walking'] == 1) & valid
    idx = np.where(w)[0]
    t0 = c['t_frame_start'][idx[0]]
    # steady walking part: skip 0.7 s of acceleration, last 0.2 s
    sel = idx[(c['t_frame_start'][idx] > t0 + 0.7) & (c['t_frame_start'][idx] < c['t_frame_start'][idx[-1]] - 0.2)]

    res = {}
    zoom = c['zoom'][sel]
    ts = c['t_frame_start'][sel]
    td = disp[sel]

    fd = np.diff(ts) * 1e3
    dd = np.diff(td) * 1e3
    res['n'] = len(sel)
    res['fps'] = 1e3 / np.mean(dd)
    res['frame_ms'] = fd
    res['disp_ms'] = dd
    res['game_ms'] = (c['t_prepared'][sel] - ts) * 1e3
    res['gpu_ms'] = (gpu_end[sel] - gpu_begin[sel]) * 1e3
    res['swap_ms'] = (c['t_swap_end'][sel] - c['t_swap_begin'][sel]) * 1e3
    res['latency_ms'] = (td - ts) * 1e3
    res['latency_jump_ms'] = np.abs(np.diff(res['latency_ms']))
    res['latency_dev_ms'] = np.abs(res['latency_ms'] - np.median(res['latency_ms']))

    med = np.median(dd)
    res['hitches_disp'] = int(np.sum(dd > 2 * med))
    res['hitches_disp_3ms'] = int(np.sum(dd > med + 3))

    def residual(t, key):
        # local fit: cubic on whole steady segment, residual in screen px
        out = []
        for ax in ('x', 'y'):
            v = c[f'{key}_{ax}'][sel]
            p = np.polyfit(t - t[0], v, 3)
            out.append((v - np.polyval(p, t - t[0])) * zoom)
        return np.hypot(out[0], out[1])

    def hf(t, key, half=0.03):
        out = []
        for ax in ('x', 'y'):
            v = c[f'{key}_{ax}'][sel]
            # moving average over +-half seconds, time-weighted by samples
            lo = np.searchsorted(t, t - half); hi = np.searchsorted(t, t + half)
            cs = np.concatenate([[0], np.cumsum(v)])
            ma = (cs[hi] - cs[lo]) / np.maximum(hi - lo, 1)
            out.append((v - ma) * zoom)
        r = np.hypot(out[0], out[1])
        m = (t > t[0] + half) & (t < t[-1] - half)
        return r[m]

    res['cam_hf_display'] = hf(td, 'cam')
    res['camraw_hf_content'] = hf(ts, 'cam_raw')
    res['char_hf_content'] = hf(ts, 'char')
    res['cam_res_content'] = residual(ts, 'cam')
    res['cam_res_display'] = residual(td, 'cam')
    res['camraw_res_content'] = residual(ts, 'cam_raw')
    res['char_res_content'] = residual(ts, 'char')

    dur = ts[-1] - ts[0]
    back = 0
    for ax in ('x', 'y'):
        d = np.diff(c[f'cam_{ax}'][sel])
        direction = np.sign(np.sum(d))
        back += int(np.sum(d * direction < 0))
    res['cam_back_per_s'] = back / dur
    # 144 Hz scanout simulation: frame visible at vblank = last displayed before it
    period = 1 / 144.0
    camx = c['cam_x'][sel] * zoom
    camy = c['cam_y'][sel] * zoom
    steps_all = []
    for phase in np.linspace(0, period, 12, endpoint=False):
        vb = np.arange(td[0] + phase, td[-1], period)
        k = np.searchsorted(td, vb, side='right') - 1
        k = k[k >= 0]
        sx = np.diff(camx[k]); sy = np.diff(camy[k])
        steps_all.append(np.hypot(sx, sy))
    st = np.concatenate(steps_all)
    res['vblank_step_px'] = st
    m = np.median(st)
    res['vblank_step_dev'] = np.abs(st - m)
    res['vblank_repeat'] = float(np.mean(st < 0.25 * m))
    return res

def summarize(name, rs):
    def cat(k): return np.concatenate([r[k] for r in rs])
    print(f"== {name}: runs={len(rs)} frames={sum(r['n'] for r in rs)} fps={np.mean([r['fps'] for r in rs]):.0f}")
    for k in ('disp_ms', 'frame_ms', 'game_ms', 'gpu_ms', 'swap_ms', 'latency_ms', 'latency_jump_ms', 'latency_dev_ms',
              'cam_hf_display', 'camraw_hf_content', 'char_hf_content', 'cam_res_content', 'cam_res_display', 'camraw_res_content', 'char_res_content',
              'vblank_step_px', 'vblank_step_dev'):
        print(f"  {k:20s} {pct(cat(k))}")
    print(f"  hitches(>2x median disp): {[r['hitches_disp'] for r in rs]}  (>median+3ms): {[r['hitches_disp_3ms'] for r in rs]}")
    print(f"  camera 1px backward steps per second: {[round(float(r['cam_back_per_s']), 2) for r in rs]}")
    print(f"  vblank repeat frac: {[round(float(r['vblank_repeat']), 4) for r in rs]}")

if __name__ == '__main__':
    groups = {}
    for a in sys.argv[1:]:
        name, path = a.split('=')
        groups.setdefault(name, []).append(analyze(path))
    for k, v in groups.items():
        summarize(k, v)
