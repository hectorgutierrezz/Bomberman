import wave, struct, os

def adjust_volume(filename, target_peak_ratio):
    path = os.path.join("2DGame/02-Bubble/02-Bubble/sound/SFX", filename)
    w = wave.open(path, "r")
    params = w.getparams()
    nframes = w.getnframes()
    data = w.readframes(nframes)
    w.close()

    samples = list(struct.unpack("<" + str(nframes * params.nchannels) + "h", data))
    peak = max(abs(s) for s in samples)
    if peak == 0:
        return
    
    target_peak = int(32767 * target_peak_ratio)
    gain = target_peak / peak
    adjusted = [max(-32767, min(32767, int(s * gain))) for s in samples]
    out_data = struct.pack("<" + str(len(adjusted)) + "h", *adjusted)

    out = wave.open(path, "w")
    out.setparams(params)
    out.writeframes(out_data)
    out.close()

    new_peak = max(abs(s) for s in adjusted)
    print(f"Adjusted {filename}: target={target_peak_ratio*100:.0f}%, new_peak={new_peak/32767*100:.1f}%")

adjust_volume("step_horizontal.wav", 0.22) # Molt suau per als passos continus
adjust_volume("step_vertical.wav", 0.40)   # Suau per al salt
