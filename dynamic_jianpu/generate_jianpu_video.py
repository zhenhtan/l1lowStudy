import argparse
import json
import math
import os
from pathlib import Path
from typing import Dict, List, Tuple

import imageio_ffmpeg
import numpy as np
from PIL import Image, ImageDraw, ImageFont
from moviepy.editor import AudioFileClip, VideoClip


Color = Tuple[int, int, int]


def configure_embedded_ffmpeg() -> None:
    # Force moviepy/imageio to use bundled ffmpeg so system ffmpeg is not required.
    if not os.environ.get("IMAGEIO_FFMPEG_EXE"):
        os.environ["IMAGEIO_FFMPEG_EXE"] = imageio_ffmpeg.get_ffmpeg_exe()


def clamp(v: float, low: float, high: float) -> float:
    return max(low, min(high, v))


def load_json(path: Path) -> Dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def load_font(font_size: int, font_path: str = "") -> ImageFont.FreeTypeFont:
    if font_path:
        return ImageFont.truetype(font_path, font_size)

    for candidate in [
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    ]:
        if Path(candidate).exists():
            return ImageFont.truetype(candidate, font_size)

    return ImageFont.load_default()


def group_notes_by_line(notes: List[Dict]) -> Dict[int, List[Dict]]:
    lines: Dict[int, List[Dict]] = {}
    for note in notes:
        line = int(note.get("line", 0))
        lines.setdefault(line, []).append(note)

    for line_idx in lines:
        lines[line_idx].sort(key=lambda x: (float(x["start"]), float(x["end"])))
    return lines


def assign_layout(notes: List[Dict], cfg: Dict) -> List[Dict]:
    width = int(cfg["video"]["width"])
    left_margin = int(cfg["video"].get("left_margin", 120))
    right_margin = int(cfg["video"].get("right_margin", 120))
    top_margin = int(cfg["video"].get("top_margin", 180))
    line_gap = int(cfg["video"].get("line_gap", 120))

    lines = group_notes_by_line(notes)
    laid_out: List[Dict] = []

    for line_idx in sorted(lines.keys()):
        line_notes = lines[line_idx]
        span = max(1, len(line_notes) - 1)
        row_width = width - left_margin - right_margin
        y = top_margin + line_idx * line_gap

        for i, note in enumerate(line_notes):
            x = left_margin + int(row_width * (i / span))
            copied = dict(note)
            copied["x"] = x
            copied["y"] = y
            copied["start"] = float(copied["start"])
            copied["end"] = float(copied["end"])
            laid_out.append(copied)

    laid_out.sort(key=lambda x: (x["start"], x["line"], x["x"]))
    return laid_out


def find_active_index(notes: List[Dict], t: float) -> int:
    for idx, note in enumerate(notes):
        if note["start"] <= t < note["end"]:
            return idx
    if t >= notes[-1]["end"]:
        return len(notes) - 1
    return -1


def draw_frame(
    t: float,
    notes: List[Dict],
    cfg: Dict,
    font: ImageFont.FreeTypeFont,
) -> np.ndarray:
    vcfg = cfg["video"]
    width = int(vcfg["width"])
    height = int(vcfg["height"])
    bg_color: Color = tuple(vcfg.get("bg_color", [18, 24, 34]))
    text_color: Color = tuple(vcfg.get("text_color", [230, 236, 245]))
    highlight_color: Color = tuple(vcfg.get("highlight_color", [255, 210, 80]))
    cursor_color: Color = tuple(vcfg.get("cursor_color", [255, 120, 80]))

    image = Image.new("RGB", (width, height), bg_color)
    draw = ImageDraw.Draw(image)

    active_idx = find_active_index(notes, t)

    for idx, note in enumerate(notes):
        x = int(note["x"])
        y = int(note["y"])
        text = str(note["text"])

        color = text_color
        scale = 1.0

        if idx == active_idx:
            color = highlight_color
            duration = max(0.001, note["end"] - note["start"])
            phase = (t - note["start"]) / duration
            pulse = 0.08 * math.sin(phase * math.pi)
            scale = 1.0 + clamp(pulse, -0.05, 0.10)

        if abs(scale - 1.0) > 1e-3:
            scaled_font_size = max(12, int(vcfg.get("font_size", 72) * scale))
            local_font = load_font(scaled_font_size, vcfg.get("font_path", ""))
        else:
            local_font = font

        bbox = draw.textbbox((0, 0), text, font=local_font)
        tw = bbox[2] - bbox[0]
        th = bbox[3] - bbox[1]
        draw.text((x - tw // 2, y - th // 2), text, fill=color, font=local_font)

    if active_idx >= 0:
        an = notes[active_idx]
        cx = int(an["x"])
        cy = int(an["y"] + int(vcfg.get("font_size", 72) * 0.75))
        draw.line((cx, cy - 18, cx, cy + 18), fill=cursor_color, width=4)
        draw.ellipse((cx - 6, cy + 18, cx + 6, cy + 30), fill=cursor_color)

    return np.array(image)


def build_video(config_path: Path, output_path: Path = None) -> Path:
    cfg = load_json(config_path)
    notes = cfg.get("notes", [])
    if not notes:
        raise ValueError("notes.json has no notes.")

    notes = assign_layout(notes, cfg)

    audio_path = Path(cfg["audio"])
    if not audio_path.is_absolute():
        audio_path = (config_path.parent / audio_path).resolve()
    if not audio_path.exists():
        raise FileNotFoundError(f"Audio not found: {audio_path}")

    out = output_path or Path(cfg.get("output", "jianpu_output.mp4"))
    if not out.is_absolute():
        out = (config_path.parent / out).resolve()

    fps = int(cfg["video"].get("fps", 30))
    font_size = int(cfg["video"].get("font_size", 72))
    font_path = cfg["video"].get("font_path", "")
    font = load_font(font_size, font_path)

    audio_clip = AudioFileClip(str(audio_path))
    duration = max(audio_clip.duration, notes[-1]["end"])

    video = VideoClip(lambda t: draw_frame(t, notes, cfg, font), duration=duration)
    video = video.set_audio(audio_clip)

    video.write_videofile(
        str(out),
        fps=fps,
        codec="libx264",
        audio_codec="aac",
        threads=4,
        preset="medium",
    )

    audio_clip.close()
    video.close()
    return out


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate a dynamic jianpu (numbered musical notation) MP4 from notes.json."
    )
    parser.add_argument(
        "--config",
        type=Path,
        default=Path("notes.json"),
        help="Path to notes.json",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help="Optional output mp4 path",
    )
    return parser.parse_args()


def main() -> None:
    configure_embedded_ffmpeg()
    args = parse_args()
    output = build_video(args.config.resolve(), args.output)
    print(f"Generated: {output}")


if __name__ == "__main__":
    main()
