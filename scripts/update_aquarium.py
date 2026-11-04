#!/usr/bin/env python3
"""
git-pet Aquarium Orchestrator
Fetches active repositories from GitHub, filters out inactive/old repositories
based on a configurable deadline (in days), and generates a multi-fish aquarium
sized so the tank is never overcrowded.
Exports both the animated GIF and structured JSON data for websites and dashboards.
"""

import os
import sys
import json
import re
import argparse
import subprocess
from datetime import datetime, timezone
import urllib.request
import urllib.error

# Ensure UTF-8 output on Windows consoles
if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

LANGUAGE_COLOR_NAMES = {
    "c": "Slate Grey",
    "c++": "Magenta Pink",
    "cpp": "Magenta Pink",
    "c#": "Forest Green",
    "csharp": "Forest Green",
    "python": "Ocean Blue",
    "py": "Ocean Blue",
    "javascript": "Canary Gold",
    "js": "Canary Gold",
    "typescript": "Sapphire Blue",
    "ts": "Sapphire Blue",
    "rust": "Terracotta Rust",
    "go": "Cyan",
    "golang": "Cyan",
    "html": "Flame Red",
    "css": "Royal Violet",
    "ruby": "Ruby Crimson",
    "java": "Amber Bronze",
    "php": "Indigo",
    "swift": "Sunset Orange",
    "kotlin": "Electric Violet",
    "kt": "Electric Violet",
    "shell": "Lime Green",
    "bash": "Lime Green",
    "sh": "Lime Green",
    "powershell": "Navy Blue",
    "makefile": "Charcoal",
    "lua": "Navy Blue",
    "dart": "Teal",
    "zig": "Peach",
    "markdown": "Deep Ocean",
    "md": "Deep Ocean",
    "r": "Sky Blue",
    "scala": "Crimson",
    "elixir": "Purple",
    "haskell": "Violet",
    "clojure": "Coral",
    "ocaml": "Bright Orange",
    "solidity": "Silver"
}

LANGUAGE_HEX_COLORS = {
    "c": "#555555",
    "c++": "#F34B7D",
    "cpp": "#F34B7D",
    "c#": "#178600",
    "csharp": "#178600",
    "python": "#3572A5",
    "py": "#3572A5",
    "javascript": "#F1E05A",
    "js": "#F1E05A",
    "typescript": "#3178C6",
    "ts": "#3178C6",
    "rust": "#DEA584",
    "go": "#00ADD8",
    "golang": "#00ADD8",
    "html": "#E34C26",
    "css": "#563D7C",
    "ruby": "#701516",
    "java": "#B07219",
    "php": "#4F5D95",
    "swift": "#F05138",
    "kotlin": "#A97BFF",
    "kt": "#A97BFF",
    "shell": "#89E051",
    "bash": "#89E051",
    "sh": "#89E051",
    "powershell": "#012456",
    "makefile": "#555555",
    "lua": "#000080",
    "dart": "#00B4AB",
    "zig": "#EC915C",
    "markdown": "#083FA1",
    "md": "#083FA1",
    "r": "#198CE7",
    "scala": "#C22D26",
    "elixir": "#6E4A7E",
    "haskell": "#5E5086",
    "clojure": "#DB5855",
    "ocaml": "#EE6A1A",
    "solidity": "#AAAAAA"
}

def get_color_name(lang):
    if not lang:
        return "Slate Grey"
    clean = lang.strip().lower()
    return LANGUAGE_COLOR_NAMES.get(clean, f"{lang} Tint")

def get_language_hex(lang):
    if not lang:
        return "#555555"
    clean = lang.strip().lower()
    return LANGUAGE_HEX_COLORS.get(clean, "#3572A5")

