# HRTF presets

**The files in this folder are NOT covered by Hypersomnia's AGPL-3.0 license.**

They are HRTF data sets in the OpenAL Soft `.mhr` format. They come from the SADIE II database and are licensed under the Apache License, Version 2.0. The full license text is in `LICENSE-SADIE-II.txt` in this folder.

## Source

SADIE II Database, AudioLab, Department of Electronic Engineering, University of York.
Gavin Kearney, Callum Armstrong, Lewis Thresh.

- https://www.york.ac.uk/sadie-project/database.html
- https://doi.org/10.5281/zenodo.10886409

Copyright © University of York. Licensed under the Apache License, Version 2.0.

Reference: C. Armstrong, L. Thresh, D. Murphy, G. Kearney, "A Perceptual Evaluation of Individual and Non-Individual HRTFs: A Case Study of the SADIE II Database", Applied Sciences 8(11), 2018.

## Modifications

The original 48 kHz, 256-tap SOFA files were converted to the `.mhr` format with OpenAL Soft's `makemhr` (version 1.25.2):

```
makemhr -w 128 -i <subject>_48K_24bit_256tap_FIR_SOFA.sofa -o "<preset name>.mhr"
```

This applies minimum-phase reconstruction, diffuse-field equalization (the default) and truncation to 128 taps.

| File | SADIE II subject |
|---|---|
| `SADIE KU100.mhr` | D1: Neumann KU100 dummy head |
| `SADIE KEMAR.mhr` | D2: GRAS KEMAR dummy head |
| `SADIE H3.mhr` | H3: human subject |
| `SADIE H4.mhr` | H4: human subject |
| `SADIE H5.mhr` | H5: human subject |

## Your own presets

Do not add files here. Put your own `.mhr` files in the `hrtf` folder inside your user folder: in Settings → Audio, click "Open user HRTF presets folder". They will show up under "User presets".
