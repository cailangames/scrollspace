# Script for generating new biomes (map levels)

import numpy as np
import matplotlib.pyplot as plt

MAP_LENGTH = 20*50
W_MIN = 5
W_MAX = 7


def new_idx_offset():
    # Random number for idx_offset
    # Low prob. of selecting 0 (10%)
    # High prob of selecting -2,2 (45% each)
    n = np.random.randint(256)
    if n < 24:
      idx_offset = 0
    elif n < 140:
      idx_offset = -2
    else:
      idx_offset = 2
    return idx_offset


def new_w_offset():
    # Random number for w_offset
    # Low prob. of selecting -1 (10%)
    # High prob of selecting 1,0 (45% each)
    n = np.random.randint(256)
    if n < 24:
      w_offset = -1
    elif n < 140:
      w_offset = 1
    else:
      w_offset = 0
    return w_offset


tmap = np.zeros((18, MAP_LENGTH))
idx = 7
w = W_MIN

# First column
tmap[idx:idx+w, 0] = 1

for i in range(1, MAP_LENGTH):
  idx_offset = 0
  w_offset = 0

  n = np.random.randint(256)
  if n < 24:
    # Alter both the location and width
    idx_offset = new_idx_offset()
    w_offset = new_w_offset()
  elif n < 140:
    # Alter the location
    idx_offset = new_idx_offset()
  else: 
    # Alter the width
    w_offset = new_w_offset()
    
  idx += idx_offset
  w += w_offset
  
  if idx < 1:
    idx = 1
  elif idx > (15 - W_MIN):
    idx = (15 - W_MIN)
  
  if w < W_MIN:
    w = W_MIN
  elif w > W_MAX:
    w = W_MAX
  
  # Write the new column to array
  tmap[idx:idx+w, i] = 1


plt.imshow(tmap, vmax=1)
plt.show()

print("static const uint8_t biome_columns[BIOME_COUNT][COLUMNS_PER_BIOME * 2] = {")
for screen in range(MAP_LENGTH//20):
  s = "  {"
  for col in range(20):
    idxs = np.argwhere(tmap[:,screen*20+col] != 0).flatten()
    if idxs.shape[0] != 0:
      s += f"{idxs[0]}, {idxs[-1]}, "
  s = s[:-2] + "},"
  print(s)
print("};")
print()

print("static const uint8_t next_possible_biomes[BIOME_COUNT][1] = {")
for screen in range(MAP_LENGTH//20):
  s = "{"
  if screen == (MAP_LENGTH//20 - 1):
    s += "0},"
  else:
    s += f"{screen+1}"
    s += "},"
  print(s)
print("};")