def get_fish_appearance(raw_lang_str, stage_str, stars, is_recent):
    """Generate human-readable color identification so users can spot the fish in the GIF."""
    parts = [p.strip() for p in raw_lang_str.split(",") if p.strip()]
    if not parts:
        body_col = "Slate Grey"
        fin_col = "Steel Blue"
        pct_sub = "C 100%"
    elif len(parts) == 1:
        lang1 = parts[0].split(":")[0].strip()
        body_col = get_color_name(lang1)
        fin_col = "Matching"
        pct_sub = parts[0].replace(":", " ") + "%" if ":" in parts[0] else parts[0]
    else:
        lang1 = parts[0].split(":")[0].strip()
        lang2 = parts[1].split(":")[0].strip()
        body_col = get_color_name(lang1)
        fin_col = get_color_name(lang2)
        p1 = parts[0].replace(":", " ") + "%" if ":" in parts[0] else parts[0]
        p2 = parts[1].replace(":", " ") + "%" if ":" in parts[1] else parts[1]
        pct_sub = f"{p1} · {p2}"

    if fin_col == "Matching":
        desc = f"**{body_col} body & fins**"
    else:
        desc = f"**{body_col} body, {fin_col} fins**"

    traits = []
    if "fry" in stage_str.lower() or "baby" in stage_str.lower():
        traits.append("Small Fry")
    if is_recent:
        traits.append("Active Bubble Puff")
    if stars > 0:
        traits.append(f"{stars} Star Sparkle{'s' if stars > 1 else ''}")

    extra_str = f" · {', '.join(traits)}" if traits else ""
    return f"{desc}<br><sub>{pct_sub}{extra_str}</sub>"

def fetch_json(url, token=None):
    headers = {
        "Accept": "application/vnd.github.v3+json",
        "User-Agent": "git-pet-aquarium"
    }
    if token:
        headers["Authorization"] = f"Bearer {token}"
    req = urllib.request.Request(url, headers=headers)
    try:
        with urllib.request.urlopen(req, timeout=15) as resp:
            headers_dict = dict(resp.info())
            data = json.loads(resp.read().decode("utf-8"))
            return data, headers_dict
    except urllib.error.HTTPError as e:
        print(f"Warning: GitHub API error fetching {url}: {e.code} {e.reason}", file=sys.stderr)
        return None, {}
    except Exception as e:
        print(f"Warning: Failed to fetch {url}: {e}", file=sys.stderr)
        return None, {}

def get_repo_commits(owner, repo_name, token=None):
    """Retrieve total commit count using lightweight HEAD commits check with Link header."""
    url = f"https://api.github.com/repos/{owner}/{repo_name}/commits?per_page=1"
    data, headers = fetch_json(url, token)
    if data is None:
        return 50  # reasonable fallback

    # Check Link header for rel="last" page number
    link_header = headers.get("Link") or headers.get("link")
    if link_header:
        match = re.search(r'[?&]page=(\d+)[^>]*>;\s*rel="last"', link_header)
        if match:
            return int(match.group(1))

    return len(data) if isinstance(data, list) else 1

def get_repo_languages(owner, repo_name, token=None):
    """Fetch language breakdown and format top 3 into 'Lang1:pct,Lang2:pct,Lang3:pct'."""
    url = f"https://api.github.com/repos/{owner}/{repo_name}/languages"
    data, _ = fetch_json(url, token)
    if not data or not isinstance(data, dict):
        return "C:100"

    total_bytes = sum(data.values())
    if total_bytes == 0:
        return "C:100"

    sorted_langs = sorted(data.items(), key=lambda kv: kv[1], reverse=True)[:3]
    parts = []
    for name, count in sorted_langs:
        pct = max(1.0, round((count / total_bytes) * 100.0))
        parts.append(f"{name}:{pct:.0f}")
    return ",".join(parts)

def format_time_ago(days):
    if days < 0.1:
        return "Today"
    elif days < 1.0:
        return f"{max(1, int(days * 24))}h ago"
    elif days < 2.0:
        return "Yesterday"
    else:
        return f"{int(round(days))}d ago"

def format_level_badge(stage_str):
    m = re.search(r'Level\s*(\d+)', stage_str)
    lvl_num = m.group(1) if m else "1"
    clean_stage = re.sub(r'\s*\(Level\s*\d+\)', '', stage_str).strip()
    return f"<kbd>Level {lvl_num}</kbd> {clean_stage}"

