#!/usr/bin/env python3
"""Publish workshop examples into the Jekyll site.

Jekyll does not list directories, so a link to an example folder is a 404 unless
that folder has a page. This copies the sources through as raw files, writes an
index for every folder, and adds a zip of each project.
"""

import html
import shutil
import zipfile
from pathlib import Path
from urllib.parse import quote

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "examples"
SITE_ROOT = ROOT / "_site"
DEST = SITE_ROOT / "examples"

SKIP_DIRS = {"build", "managed_components", ".git"}
SKIP_FILES = {"sdkconfig", "sdkconfig.old"}


def included(path: Path, root: Path) -> bool:
    relative = path.relative_to(root)
    if any(part in SKIP_DIRS for part in relative.parts):
        return False
    return path.name not in SKIP_FILES


def project_files(root: Path):
    for path in sorted(root.rglob("*")):
        if path.is_file() and included(path, root):
            yield path


def copy_tree() -> None:
    if DEST.exists():
        shutil.rmtree(DEST)
    DEST.mkdir(parents=True)
    for path in project_files(SOURCE):
        target = DEST / path.relative_to(SOURCE)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, target)


def write_zip(src: Path, dest: Path, arc_root: str) -> None:
    dest.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(dest, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for path in project_files(src):
            arcname = f"{arc_root}/{path.relative_to(src).as_posix()}"
            info = zipfile.ZipInfo.from_file(path, arcname)
            info.compress_type = zipfile.ZIP_DEFLATED
            with path.open("rb") as raw:
                archive.writestr(info, raw.read())


def write_archives() -> None:
    for chip_name in ("C5", "C6"):
        chip = SOURCE / chip_name
        if not chip.is_dir():
            continue
        write_zip(chip, DEST / f"{chip_name}.zip", chip_name)
        for project in sorted(chip.iterdir()):
            if project.is_dir() and (project / "CMakeLists.txt").is_file():
                write_zip(project, DEST / chip_name / f"{project.name}.zip", project.name)


def page(title: str, body: str) -> str:
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{html.escape(title)}</title>
  <link rel="stylesheet" href="/assets/css/style.css">
</head>
<body>
  <header>
    <div class="container">
      <a id="a-title" href="/"><h1>Honza0297.github.io</h1></a>
    </div>
  </header>
  <div class="container">
    <section id="main_content">
      <h1>{html.escape(title)}</h1>
      {body}
    </section>
  </div>
</body>
</html>
"""


def href(name: str) -> str:
    return quote(name)


def write_indexes() -> None:
    directories = [DEST, *sorted(path for path in DEST.rglob("*") if path.is_dir())]
    for directory in directories:
        relative = directory.relative_to(DEST)
        title = "examples" if relative == Path(".") else f"examples/{relative.as_posix()}"
        rows = []
        if directory != DEST:
            rows.append('<li><a href="../">..</a></li>')

        downloads = []
        chip_zip = directory.parent / f"{directory.name}.zip"
        if directory != DEST and chip_zip.is_file():
            downloads.append(
                f'<a href="../{href(chip_zip.name)}">{html.escape(chip_zip.name)}</a>'
            )

        entries = sorted(directory.iterdir(), key=lambda path: (path.is_file(), path.name.lower()))
        for entry in entries:
            if entry.name == "index.html" or entry.suffix == ".zip":
                continue
            label = entry.name + ("/" if entry.is_dir() else "")
            link = f'<a href="{href(entry.name)}{"/" if entry.is_dir() else ""}">{html.escape(label)}</a>'
            zip_path = directory / f"{entry.name}.zip"
            if entry.is_dir() and zip_path.is_file():
                link += f' — <a href="{href(zip_path.name)}">{html.escape(zip_path.name)}</a>'
            rows.append(f"<li>{link}</li>")

        download_html = ""
        if downloads:
            download_html = "<p>Download: " + ", ".join(downloads) + "</p>\n"
        body = download_html + "<ul>\n" + "\n".join(rows) + "\n</ul>\n"
        (directory / "index.html").write_text(page(title, body), encoding="utf-8")


def main() -> None:
    if not SOURCE.is_dir():
        raise SystemExit(f"missing examples directory: {SOURCE}")
    if not SITE_ROOT.is_dir():
        raise SystemExit("missing _site; run 'bundle exec jekyll build' first")
    copy_tree()
    write_archives()
    write_indexes()
    print(f"published examples to {DEST}")


if __name__ == "__main__":
    main()
