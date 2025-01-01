import os
import numpy as np
from PIL import Image
import matplotlib.pyplot as plt

""" 
This code converts tiles with the following palettes 
to a C array suitable for GBDK 2020.

https://gbdev.io/pandocs/Tile_Data.html

Sprites: RGB
  R,G,B        Palette color    GB Color Code
-----------------------------------------------
101,255,0       Transparent           0  (356)
224,248,207      Light Green          1  (679)
134,192,108      Dark Green           2  (434)
7,24,33            Black              3  (64)

Backgrounds: RGB
  R,G,B        Palette color    GB Color Code
-----------------------------------------------
224,248,207      Light Green          0  (679)
134,192,108      Medium Green         1  (434)
48,104,50        Dark Green           2  (202)
7,24,33            Black              3  (64)

Indexed PNGs (Sprites & Backgrounds)
AsepriteValue      Palette color        GB Color Code
-----------------------------------------------------
      0             Transparent             0   (sprites)
      1             Light Green            1/0  (sprites/bkgs)
      2             Medium Green           2/1  (sprites/bkgs)
      3             Dark Green              2   (bkgs)
      4             Black                   3   (sprites/bkgs)

"""
def convert_tile_to_vram_data(tile, gb_code, debug=False):
    s = ""
    c_array = ""
    for row in range(tile.shape[0]):
        msb = '0b'
        lsb = '0b'
        for col in range(tile.shape[1]):
            try:
              code = gb_code[tile[row,col]]
            except:
                # Find closes key to value
                value = tile[row, col]
                keys = np.array(list(gb_code.keys()))
                key_ind = np.argmax(1/np.abs(keys - value))
                code = gb_code[keys[key_ind]]

            msb += str((code & 0x2) >> 1)
            lsb += str((code & 0x1))

        msb = int(msb, 2)
        lsb = int(lsb, 2)

        if lsb < 16:
          s += hex(lsb).replace("0x", "0")
          c_array += hex(lsb).replace("0x", "0x0").upper() + ","
        else:  
          s += hex(lsb).replace("0x", "")
          c_array += hex(lsb).upper() + ","

        s += " "

        if msb < 16:
          s += hex(msb).replace("0x", "0")
          c_array += hex(msb).replace("0x", "0x0").upper() + ","
        else:  
          s += hex(msb).replace("0x", "")
          c_array += hex(msb).upper() + ","

        s += " "
    if debug:
       print(s)

    return c_array 


def get_sprite_array(im, array_name, gb_code, debug=False):
    ntiles = int(im.size/(8*8))  # Total number of 8x8 tiles = npixels/64
    stride = int(im.shape[1]/8)

    c_array = f"unsigned char {array_name}_data[] = \n"
    c_array += "{\n"
    for i in range(ntiles):
        c_array += "  "
        # Grab the first 8x8 tile
        irow = int(i / stride)
        icol = i % stride

        tile = im[8*irow:8*(irow+1),8*icol:8*(icol+1)]
        c_array += convert_tile_to_vram_data(tile, gb_code, debug)

        c_array += "\n"

    c_array = c_array[:-2] + "\n};"

    return c_array