def update_readme_content(readme_path, fish_rows):
    """Update the live aquarium image tag and fish roster table in README.md."""
    if not os.path.isfile(readme_path):
        return

    with open(readme_path, "r", encoding="utf-8") as f:
        content = f.read()

    # 1. Clean header image
    img_tag = '<img src="assets/aquarium.gif" alt="Live Repository Aquarium" width="640" />'
    content = re.sub(r'<picture>.*?</picture>', img_tag, content, flags=re.DOTALL)

    # 2. Build live fish roster table with visual appearance identification and zero emojis
    table_header = (
        "<!-- FISH_TABLE_START -->\n"
        "### Aquarium Roster\n\n"
        "| Repository | Fish Colors & Appearance | Level & Stage | Finnage Style | Activity |\n"
        "| :--- | :--- | :--- | :--- | :--- |\n"
    )

    rows_md = []
    for f in fish_rows:
        appearance = get_fish_appearance(f.get("lang_raw", ""), f["stage"], f["stars"], f["is_recent"])
        level_str = format_level_badge(f["stage"])
        phenotype_badge = f"{f['fin_style']}<br><sub>{f['archetype']}</sub>"
        activity_str = "Active (<48h)" if f["is_recent"] else f"Idle ({f['last_active']})"
        if f["stars"] > 0:
            activity_str += f" · {f['stars']} star{'s' if f['stars'] > 1 else ''}"

        rows_md.append(
            f"| [**{f['name']}**]({f['url']}) "
            f"| {appearance} "
            f"| {level_str}<br><code>{f['commits']:,} commits</code> "
            f"| {phenotype_badge} "
            f"| {activity_str} |"
        )

    table_body = "\n".join(rows_md) + "\n<!-- FISH_TABLE_END -->"
    new_block = table_header + table_body

    pattern = r"<!-- FISH_TABLE_START -->.*?<!-- FISH_TABLE_END -->"
    if re.search(pattern, content, flags=re.DOTALL):
        updated_content = re.sub(pattern, new_block, content, flags=re.DOTALL)
    else:
        anchor = "</div>\n"
        if anchor in content:
            idx = content.find(anchor) + len(anchor)
            updated_content = content[:idx] + "\n" + new_block + "\n" + content[idx:]
        else:
            updated_content = content + "\n\n" + new_block

    with open(readme_path, "w", encoding="utf-8") as f:
        f.write(updated_content)

    print(f"Updated live fish roster in {readme_path}")

