#!/usr/bin/env python3

notes = "C C#D D#E F F#G G#A A#H "

# https://de.wikipedia.org/wiki/Frequenzen_der_gleichstufigen_Stimmung
print("const uint16_t notes[] = {")
n = 0
for t in range(28, 87):
    freq = pow(2, (t-49) / 12) * 440
    length = 1 / freq
    arr = (int)((length * 8000000) - 1)
    print("{}, // {} {:02d} {}".format(arr, notes[n*2:n*2+2:1], t - 28, round(freq, 2)))
    n = n + 1
    if (n > 11):
      n = 0
print("};")
