import pyPQevalAudioBasic as pq
import librosa as lr
import numpy as np

o = pq.BasicOptions()
o.default()
print(o.Lp)
print(o.Ni)
print(o.ClipMOV)
print(o.PCinit)
print(o.PDfactor)
print(o.OverlapDelay)
print(o.DataBounds)
print(o.EndMin)
a = lr.load("a.wav", sr=48000)[0]
b = lr.load("b.wav", sr=48000)[0]
pq.wavfiles(a, b, o)
