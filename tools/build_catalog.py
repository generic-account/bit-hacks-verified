#!/usr/bin/env python3
"""
Builds Jekyll collection entries from Coq/Rocq sources and a search index.

- Scans src/**.v
- Extracts YAML-like header inside the first Coq comment block containing --- ... ---
- Derives categories from folder path
- Emits site/_proofs/<slug>.md with front matter + code block
- Emits site/search.json with {title, summary, tags, url, path}
- Emits simple per-tag pages at site/tags/<tag>/index.md (layout: tag)

Requires: pyyaml
"""
import re
import json
import yaml
import html
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[1]
SRC = ROOT / "src"
SITE = ROOT / "site"
OUT = SITE / "_proofs"
SEARCH_JSON = SITE / "search.json"
TAGS_DIR = SITE / "tags"

HEADER_RE = re.compile(r"\(\*\s*---(.*?)---\s*\*\)", re.S | re.M)

def slugify(s: str) -> str:
    import unicodedata
    s = unicodedata.normalize("NFKD", s)
    s = s.encode("ascii", "ignore").decode("ascii")
    s = s.lower()
    s = re.sub(r"[^a-z0-9]+", "-", s).strip("-")
    return s or "item"

def extract_header_and_body(text: str):
    """
    Looks for a Coq block comment containing front-matter:
    (*
    ---
    title: ...
    ...
    ---
    *)
    Returns (meta_dict, remaining_source_without_header)
    """
    m = HEADER_RE.search(text)
    meta = {}
    if m:
        chunk = m.group(1)
        meta = yaml.safe_load(chunk) or {}
        text = text[:m.start()] + text[m.end():]
    return meta, text.strip()

def categories_from_relpath(relpath: pathlib.Path):
    if relpath.parent == pathlib.Path('.'):
        return []
    return [p for p in relpath.parent.parts]

def ensure_dir(p: pathlib.Path):
    p.mkdir(parents=True, exist_ok=True)

def write_markdown(proof_out: pathlib.Path, front: dict, code_body: str, lang="coq"):
    fm = "---\n" + yaml.safe_dump(front, sort_keys=False) + "---\n\n"
    # Escape nothing; we want raw code. Fenced block is fine.
    content = f"```{lang}\n{code_body}\n```\n"
    proof_out.write_text(fm + content, encoding="utf-8")

def main():
    ensure_dir(OUT)
    ensure_dir(TAGS_DIR)

    records = []
    all_tags = set()

    for vfile in sorted(SRC.rglob("*.v")):
        rel = vfile.relative_to(SRC)
        raw = vfile.read_text(encoding="utf-8")

        meta, body = extract_header_and_body(raw)

        title = meta.get("title") or rel.stem
        slug = meta.get("slug") or slugify(title)
        tags = meta.get("tags") or []
        summary = meta.get("summary") or ""
        related = meta.get("related") or []
        categories = categories_from_relpath(rel)

        # Build Jekyll front matter
        front = {
            "layout": "proof",
            "title": title,
            "tags": tags,
            "summary": summary,
            "categories": categories,
            "source_path": str(rel).replace("\\", "/"),
            "slug": slug,
            "related": related,
        }
        # Write collection item
        md_path = OUT / f"{slug}.md"
        write_markdown(md_path, front, body, lang="coq")

        # URL matches collection permalink: /proofs/:name/
        url = f"/proofs/{slug}/"
        records.append({
            "title": title,
            "summary": summary,
            "tags": tags,
            "url": url,
            "path": str(rel).replace("\\", "/"),
            "categories": categories,
            "slug": slug,
        })
        for t in tags:
            all_tags.add(t)

    # search.json for client-side search
    SEARCH_JSON.write_text(json.dumps(records, indent=2), encoding="utf-8")

    # emit per-tag pages if missing
    for t in sorted(all_tags):
        tag_dir = TAGS_DIR / slugify(t)
        ensure_dir(tag_dir)
        tag_index = tag_dir / "index.md"
        if not tag_index.exists():
            fm = {
                "layout": "tag",
                "tag": t,
                "title": f"Tag: {t}",
                "permalink": f"/tags/{slugify(t)}/",
            }
            tag_index.write_text(
                "---\n" + yaml.safe_dump(fm, sort_keys=False) + "---\n\n", encoding="utf-8"
            )

    print(f"Generated {len(records)} proofs, {len(all_tags)} tags.")

if __name__ == "__main__":
    main()
