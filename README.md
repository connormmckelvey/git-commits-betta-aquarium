<div align="center">
  <h1>git-pet</h1>
  <p>A living, procedural pixel-art Betta aquarium that grows with every GitHub commit.</p>
  <br />
  <img src="assets/aquarium.gif" alt="Live Repository Aquarium" width="640" />
</div>

<!-- FISH_TABLE_START -->
### Aquarium Roster

| Repository | Fish Colors & Appearance | Level & Stage | Finnage Style | Activity |
| :--- | :--- | :--- | :--- | :--- |
| [**git-commits-betta-aquarium**](https://github.com/connormmckelvey/git-commits-betta-aquarium) | **Slate Grey body, Ocean Blue fins**<br><sub>C 90% · Python 10% · Active Bubble Puff</sub> | <kbd>Level 2</kbd> Juvenile<br><code>19 commits</code> | Crowntail Spikes<br><sub>Compact Chubby</sub> | Active (<48h) |
| [**portfolio**](https://github.com/connormmckelvey/portfolio) | **Flame Red body, Canary Gold fins**<br><sub>HTML 42% · JavaScript 34%</sub> | <kbd>Level 3</kbd> Mature Adult<br><code>101 commits</code> | Veiltail Cascade<br><sub>Compact Chubby</sub> | Idle (6d ago) |
| [**Haplink**](https://github.com/connormmckelvey/Haplink) | **Ocean Blue body, Magenta Pink fins**<br><sub>Python 79% · C++ 18% · 5 Star Sparkles</sub> | <kbd>Level 2</kbd> Juvenile<br><code>40 commits</code> | Plakat Fighter Blade<br><sub>Agile Predator</sub> | Idle (30d ago) · 5 stars |
| [**surgical-cleaning-robot**](https://github.com/connormmckelvey/surgical-cleaning-robot) | **Ocean Blue body, Slate Grey fins**<br><sub>Python 39% · C 36% · Small Fry</sub> | <kbd>Level 1</kbd> Baby Fry<br><code>5 commits</code> | Doubletail Twin-Lobe<br><sub>Sleek Torpedo</sub> | Idle (99d ago) |
| [**surgical-arm-ros2**](https://github.com/connormmckelvey/surgical-arm-ros2) | **Ocean Blue body, Lime Green fins**<br><sub>Python 96% · Shell 3%</sub> | <kbd>Level 2</kbd> Juvenile<br><code>21 commits</code> | Doubletail Twin-Lobe<br><sub>Royal Crest</sub> | Idle (114d ago) |
| [**TACTUS-Medical-Robotics-FRI-Project**](https://github.com/connormmckelvey/TACTUS-Medical-Robotics-FRI-Project) | **Flame Red body, Ocean Blue fins**<br><sub>HTML 51% · Python 32%</sub> | <kbd>Level 2</kbd> Juvenile<br><code>29 commits</code> | Rosetail Petal-Fringe<br><sub>Sleek Torpedo</sub> | Idle (186d ago) |
<!-- FISH_TABLE_END -->

---

<details open>
  <summary><b>How It Works & Level Progression</b></summary>
  <br />

Each repository grows its own procedural Betta fish shaped by its commit history:

| Level | Growth Stage | Commit Milestone | Anatomical Evolution |
| :---: | :--- | :---: | :--- |
| <kbd>Level 1</kbd> | Baby Fry | 1 – 11 | Compact body, baby belly, short fin buds |
| <kbd>Level 2</kbd> | Juvenile | 12 – 59 | Spine elongation, fin ray branching, agile swimming |
| <kbd>Level 3</kbd> | Mature Adult | 60 – 249 | Billowing caudal fan, dorsal sail, flowing anal curtain |
| <kbd>Level 4</kbd> | Master Elder | 250 – 999 | Cascading waterfall finnage, peony petal ruffles |
| <kbd>Level 5</kbd> | Mythic Leviathan | 1,000+ | Titan morphology, needle ray spikes, armored scales |

#### Biological Features & Community Aura
- **Pectoral Fin Flutter**: Translucent pectoral fins flutter at 4x swimming tempo behind the gill plate for station hover.
- **Eye Blinks**: Fish naturally rest their eyes for 2 frames at periodic intervals.
- **Star Glint Sparkles**: Repositories with GitHub stars emit diamond scale glints across their flank.
- **Recent Commit Bubble Puffs**: Repositories committed to within the last 48 hours emit rising micro-bubble puffs from the mouth.
- **Color Genetics**: GitHub Linguist programming languages dictate primary scale, fin veil, and accent color ramps.
</details>

<details>
  <summary><b>Automatic GitHub Actions Updates & Anti-Crowding</b></summary>
  <br />

The tank manages itself autonomously via [`.github/workflows/aquarium.yml`](.github/workflows/aquarium.yml):
- **1-Year Cutoff (`--deadline-days 365`)**: Repositories inactive for over a year retire to prevent overcrowding.
- **Spacious Tank Limit (`--max-fish 6`)**: Caps the aquarium population at 6 fish so every fish has open swimming territory.
- **Automated Sweeps**: Evaluates deadlines weekly on Sundays via cron (`0 0 * * 0`) and updates on every code push.

```yaml
python scripts/update_aquarium.py \
  --user "${{ github.repository_owner }}" \
  --deadline-days 365 \
  --max-fish 6 \
  --mode stream \
  --out assets/aquarium.gif \
  --json assets/aquarium.json
```
</details>

<details>
  <summary><b>Display On Your Personal Website & Live JSON API</b></summary>
  <br />

#### Embed Animated Aquarium
```html
<img 
  src="https://raw.githubusercontent.com/<YOUR-USERNAME>/git-pet/main/assets/aquarium.gif" 
  alt="Live Git Aquarium" 
  width="480" 
/>
```

#### Fetch Live Fish Roster JSON
Every GitHub Actions sweep exports [`assets/aquarium.json`](assets/aquarium.json) containing structured fish metadata (colors with hex codes, levels, commit counts, fin styles, and repository links):

```javascript
// Fetch via jsDelivr CDN (CORS-friendly) or raw GitHub
const res = await fetch("https://cdn.jsdelivr.net/gh/<YOUR-USERNAME>/git-pet@main/assets/aquarium.json");
const { tank, fish } = await res.json();

console.log(fish);
// [
//   {
//     "name": "portfolio",
//     "level": 3,
//     "stage": "Mature Adult",
//     "commits": 101,
//     "appearance": "Flame Red body, Canary Gold fins",
//     "colors": { "primary": { "hex": "#E34C26" }, "secondary": { "hex": "#F1E05A" } },
//     "repo_url": "https://github.com/..."
//   }, ...
// ]
```
</details>

<details>
  <summary><b>Building & Developer Guide</b></summary>
  <br />

Built in pure C99 with zero external dependencies:

```bash
# Build engine
make
# or directly:
gcc -O2 src/*.c -o git-betta -lm

# Render a single repository pet
./git-betta --repo "connor/git-pet" --commits 50 --languages "C:85,Shell:15" --animated --out pet.gif

# Render growth time-lapse animation (1 to 1000 commits)
./git-betta --repo "connor/git-pet" --commits 1000 --languages "C:70,Python:25,Shell:5" --grow --out growth.gif

# Render multi-fish aquarium simulation
./git-betta --aquarium --stream --fish-count 6 \
  --seeds "connor/git-pet,portfolio,Haplink" \
  --fish-commits "13,101,40" \
  --fish-stars "0,0,5" \
  --fish-recent "1,0,0" \
  --out aquarium.gif
```
</details>