def get_background_data_and_map(im, name, gb_code, offset=37, debug=False):
    """
    Split the image into 8x8 tiles.

    Only supports backgrounds of size 160x144 (1 screen)

    The offset parameter defaults to 37 because the map will come after the 
    font, which is 37 tiles. This only affects the _map array.

    We need to generate 2 files:
        (1) Pixel information for all unique tiles that make up the background.
        (2) Map that contains the tile index from (1) to construct the map in the PNG


    Approach:
        (1) Create a dictionary to hold the unique tile id (utid) as keys and the tile array as data
        (2) Grab tile 0 and add it to the unique tiles dictionary with utid = 0
        (3) Iterate over the remaining tiles and find duplicates, if any. 
            (a) Add the tile index of any matches to a skip_index list
        (4) For the remaining tiles (1 through N), 
            (a) Check if current tile index is in skip_index list
                 (i) Skip if tile index in skip_index
                (ii) Repeat Steps (2) - (4)

    """
    ntiles = int(im.size/(8*8))  # Total number of 8x8 tiles = npixels/64
    stride = int(im.shape[1]/8)

    skip_inds = []
    unique_tiles = dict()
    tileind_map = np.ones(ntiles)*np.nan  # Contains the utid for each tile in the map
    utid = 0

    for i in range(ntiles):
      if i in skip_inds:
          continue

      irow = int(i / stride)
      icol = i % stride
      tile = im[8*irow:8*(irow+1),8*icol:8*(icol+1)]
      unique_tiles[utid] = tile
      tileind_map[i] = utid

      for j in range(i+1, ntiles):
          if j in skip_inds:
              continue
          
          irow2 = int(j / stride)
          icol2 = j % stride
          tile2 = im[8*irow2:8*(irow2+1),8*icol2:8*(icol2+1)]

          if np.sum(tile == tile2) == 64:
              # Identical
              skip_inds.append(j)
              tileind_map[j] = utid

      utid += 1

    if debug:
        print(tileind_map)

    # Construct tile data and tile map
    tilemap = np.zeros((8, 8*len(unique_tiles)))
    for i in range(len(unique_tiles)):
        tilemap[:, 8*i:8*(i+1)] = unique_tiles[i]       
    
    tile_data_array = get_sprite_array(tilemap, f"{filename}", gb_code, debug=True)
    tile_data_array = tile_data_array.replace("data", "tiles")

    tile_data_array = f"/*\nNumber of tiles: {len(unique_tiles)}\n*/\n\n{tile_data_array}"

    if debug:
        print(tile_data_array)

    tilemap_array = f"/*\nTile map size: {stride}x{int(ntiles/stride)}\n*/\n\n"
    tilemap_array += f"unsigned char {filename}_ntiles = {len(unique_tiles)};\n"
    tilemap_array += f"unsigned char {filename}_map[] = \n"
    tilemap_array += "{"
    for i in range(ntiles):
        if i % 20 == 0:
           tilemap_array += "\n  "
           
        code = int(tileind_map[i]) + offset
        msb = code >> 4
        lsb = (code & 0xF)

        tilemap_array += hex(msb).upper()
        tilemap_array += hex(lsb).replace("0x", "").upper() + ","
    
    tilemap_array = tilemap_array[:-1] + "\n};"
    if debug:
        print(tilemap_array)

    if debug:
        tilemap = np.zeros((8, 8*len(unique_tiles)))
        for i in range(len(unique_tiles)):
            tilemap[:, 8*i:8*(i+1)] = unique_tiles[i]       

        fig, ax = plt.subplots(1,2)
        ax[0].imshow(im)
        ax[1].imshow(tilemap)

        xticks = np.arange(-0.5,im.shape[1]-0.5,8)
        yticks = np.arange(-0.5,im.shape[0]-0.5,8)
        ax[0].set_xticks(xticks)
        ax[0].set_yticks(yticks)
        ax[0].grid(color="#ff0000")

        xticks = np.arange(-0.5,8*len(unique_tiles)-0.5,8)
        yticks = np.arange(-0.5,7.5,8)
        ax[1].set_xticks(xticks)
        ax[1].set_yticks(yticks)
        ax[1].grid(color="#ff0000")
        
        plt.grid()
        plt.show()

    return tile_data_array, tilemap_array

#fn_path = "../assets/block.png"
#fn_path = "../assets/projectiles.png"
#fn_path = "../assets/powerups.png"
#fn_path = "../assets/player.png"
#fn_path = "../assets/player_shield.png"
#fn_path = "../assets/progressbar.png"
#fn_path = "../assets/font-extras.png" 
#fn_path = "../assets/lock.png"

fn_path = "../assets/tutorial-screen.png" 
tilemap_offset = 59 #37

fn_path = "../assets/cailan-games-logo.png" 
tilemap_offset = 68

filename = os.path.splitext(os.path.split(fn_path)[-1])[0].lower()
filename = filename.replace("-","_")
filename = filename.replace(" ","_")