def export_roster_json(json_path, args, user, fish_rows):
    """Export structured fish roster and tank metadata to a clean JSON file for external websites."""
    if not json_path:
        return

    os.makedirs(os.path.dirname(os.path.abspath(json_path)), exist_ok=True)
    now_utc = datetime.now(timezone.utc).isoformat()

    fish_list = []
    for f in fish_rows:
        m = re.search(r'Level\s*(\d+)', f.get("stage", ""))
        lvl_num = int(m.group(1)) if m else 1
        clean_stage = re.sub(r'\s*\(Level\s*\d+\)', '', f.get("stage", "")).strip()

        lang_parts = [p.strip() for p in f.get("lang_raw", "").split(",") if p.strip()]
        color_details = {}

        if len(lang_parts) > 0:
            p0 = lang_parts[0].split(":")
            lang0 = p0[0].strip()
            pct0 = int(float(p0[1])) if len(p0) > 1 else 100
            color_details["primary"] = {
                "role": "body_scales",
                "language": lang0,
                "percentage": pct0,
                "color_name": get_color_name(lang0),
                "hex": get_language_hex(lang0)
            }

        if len(lang_parts) > 1:
            p1 = lang_parts[1].split(":")
            lang1 = p1[0].strip()
            pct1 = int(float(p1[1])) if len(p1) > 1 else 0
            color_details["secondary"] = {
                "role": "fins_and_veil",
                "language": lang1,
                "percentage": pct1,
                "color_name": get_color_name(lang1),
                "hex": get_language_hex(lang1)
            }

        if len(lang_parts) > 2:
            p2 = lang_parts[2].split(":")
            lang2 = p2[0].strip()
            pct2 = int(float(p2[1])) if len(p2) > 1 else 0
            color_details["accent"] = {
                "role": "streamers_and_iridescence",
                "language": lang2,
                "percentage": pct2,
                "color_name": get_color_name(lang2),
                "hex": get_language_hex(lang2)
            }

        raw_desc = get_fish_appearance(f.get("lang_raw", ""), f["stage"], f["stars"], f["is_recent"])
        clean_desc = raw_desc.replace("<br>", " ").replace("<sub>", "(").replace("</sub>", ")").replace("**", "")

        fish_list.append({
            "name": f["name"],
            "full_name": f["full_name"],
            "repo_url": f["url"],
            "level": lvl_num,
            "stage": clean_stage,
            "commits": f["commits"],
            "stars": f["stars"],
            "is_recent": bool(f["is_recent"]),
            "last_active": f["last_active"],
            "appearance": clean_desc,
            "colors": color_details,
            "phenotype": {
                "fin_style": f["fin_style"],
                "archetype": f["archetype"]
            }
        })

    data = {
        "updated_at": now_utc,
        "tank": {
            "width": args.tank_width * 2,
            "height": args.tank_height * 2,
            "mode": args.mode,
            "fps": args.fps,
            "frames": args.frames,
            "population": len(fish_list),
            "deadline_days": args.deadline_days,
            "max_capacity": args.max_fish,
            "gif_url": args.out
        },
        "fish": fish_list
    }

    with open(json_path, "w", encoding="utf-8") as out_f:
        json.dump(data, out_f, indent=2)

    print(f"Exported live fish roster JSON to {json_path}")

def find_binary(explicit_path=None):
    """Locate the git-betta binary across platforms."""
    candidates = [
        explicit_path,
        "./git-betta",
        "./git-betta.exe",
        "./dist/git-betta",
        "./dist/git-betta.exe",
        "git-betta",
        "git-betta.exe",
    ]
    for c in candidates:
        if c and os.path.isfile(c) and os.access(c, os.X_OK):
            return c
        if c and os.path.isfile(c):
            return c
    return None

