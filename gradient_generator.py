import numpy as np
from PIL import Image
from math import floor, ceil

RESOLUTION = 128
GRADIENT_COLORS = [[0, 255, 0], [255, 0, 0], [255, 0, 255], [0, 0, 0]] # green -> red -> violet -> black
data = np.zeros((1, RESOLUTION, 3), dtype=np.uint8)
for i in range(RESOLUTION):
    color_factor = i / RESOLUTION * (len(GRADIENT_COLORS)-1)
    left_color_idx = floor(color_factor)
    right_color_idx = ceil(color_factor)
    interpolation_factor = (color_factor - left_color_idx)
    #print(interpolation_factor)
    color = [255, 255, 255]
    for j in range(3):
        colorchannel_left = GRADIENT_COLORS[left_color_idx][j]
        colorchannel_right = GRADIENT_COLORS[right_color_idx][j]
        colorchannel = (1-interpolation_factor)*colorchannel_left + interpolation_factor*colorchannel_right
        color[j] = int(colorchannel)
    print(color)
    data[0, i] = color
img = Image.fromarray(data)
img.save("gradient.png")
