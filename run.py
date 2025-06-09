import uuid

import torch, numpy as np, matplotlib.pyplot as plt
from PIL import Image
from segment_anything import sam_model_registry, SamAutomaticMaskGenerator
import clip

import dbtest

device = "cpu"

# ────────────────────────────── 2. Load models ─────────────────────────
sam = sam_model_registry["vit_b"](checkpoint="sam_vit_b.pth").to(device)
mask_gen = SamAutomaticMaskGenerator(
    sam,
    points_per_side=32, points_per_batch=64,
    pred_iou_thresh=0.90, stability_score_thresh=0.90,
)
clip_model, clip_preprocess = clip.load("RN50", device=device)

import time
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import os

import threading

import json, pathlib

META_PATH = pathlib.Path(__file__).with_name("last_download.json")

file_queue = []

trials = 0

# shared flag
new_file_detected = threading.Event()

class NewFileHandler(FileSystemEventHandler):
    def on_created(self, event):
        if not event.is_directory:
            file_name = os.path.basename(event.src_path)
            print(file_name)
            print(f"New file detected: {event.src_path}")
            file_queue.append(file_name)
            new_file_detected.set()

def wait_for_new_file(path_to_watch):
    global new_file_detected

    event_handler = NewFileHandler()
    observer = Observer()
    observer.schedule(event_handler, path=path_to_watch, recursive=False)
    observer.start()

    try:
        while True:
            print("Waiting for a new file...")
            new_file_detected.wait()
            new_file_detected.clear()
            
            # Your function goes here
            file_name = file_queue.pop(0)
            print("Processing the new file...")
            your_function(file_name)

    finally:
        observer.stop()
        observer.join()

def your_function(file_name):
    try:
        time.sleep(0.1)
        with META_PATH.open() as f:
            m = json.load(f)
        if (m.get('filename') == file_name):
            category = m.get('category')

        text_prompt = clip.tokenize(["a "+category]).to(device)

        # ───────────────────── 3. pick an image ───────────────────────────
        img_path = "downloads/"+file_name
        img_orig = Image.open(img_path).convert("RGB")

        # Down-sample so the short side ≤ 512 px  (saves GPU RAM)
        max_short = 512
        w, h      = img_orig.size
        scale     = min(max_short / w, max_short / h, 1.0)
        img_small = img_orig.resize((int(w*scale), int(h*scale)), Image.LANCZOS)
        img_arr   = np.array(img_small)                      # H×W×3  np.uint8

        # ───────────────────── 4. generate masks ─────────────────────────
        masks = mask_gen.generate(img_arr)
        print(f"Generated {len(masks)} raw masks")

        # ───────────────────── 5. filter by area ─────────────────────────
        H, W      = img_arr.shape[:2]
        total_px  = H * W
        good      = []                     # (idx, mask-dict) pairs we keep
        for i, m in enumerate(masks):
            frac = m["segmentation"].sum() / total_px
            if 0.05 < frac < 0.80:         # keep 5–80 % of image
                good.append((i, m))
        print(f"Kept {len(good)} masks after area filter")

        # ───────────────────── 6. CLIP-score survivors ───────────────────
        best_idx, best_score, best_mask = None, -1, None
        for i, m in good:
            x, y, ww, hh = m["bbox"]
            crop = img_small.crop((x, y, x+ww, y+hh))        # <-- raw crop, no masking
            with torch.no_grad():
                score = clip_model(
                    clip_preprocess(crop).unsqueeze(0).to(device),
                    text_prompt
                )[0].item()

            if score > best_score:
                best_idx, best_score, best_mask = i, score, m["segmentation"]

        # ───────────────────── 7. upscale mask to original ───────────────
        mask_full = Image.fromarray(best_mask.astype(np.uint8)*255).resize(
            img_orig.size, Image.NEAREST
        )
        mask_bin = np.array(mask_full) > 127   # boolean H×W  in ORIGINAL res

        '''
        np.save("best_mask.npy", mask_bin)
        print("✅  saved binary mask → best_mask.npy")

        mask_bin = np.load("best_mask.npy")
        '''

        # mask_bin  : bool  H×W  (True inside T-shirt)
        # img_orig  : PIL Image in the same resolution

        mask = mask_bin.astype(np.uint8)           # 1 = inside mask, 0 = outside
        H, W = mask.shape

        # ── histogram-based maximal all-ones rectangle ──────────────────────
        heights     = np.zeros(W, dtype=int)       # running column heights of 1's
        best_area   = 0
        best_coords = None                         # (x0, y0, x1, y1)  (x1,y1 exclusive)

        for y in range(H):
            # update vertical histogram of consecutive 1's
            heights = np.where(mask[y], heights + 1, 0)

            stack = []                             # indices with increasing height
            x = 0
            while x <= W:                          # <=  W to flush stack at row end
                cur_h = heights[x] if x < W else 0
                if not stack or cur_h >= heights[stack[-1]]:
                    stack.append(x)
                    x += 1
                else:
                    top = stack.pop()
                    width = x if not stack else x - stack[-1] - 1
                    area  = heights[top] * width
                    if area > best_area:
                        best_area = area
                        x1 = (stack[-1] + 1) if stack else 0
                        x2 = x                       # exclusive
                        y2 = y + 1                   # exclusive
                        y1 = y2 - heights[top]
                        best_coords = (x1, y1, x2, y2)

        x0, y0, x1, y1 = best_coords
        print(f"Best inscribed rectangle  :  ({x0},{y0})–({x1-1},{y1-1})  "
              f"size {x1-x0}×{y1-y0}  area {best_area}")

        # ── crop & save / show ──────────────────────────────────────────────
        crop = img_orig.crop((x0, y0, x1, y1))     # PIL expects (left, upper, right, lower)
        crop.save("output/crop.png")
        crop.show()

        print("Saved → crop.png")
        random_id = str(uuid.uuid4())
        print(random_id)

        dbtest.upload_file("output/crop.png", category=category, garment_id = random_id)
        print('uploaded to db')
    except Exception as e:
        print(e)
        your_function(file_name)


# Example usage
wait_for_new_file("downloads")