def main():
    parser = argparse.ArgumentParser(description="Generate updating aquarium GIF with activity deadline filter.")
    parser.add_argument("--user", help="GitHub username/organization (defaults to GITHUB_REPOSITORY_OWNER)")
    parser.add_argument("--deadline-days", type=int, default=365,
                        help="Only include repos updated within this many days (default: 365)")
    parser.add_argument("--max-fish", type=int, default=6,
                        help="Max fish in the aquarium to prevent overcrowding (default: 6)")
    parser.add_argument("--include-forks", action="store_true", help="Include forked repositories (default: False)")
    parser.add_argument("--include-archived", action="store_true", help="Include archived repositories (default: False)")
    parser.add_argument("--current-repo", help="Always include this repo (defaults to GITHUB_REPOSITORY)")
    parser.add_argument("--mode", default="stream", choices=["stream", "cross"],
                        help="Aquarium style: 'stream' (zen calm hover, default) or 'cross' (slow leisurely cruise)")
    parser.add_argument("--tank-width", type=int, default=320, help="Tank simulation width (default: 320)")
    parser.add_argument("--tank-height", type=int, default=180, help="Tank simulation height (default: 180)")
    parser.add_argument("--out", default="assets/aquarium.gif", help="Output aquarium GIF path")
    parser.add_argument("--json", default="assets/aquarium.json", help="Path to export structured fish data JSON (or empty to disable)")
    parser.add_argument("--readme", default="README.md", help="Path to README.md to update with fish table (or empty to disable)")
    parser.add_argument("--token", help="GitHub token (defaults to GITHUB_TOKEN env var)")
    parser.add_argument("--binary", help="Path to git-betta binary")
    parser.add_argument("--frames", type=int, default=None, help="Animation frames (default: 60 for stream, 144 for cross)")
    parser.add_argument("--fps", type=int, default=10, help="Playback fps (default: 10)")
    parser.add_argument("--dry-run", action="store_true", help="Print selected repos without executing binary")

    args = parser.parse_args()

    if args.frames is None:
        args.frames = 60 if args.mode == "stream" else 144

    token = args.token or os.environ.get("GITHUB_TOKEN")
    user = args.user or os.environ.get("GITHUB_REPOSITORY_OWNER")
    current_repo_full = args.current_repo or os.environ.get("GITHUB_REPOSITORY")

    if not user and current_repo_full and "/" in current_repo_full:
        user = current_repo_full.split("/")[0]

    if not user:
        print("Error: Specify GitHub username with --user or GITHUB_REPOSITORY_OWNER env var", file=sys.stderr)
        sys.exit(1)

    print(f"============================================================")
    print(f"  git-pet Aquarium Manager for @{user}")
    print(f"  Activity Deadline: Repos active within last {args.deadline_days} days")
    print(f"  Tank Capacity Limit: Max {args.max_fish} fish")
    print(f"============================================================")

    repos_url = f"https://api.github.com/users/{user}/repos?sort=pushed&direction=desc&per_page=100"
    repos, _ = fetch_json(repos_url, token)

    if not repos or not isinstance(repos, list):
        print(f"Warning: No repositories returned from GitHub for user '{user}'.", file=sys.stderr)
        repos = []

    now = datetime.now(timezone.utc)
    qualified_repos = []
    excluded_count = 0

    # Identify current repo if specified
    current_repo_data = None
    for r in repos:
        if current_repo_full and r.get("full_name", "").lower() == current_repo_full.lower():
            current_repo_data = r
            break

    for r in repos:
        name = r.get("name")
        full_name = r.get("full_name")
        is_fork = r.get("fork", False)
        is_archived = r.get("archived", False)
        pushed_str = r.get("pushed_at")

        if not pushed_str:
            continue

        try:
            pushed_at = datetime.fromisoformat(pushed_str.replace("Z", "+00:00"))
        except Exception:
            continue

        age_days = (now - pushed_at).total_seconds() / 86400.0

        # Skip forks unless requested
        if is_fork and not args.include_forks:
            continue

        # Skip archived unless requested
        if is_archived and not args.include_archived:
            continue

        # DEADLINE CHECK: Skip old dormant repos to avoid overcrowding!
        if age_days > args.deadline_days:
            excluded_count += 1
            print(f"  [FILTERED OUT] {full_name:30s} | Inactive for {age_days:5.1f} days (> {args.deadline_days}d deadline)")
            continue

        print(f"  [QUALIFIED]    {full_name:30s} | Last active {age_days:5.1f} days ago")
        qualified_repos.append(r)

    # Ensure the current repo has a spot if it wasn't already in the qualified list
    if current_repo_data and current_repo_data not in qualified_repos:
        print(f"  [HOST REPO]    {current_repo_data.get('full_name')} included as host tank representative")
        qualified_repos.insert(0, current_repo_data)

    # If all repos exceeded deadline, keep at least 1-2 most recent so the tank isn't empty
    if not qualified_repos and repos:
        print(f"  Note: All repos exceeded {args.deadline_days}-day deadline. Keeping most recent repo to keep tank alive.")
        qualified_repos.append(repos[0])

    # Cap to max_fish to prevent overcrowding
    final_repos = qualified_repos[:args.max_fish]

    print(f"\n--- Selected Tank Population ({len(final_repos)} fish, {excluded_count} repos filtered by deadline) ---")

    seeds = []
    commits_list = []
    languages_list = []
    stars_list = []
    recent_list = []

    for r in final_repos:
        owner = r.get("owner", {}).get("login", user)
        rname = r.get("name")
        full_name = r.get("full_name", f"{owner}/{rname}")

        commits = get_repo_commits(owner, rname, token)
        langs = get_repo_languages(owner, rname, token)
        stars = r.get("stargazers_count", 0)

        pushed_str = r.get("pushed_at")
        is_recent = 0
        if pushed_str:
            try:
                pushed_dt = datetime.fromisoformat(pushed_str.replace("Z", "+00:00"))
                age_days = (now - pushed_dt).total_seconds() / 86400.0
                if age_days <= 2.0:
                    is_recent = 1
            except Exception:
                pass

        seeds.append(full_name)
        commits_list.append(str(commits))
        languages_list.append(langs)
        stars_list.append(str(stars))
        recent_list.append(str(is_recent))

        status_str = "Active <48h" if is_recent else "Idle"
        print(f"  • {full_name:28s} | {commits:5d} commits | {stars:2d} stars | {status_str} | Languages: {langs}")

    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)

    binary = find_binary(args.binary)
    if not binary and not args.dry_run:
        print(f"\nError: Could not locate git-betta binary. Build it with 'make' or gcc first.", file=sys.stderr)
        sys.exit(2)

    cmd = [
        binary or "__BINARY__",
        "--aquarium",
        "--stream" if args.mode == "stream" else "--cross",
        "--fish-count", str(len(final_repos)),
        "--seeds", ",".join(seeds),
        "--fish-commits", ",".join(commits_list),
        "--fish-languages", ";".join(languages_list),
        "--fish-stars", ",".join(stars_list),
        "--fish-recent", ",".join(recent_list),
        "--frames", str(args.frames),
        "--fps", str(args.fps),
        "--scale", "2",
        "--tank-width", str(args.tank_width),
        "--tank-height", str(args.tank_height),
        "--out", args.out
    ]

    if args.dry_run:
        print("\n[Dry Run] Base command:")
        print(" ".join(cmd))
        return

    print(f"\nRendering Midnight Abyss Aquarium GIF...")
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if res.stdout:
        print(res.stdout)

    if res.returncode != 0:
        print(f"\nError: git-betta failed rendering aquarium (code {res.returncode})", file=sys.stderr)
        sys.exit(res.returncode)

    print(f"\nSuccessfully generated aquarium GIF: {args.out}")

    # Parse stdout for fish traits and phenotypes
    fish_data_by_repo = {}
    for line in res.stdout.splitlines():
        m = re.search(r'Fish #\d+:\s+Repo\s+"([^"]+)"\s+\[(\d+)\s+commits\s+-\s+([^|]+)\|\s+(\d+)\s+stars\s+\|\s+([^\]]+)\]\s+\|\s+([^|]+)\|\s+(.*)', line)
        if m:
            r_seed, r_c, r_st, r_stars, r_recent, r_fin, r_arch = m.groups()
            fish_data_by_repo[r_seed] = {
                "stage": r_st.strip(),
                "fin_style": r_fin.strip(),
                "archetype": r_arch.strip()
            }

    fish_rows = []
    for idx, r in enumerate(final_repos):
        owner = r.get("owner", {}).get("login", user)
        rname = r.get("name")
        full_name = r.get("full_name", f"{owner}/{rname}")
        html_url = r.get("html_url", f"https://github.com/{full_name}")

        pushed_str = r.get("pushed_at")
        age_str = "Recently"
        is_recent_flag = int(recent_list[idx])
        if pushed_str:
            try:
                pushed_dt = datetime.fromisoformat(pushed_str.replace("Z", "+00:00"))
                age_days = (now - pushed_dt).total_seconds() / 86400.0
                age_str = format_time_ago(age_days)
            except Exception:
                pass

        meta = fish_data_by_repo.get(full_name, {})
        stage_name = meta.get("stage", "Active")
        fin_name = meta.get("fin_style", "Flowing Fins")
        arch_name = meta.get("archetype", "Standard")

        lang_raw = languages_list[idx]

        fish_rows.append({
            "name": rname,
            "full_name": full_name,
            "url": html_url,
            "commits": int(commits_list[idx]),
            "stars": int(stars_list[idx]),
            "is_recent": is_recent_flag,
            "stage": stage_name,
            "lang_raw": lang_raw,
            "fin_style": fin_name,
            "archetype": arch_name,
            "last_active": age_str
        })

    if args.readme:
        update_readme_content(args.readme, fish_rows)

    if args.json:
        export_roster_json(args.json, args, user, fish_rows)

if __name__ == "__main__":
    main()
