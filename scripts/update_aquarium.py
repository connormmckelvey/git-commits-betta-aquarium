#!/usr/bin/env python3
"""
git-pet Aquarium Orchestrator
Fetches active repositories from GitHub, filters out inactive/old repositories
based on a configurable deadline (in days), and generates a multi-fish aquarium
sized so the tank is never overcrowded.
Supports dual dark/light mode rendering, star glitter, and recent activity auras.
"""

import os
import sys
import json
import re
import shutil
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

def get_stage_icon(stage_str):
    s = stage_str.lower()
    if "fry" in s or "baby" in s:
        return "🍼"
    if "juvenile" in s:
        return "🐠"
    if "adult" in s:
        return "🐟"
    if "elder" in s:
        return "👑"
    if "leviathan" in s:
        return "🐉"
    return "🐟"

def parse_lang_slots(lang_str):
    """Split comma string like 'C:94,Python:6,Makefile:1' into body, fins, and accent descriptions."""
    parts = [p.strip() for p in lang_str.split(",") if p.strip()]
    def fmt(p):
        if ":" in p:
            k, v = p.split(":", 1)
            return f"{k} ({v}%)"
        return p
    body = fmt(parts[0]) if len(parts) > 0 else "Primary Shade"
    fins = fmt(parts[1]) if len(parts) > 1 else "Tonal Shade"
    accent = fmt(parts[2]) if len(parts) > 2 else "Iridescent Shimmer"
    return body, fins, accent

def update_readme_content(readme_path, fish_rows):
    """Update the live aquarium <picture> tag and fish roster table in README.md."""
    if not os.path.isfile(readme_path):
        return

    with open(readme_path, "r", encoding="utf-8") as f:
        content = f.read()

    # 1. Ensure GitHub Light/Dark Mode <picture> tag is active
    picture_block = (
        '<picture>\n'
        '    <source media="(prefers-color-scheme: dark)" srcset="assets/aquarium_dark.gif">\n'
        '    <source media="(prefers-color-scheme: light)" srcset="assets/aquarium_light.gif">\n'
        '    <img src="assets/aquarium.gif" alt="Live Repository Aquarium" width="640" />\n'
        '  </picture>'
    )

    # Replace legacy <img src="assets/aquarium.gif" ... /> if not already using <picture>
    if "<picture>" not in content:
        content = re.sub(r'<img\s+src="assets/aquarium\.gif"[^>]*>', picture_block, content)

    # 2. Build live fish roster table
    table_header = (
        "<!-- FISH_TABLE_START -->\n"
        "### 🐠 Swimming in the Aquarium\n\n"
        "| Repository | Growth Stage & Commits | Languages & Color Ramps | Phenotype & Finnage | Stars & Activity |\n"
        "| :--- | :--- | :--- | :--- | :--- |\n"
    )

    rows_md = []
    for f in fish_rows:
        activity_badge = "🫧 **Active (<48h)**" if f["is_recent"] else f"💤 *{f['last_active']}*"
        stars_badge = f"⭐ {f['stars']} star{'s' if f['stars'] != 1 else ''}"

        rows_md.append(
            f"| [**{f['name']}**]({f['url']}) "
            f"| {f['icon']} **{f['stage']}**<br>`{f['commits']:,}` commits "
            f"| **Body:** `{f['body_lang']}`<br>**Fins:** `{f['fin_lang']}`<br>**Accents:** `{f['accent_lang']}` "
            f"| **{f['fin_style']}**<br>*{f['archetype']}* "
            f"| {stars_badge}<br>{activity_badge} |"
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

    print(f"Updated live fish roster and dual-mode <picture> in {readme_path}")

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
    parser.add_argument("--out", default="assets/aquarium.gif", help="Output fallback aquarium GIF path")
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
    print(f"  🐟 git-pet Aquarium Manager for @{user}")
    print(f"  Activity Deadline: Repos active within last {args.deadline_days} days")
    print(f"  Tank Capacity Limit: Max {args.max_fish} fish (Prevents overcrowding)")
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

    assets_dir = os.path.dirname(os.path.abspath(args.out))
    os.makedirs(assets_dir, exist_ok=True)

    out_dark = os.path.join(assets_dir, "aquarium_dark.gif")
    out_light = os.path.join(assets_dir, "aquarium_light.gif")
    out_fallback = os.path.abspath(args.out)

    binary = find_binary(args.binary)
    if not binary and not args.dry_run:
        print(f"\nError: Could not locate git-betta binary. Build it with 'make' or gcc first.", file=sys.stderr)
        sys.exit(2)

    base_cmd = [
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
        "--tank-height", str(args.tank_height)
    ]

    if args.dry_run:
        print("\n[Dry Run] Base command:")
        print(" ".join(base_cmd))
        return

    # 1. Render Dark Mode GIF
    print(f"\nRendering Midnight Abyss (Dark Mode) Aquarium GIF...")
    cmd_dark = list(base_cmd) + ["--lighting", "dark", "--out", out_dark]
    res_dark = subprocess.run(cmd_dark, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if res_dark.stdout:
        print(res_dark.stdout)

    if res_dark.returncode != 0:
        print(f"\nError: git-betta failed rendering dark mode (code {res_dark.returncode})", file=sys.stderr)
        sys.exit(res_dark.returncode)

    # 2. Render Light Mode GIF
    print(f"\nRendering Sunlit Lagoon (Light Mode) Aquarium GIF...")
    cmd_light = list(base_cmd) + ["--lighting", "light", "--out", out_light]
    res_light = subprocess.run(cmd_light, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if res_light.stdout:
        print(res_light.stdout)

    if res_light.returncode != 0:
        print(f"\nError: git-betta failed rendering light mode (code {res_light.returncode})", file=sys.stderr)
        sys.exit(res_light.returncode)

    # 3. Create Fallback GIF
    shutil.copyfile(out_dark, out_fallback)
    print(f"\nSuccessfully generated dual-theme aquarium:")
    print(f"  • Dark Mode:  {out_dark}")
    print(f"  • Light Mode: {out_light}")
    print(f"  • Fallback:   {out_fallback}")

    # Parse stdout for fish traits and phenotypes
    fish_data_by_repo = {}
    for line in res_dark.stdout.splitlines():
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
        b_lang, f_lang, a_lang = parse_lang_slots(lang_raw)

        fish_rows.append({
            "name": rname,
            "full_name": full_name,
            "url": html_url,
            "commits": int(commits_list[idx]),
            "stars": int(stars_list[idx]),
            "is_recent": is_recent_flag,
            "stage": stage_name,
            "icon": get_stage_icon(stage_name),
            "body_lang": b_lang,
            "fin_lang": f_lang,
            "accent_lang": a_lang,
            "fin_style": fin_name,
            "archetype": arch_name,
            "last_active": age_str
        })

    if args.readme:
        update_readme_content(args.readme, fish_rows)

if __name__ == "__main__":
    main()
