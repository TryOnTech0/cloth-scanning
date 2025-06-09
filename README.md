# Cloth-Scanning Module

This module gets the image of a garment from database (which taken from the mobile app and sent to there), then masks it with clip model and crops the biggest box inside of that mask. Then the cropped part (texture of the garment) sends to the database again for other modules use.

## How to run it
To run this module there is only one step if it is going to be run seperately from the other modules.

1. python run.py

This will load the model and will wait for a new file in the downloads directory. When there is a new file it instantly starts the process and generates its output to the output directory. (You can simply drag an image file inside of the downloads directory, it will start the process.)

To run it with connected to other modules, there are more steps.

1. python run.py
2. node watcher.js
3. node server/src/server.js

First two files are already in this module but the third one (server.js) is a file of the server module and should be installed from that repository under this organization. After these steps module is waiting for a new file under the specified folder in database. When image is taken in mobile it sends it to database and the process starts.

### Requirements
It requires python3 and node for starting the programs. And the libraries required is like these:

For Python
1. torch
2. numpy
3. matplotlib
4. Pillow
5. segment-anything
6. clip-by-openai
7. watchdog

For Node
1. mongodb
2. axios
