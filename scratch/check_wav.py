import wave, struct, math, glob

for path in sorted(glob.glob("2DGame/02-Bubble/02-Bubble/sound/SFX/*.wav")):
    w = wave.open(path, "r")
    nframes = w.getnframes()
    data = w.readframes(nframes)
    samples = struct.unpack("<" + str(nframes * w.getnchannels()) + "h", data)
    peak = max(abs(s) for s in samples)
    rms = math.sqrt(sum(s*s for s in samples)/len(samples))
    print(f"{path.split('/')[-1]}: Peak={peak/32767*100:.1f}%, RMS={rms/32767*100:.1f}%")
