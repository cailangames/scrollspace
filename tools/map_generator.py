import numpy as np
import matplotlib.pyplot as plt

def new_idx_offset():
    # Random number for idx_offset
    # Low prob. of selecting 0 (10%)
    # High prob of selecging -1,1 (45% each)
    n = np.random.randint(256)
    if n < 24:
      idx_offset = 0
    elif n < (140):
      idx_offset = -2
    else:
      idx_offset = 2

    return idx_offset

def new_w_offset():
    # Random number for w_offset
    # Low prob. of selecting -1 (10%)
    # High prob of selecging 1,0 (45% each)
    n = np.random.randint(256)
    if n < 24:
      w_offset = -1
    elif n < (140):
      w_offset = 1
    else:
      w_offset = 0
    
    return w_offset

def new_obstacle():
  # Generate a random number between 0-256
  # Divide by 14 to get a tile index in the 
  # range 0-18.
  tile_idx = np.random.randint(256) // 14
  
  return tile_idx

map_length = 20*50

tmap = np.zeros((18,map_length))

w_min = 5
w_max = 7
obs_max = w_min // 3
if obs_max == 0:
  obs_max = 1

idx = 7
w = w_min

# First column
tmap[idx:idx+w, 0] = 1

for i in range(1, map_length):
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
  elif idx > (15 - w_min):
    idx = (15 - w_min)
  
  if w < w_min:
    w = w_min
  elif w > w_max:
    w = w_max
  elif w > 18:
    w = 18
  
  # Write the new column to array
  tmap[idx:idx+w, i] = 1

  # # Add obstacle
  # n = np.random.randint(256)
  # if n < 96:
    # tile_idx = new_obstacle()
    # if tile_idx > idx and tile_idx < idx+w:
      # #print(i, tile_idx, idx, idx+w)
      # tmap[tile_idx:tile_idx+obs_max, i] = 0
  

plt.imshow(tmap, vmax=1)
plt.show()

print("static const uint8_t biome_columns[BIOME_COUNT][COLUMNS_PER_BIOME * 2] = {")
for screen in range(map_length//20):
  s = "    {"
  for col in range(20):
    idxs = np.argwhere(tmap[:,screen*20+col] != 0).flatten()
    if idxs.shape[0] != 0:
      s += f"{idxs[0]}, {idxs[-1]}, "
            
  if screen == (map_length//20 - 1):
    s = s[:-2] + "}"
  else:
    s = s[:-2] + "},"
  print(s)
print("};")

print("static const uint8_t next_possible_biomes[BIOME_COUNT][1] = {")
for screen in range(map_length//20):
  s = "{"
  if screen == (map_length//20 - 1):
    s += "0}"
  else:
    s += f"{screen+1}"
    s += "},"
  print(s)
print("};")