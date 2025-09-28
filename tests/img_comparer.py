from io import BytesIO

import numpy as np
from PIL import Image


def load_image_bytes(img_bytes):
    return np.array(Image.open(BytesIO(img_bytes)).convert("RGB"), dtype=np.float32)


def mse(img1, img2):
    return np.mean((img1 - img2) ** 2)