gb_code_sprite_rgb = {356:0, 679:1, 434:2, 64:3}  # GBSutdio sprite pallete
gb_code_bkg_rgb = {679:0, 434:1, 202:2, 64:3}  # GBSutdio bkg pallete
gb_code_sprite_indexed = {0:0, 1:1, 2:2, 4:3}  # GBSutdio sprite pallete
gb_code_bkg_indexed = {1:0, 2:1, 3:2, 4:3}  # GBSutdio bkg pallete

im = np.asarray(Image.open(fn_path))

if len(im.shape) > 2:
    # RGB image with alpha channel
    # Drop alpha channel and reduce to 2D by summing the RGB channels
    im = im[:,:,:3]
    im = im.sum(axis=2)

    # Look for the RGB sum of 356 to assign the sprite dictionary (transparency color)
    if len(np.argwhere(im == 356)) > 0:
      image_type = "sprite"
      gb_code = gb_code_sprite_rgb
    else:
      image_type = "background"
      gb_code = gb_code_bkg_rgb

else:
    # Aseprite Indexed PNG
    # Look for the Aseprite index 3 to assign the background dictionary
    if len(np.argwhere(im == 3)) > 0:
      image_type = "background"
      gb_code = gb_code_bkg_indexed
    else:
      image_type = "sprite"
      gb_code = gb_code_sprite_indexed

if image_type == "sprite":
    array_string = get_sprite_array(im, filename, gb_code, debug=True)
    header = array_string.split("\n")[0].replace(" =", ";")
    print(header, array_string)
    
    with open(f"../src/{filename}_sprites.h", "w") as f:
        f.write(array_string)

else:
    background_tiles, background_map = get_background_data_and_map(im, filename, gb_code, debug=True, offset=tilemap_offset)
    tiles_header = background_tiles.split("\n")[0].replace(" =", ";")
    map_header = background_map.split("\n")[0].replace(" =", ";")
    print(tiles_header, background_tiles, map_header, background_map)

    with open(f"../src/{filename}_tiles.h", "w") as f:
        f.write(background_tiles)
    with open(f"../src/{filename}_map.h", "w") as f:
        f.write(background_map)

# ntiles = int(im.size/(8*8))  # Total number of 8x8 tiles = npixels/64
# rowtiles = int(im.shape[0]/8)
# coltiles = int(im.shape[1]/8)

# s = ""
# c_array = f"unsigned char {filename}_data[] = "
# c_array += "{\n"

# for i in range(ntiles):
  # c_array += "  "
  # # Grab the first 8x8 tile
  # irow = int(i / coltiles)
  # icol = i % coltiles

  # tile = im[8*irow:8*(irow+1),8*icol:8*(icol+1)]

  # # Print the values in the tile
  # for row in range(tile.shape[0]):
      # msb = '0b'
      # lsb = '0b'
      # for col in range(tile.shape[1]):
          # try:
            # code = gb_code[tile[row,col]]
          # except:
              # # Find closes key to value
              # value = tile[row, col]
              # keys = np.array(list(gb_code.keys()))
              # key_ind = np.argmax(1/np.abs(keys - value))
              # code = gb_code[keys[key_ind]]

          # msb += str((code & 0x2) >> 1)
          # lsb += str((code & 0x1))

      # msb = int(msb, 2)
      # lsb = int(lsb, 2)

      # if lsb < 16:
        # s += hex(lsb).replace("0x", "0")
        # c_array += hex(lsb).replace("0x", "0x0").upper() + ","
      # else:  
        # s += hex(lsb).replace("0x", "")
        # c_array += hex(lsb).upper() + ","

      # s += " "

      # if msb < 16:
        # s += hex(msb).replace("0x", "0")
        # c_array += hex(msb).replace("0x", "0x0").upper() + ","
      # else:  
        # s += hex(msb).replace("0x", "")
        # c_array += hex(msb).upper() + ","

      # s += " "
  # s += "\n"
  # c_array = c_array + "\n"

# c_array = c_array[:-1] + "\n}"
# print(s)
# print(c_array)
