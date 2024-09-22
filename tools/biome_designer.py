import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from matplotlib.collections import PatchCollection

gaps = np.zeros((20)*(18))
canvas = np.zeros((144, 160))
stride = 20  # Line width in tile index

def clear_collision_blocks():
    global gaps, ax, stride
    size = 8
    
    # Remove all collections from ax
    for c in ax.collections:
       c.remove()

    rectangles = []
    
    for ind in np.argwhere(gaps == 0).flatten():
        # Convert ind to x,y in tile space
        y = int(ind/stride) 
        x = ind % stride

        # Convert x,y into real space
        x *= 8
        y *= 8

        # Draw a red rectangle with alpha 0.25
        rectangles.append(Rectangle((x,y), size, size))

    pc = PatchCollection(rectangles, alpha=0)
    ax.add_collection(pc)

def mark_collision_blocks():
    global gaps, ax, stride
    size = 8
    
    # Remove all collections from ax
    for c in ax.collections:
       c.remove()

    rectangles = []
    
    for ind in np.argwhere(gaps != 0).flatten():
        # Convert ind to x,y in tile space
        y = int(ind/stride) 
        x = ind % stride

        # Convert x,y into real space
        x *= 8
        y *= 8

        # Draw a red rectangle with alpha 0.25
        rectangles.append(Rectangle((x,y), size, size))

    pc = PatchCollection(rectangles, facecolor="#ffffff", edgecolor="#000000")
    ax.add_collection(pc)

def onclick(event):
    global gaps, stride, fig

    if event.button != 1:
      return
    # Get click location and convert to tile index in x,y
    x,y = event.xdata, event.ydata
    # print(x,y)
    x = int(x/8)  # [0, 20]
    y = int(y/8)  # [0, 18]
    # print(x,y)

    # Convert tile index to flat array index
    ind = stride*y + x 
    # print(ind)

    if gaps[ind] == 0:
      # Flag location as a collision block
      gaps[ind] = 1
      mark_collision_blocks()

    else:
      gaps[ind] = 0
      clear_collision_blocks()
      mark_collision_blocks()

    fig.canvas.draw()


fig, ax = plt.subplots(1,1)
ax.imshow(canvas, cmap="Greys_r")

# Set up tile grid
xticks = np.arange(0,160,8)
yticks = np.arange(0,144,8)
ax.set_xticks(xticks)
ax.set_yticks(yticks)
ax.grid(color="#ff0000")

fig.canvas.mpl_connect("button_press_event", onclick)

plt.show()

gaps = np.resize(gaps, (18,20))
s = "{"
for col in range(20):
  idxs = np.argwhere(gaps[:,col] != 0).flatten()
  if idxs.shape[0] != 0:
    s += f"{idxs[0]}, {idxs[-1]}, "
          
s = s[:-2] + "}"

print(s)
    