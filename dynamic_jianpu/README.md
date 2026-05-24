# Dynamic Jianpu Video Template

This template reads `notes.json` and generates a dynamic jianpu MP4.

## 1) Install dependencies

```bash
cd /home/zhenhtan/workspace/l1low/leaningProgram/dynamic_jianpu
python -m pip install -r requirements.txt
```

System `ffmpeg` is not required. The script uses `imageio-ffmpeg` bundled binary by default.

## 2) Put your audio file

Place your audio file in this folder, for example `song.mp3`.
Then update `notes.json` field:

```json
"audio": "song.mp3"
```

## 3) Edit notes.json

Each note item needs:
- `text`: jianpu symbol (e.g. `1`, `2`, `5`, `1'`, `-`, `|`)
- `start`: note start time (seconds)
- `end`: note end time (seconds)
- `line`: row index (0, 1, 2...)

Example:

```json
{ "text": "5", "start": 0.0, "end": 0.5, "line": 0 }
```

## 4) Generate video

```bash
python generate_jianpu_video.py --config notes.json
```

Or specify output path:

```bash
python generate_jianpu_video.py --config notes.json --output out.mp4
```

## Notes

- Current note is highlighted and lightly pulsed.
- A moving cursor is shown below the active note.
- Horizontal positions are automatically distributed by note count in each line.
- If you want custom font, add in `video`:

```json
"font_path": "/path/to/your/font.ttf"
```
