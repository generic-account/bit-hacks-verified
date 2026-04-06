#!/usr/bin/env python3

import html
import json
import pathlib
import re
import tomllib


ROOT = pathlib.Path(__file__).resolve().parents[2]
HACKS_DIR = ROOT / "src" / "hacks"
SITE_DIR = ROOT / "site"
PROOFS_DIR = SITE_DIR / "_proofs"
TAGS_DIR = SITE_DIR / "tags"
SEARCH_JSON = SITE_DIR / "search.json"
INDEX_MD = SITE_DIR / "index.md"

TOP_COMMENT_RE = re.compile(r"\A\s*/\*(.*?)\*/", re.DOTALL)
FUNC_RE_TEMPLATE = r"\bbh_output_t\s+{name}\s*\(\s*bh_input_t\s+\w+\s*\)\s*\{{"


def ensure_dir(path: pathlib.Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def extract_top_comment(text: str) -> str:
    match = TOP_COMMENT_RE.match(text)
    if match is None:
        raise ValueError("missing top metadata comment")
    return match.group(1).strip()


def extract_function(text: str, name: str) -> str:
    match = re.search(FUNC_RE_TEMPLATE.format(name=re.escape(name)), text)
    if match is None:
        raise ValueError(f"missing function {name}")

    start = match.start()
    brace_depth = 0
    index = text.find("{", match.start())
    if index < 0:
        raise ValueError(f"malformed function {name}")

    while index < len(text):
        char = text[index]
        if char == "{":
            brace_depth += 1
        elif char == "}":
            brace_depth -= 1
            if brace_depth == 0:
                return text[start:index + 1].strip()
        index += 1

    raise ValueError(f"unterminated function {name}")


def md_list(items: list[str]) -> str:
    if not items:
        return "- None\n"
    return "".join(f"- {item}\n" for item in items)


def json_list(items: list[str]) -> str:
    return json.dumps(items, ensure_ascii=False)


def clear_generated_tree(path: pathlib.Path) -> None:
    if not path.exists():
        return
    for child in sorted(path.iterdir()):
        if child.is_dir():
            clear_generated_tree(child)
            child.rmdir()
        else:
            child.unlink()


def slugify(text: str) -> str:
    text = text.lower()
    text = re.sub(r"[^a-z0-9]+", "-", text)
    return text.strip("-") or "item"


def render_markdown(metadata: dict, relpath: str, optimized_src: str, reference_src: str) -> str:
    front_matter = {
        "layout": "proof",
        "title": metadata["title"],
        "summary": metadata["summary"],
        "hack_id": metadata["hack_id"],
        "tags": metadata["tags"],
        "search_keywords": metadata["search_keywords"],
        "source_path": relpath,
        "slug": metadata["hack_id"],
    }
    lines = ["---"]
    for key, value in front_matter.items():
        if isinstance(value, list):
            lines.append(f"{key}: {json_list(value)}")
        else:
            lines.append(f'{key}: "{value}"')
    lines.extend(
        [
            "---",
            "",
            "## Summary",
            "",
            metadata["summary"],
            "",
            "## Contract",
            "",
            metadata["contract"],
            "",
            "## Notes",
            "",
            metadata["notes"].strip(),
            "",
            "## Verification",
            "",
            md_list(metadata["verification"]).rstrip(),
            "",
            "## Optimized Implementation",
            "",
            "```c",
            optimized_src,
            "```",
            "",
            "## Reference Implementation",
            "",
            "```c",
            reference_src,
            "```",
            "",
            "## Source File",
            "",
            f"`{relpath}`",
            "",
            "## Sources",
            "",
            md_list(metadata["sources"]).rstrip(),
            "",
        ]
    )
    return "\n".join(lines)


def render_index(records: list[dict]) -> str:
    lines = [
        "---",
        'layout: default',
        'title: Verified Bit Hacks',
        "---",
        "",
        "# Verified Bit Hacks",
        "",
        "Standalone C implementations with differential verification, sanitizers, and fuzz smoke coverage.",
        "",
        "## Hacks",
        "",
    ]
    for record in records:
        summary = record["summary"]
        tags = ", ".join(record["tags"])
        proof_path = record["url"]
        lines.append(
            f'- [{record["title"]}](' + '{{ "' + proof_path + '" | relative_url }}' + ')'
            f'{" — " + summary if summary else ""}'
            f'{" [" + tags + "]" if tags else ""}'
        )
    lines.append("")
    return "\n".join(lines)


def render_tag_index(tag: str) -> str:
    return "\n".join(
        [
            "---",
            "layout: tag",
            f'tag: "{tag}"',
            f'title: "Tag: {tag}"',
            f'permalink: "/tags/{slugify(tag)}/"',
            "---",
            "",
        ]
    )


def main() -> None:
    ensure_dir(PROOFS_DIR)
    ensure_dir(TAGS_DIR)
    clear_generated_tree(PROOFS_DIR)
    clear_generated_tree(TAGS_DIR)

    records = []
    all_tags = set()
    for hack_path in sorted(HACKS_DIR.glob("*.c")):
        text = hack_path.read_text(encoding="utf-8")
        metadata = tomllib.loads(extract_top_comment(text))
        optimized_src = extract_function(text, "bh_optimized")
        reference_src = extract_function(text, "bh_reference")
        relpath = str(hack_path.relative_to(ROOT))

        markdown = render_markdown(metadata, relpath, optimized_src, reference_src)
        md_out = PROOFS_DIR / f"{metadata['hack_id']}.md"
        md_out.write_text(markdown, encoding="utf-8")

        records.append(
            {
                "title": metadata["title"],
                "hack_id": metadata["hack_id"],
                "tags": metadata["tags"],
                "search_keywords": metadata["search_keywords"],
                "summary": metadata["summary"],
                "contract": metadata["contract"],
                "path": relpath,
                "url": f"/proofs/{metadata['hack_id']}/",
            }
        )
        for tag in metadata["tags"]:
            all_tags.add(tag)

    records.sort(key=lambda item: item["title"])
    for tag in sorted(all_tags):
        tag_dir = TAGS_DIR / slugify(tag)
        ensure_dir(tag_dir)
        (tag_dir / "index.md").write_text(render_tag_index(tag), encoding="utf-8")

    INDEX_MD.write_text(render_index(records), encoding="utf-8")
    SEARCH_JSON.write_text(json.dumps(records, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
