import wave, struct, math, glob, os

for path in sorted(glob.glob("2DGame/02-Bubble/02-Bubble/sound/SFX/*.wav")):
    w = wave.open(path, "r")
    params = w.getparams()
    nframes = w.getnframes()
    data = w.readframes(nframes)
    w.close()
    
    samples = list(struct.unpack("<" + str(nframes * params.nchannels) + "h", data))
    peak = max(abs(s) for s in samples)
    if peak == 0:
        continue
    
    target_peak = 31000 # ~95% of 32767
    gain = target_peak / peak
    
    # Per bomb_explosion volem el màxim impacte
    if "bomb_explosion" in path:
        gain = min(gain, 3.5)
    
    amplified = [max(-32767, min(32767, int(s * gain))) for s in samples]
    out_data = struct.pack("<" + str(len(amplified)) + "h", *amplified)
    
    # Save back
    out = wave.open(path, "w")
    out.setparams(params)
    out.writeframes(out_data)
    out.close()
    
    new_peak = max(abs(s) for s in amplified)
    print(f"Normalized {os.path.basename(path)}: gain={gain:.2f}x, new_peak={new_peak/32767*100:.1f}%")
