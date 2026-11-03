# 🐟 git-pet

<div align="center">
  <img src="assets/aquarium.gif" alt="Live Repository Aquarium" width="640" />
  <p><strong>A living, procedural pixel-art Betta aquarium that grows with every GitHub commit.</strong></p>
</div>

<!-- FISH_TABLE_START -->
### 🐠 Swimming in the Aquarium

| Repository | Growth Stage & Commits | Languages & Color Ramps | Phenotype & Finnage | Last Active |
| :--- | :--- | :--- | :--- | :--- |
| [**git-commits-betta-aquarium**](https://github.com/connormmckelvey/git-commits-betta-aquarium) | 🍼 **Baby Fry (Level 1)**<br>`10` commits | **Body:** `C (91%)`<br>**Fins:** `Python (9%)`<br>**Accents:** `Makefile (1%)` | **Crowntail Spikes**<br>*Compact Chubby* | Today |
| [**portfolio**](https://github.com/connormmckelvey/portfolio) | 🐟 **Mature Adult (Level 3)**<br>`101` commits | **Body:** `HTML (42%)`<br>**Fins:** `JavaScript (34%)`<br>**Accents:** `CSS (24%)` | **Veiltail Cascade**<br>*Compact Chubby* | 5d ago |
| [**Haplink**](https://github.com/connormmckelvey/Haplink) | 🐠 **Juvenile (Level 2)**<br>`40` commits | **Body:** `Python (79%)`<br>**Fins:** `C++ (18%)`<br>**Accents:** `C (3%)` | **Plakat Fighter Blade**<br>*Agile Predator* | 29d ago |
| [**surgical-cleaning-robot**](https://github.com/connormmckelvey/surgical-cleaning-robot) | 🍼 **Baby Fry (Level 1)**<br>`5` commits | **Body:** `Python (39%)`<br>**Fins:** `C (36%)`<br>**Accents:** `C++ (14%)` | **Doubletail Twin-Lobe**<br>*Sleek Torpedo* | 98d ago |
| [**surgical-arm-ros2**](https://github.com/connormmckelvey/surgical-arm-ros2) | 🐠 **Juvenile (Level 2)**<br>`21` commits | **Body:** `Python (96%)`<br>**Fins:** `Shell (3%)`<br>**Accents:** `PowerShell (1%)` | **Doubletail Twin-Lobe**<br>*Royal Crest* | 113d ago |
| [**TACTUS-Medical-Robotics-FRI-Project**](https://github.com/connormmckelvey/TACTUS-Medical-Robotics-FRI-Project) | 🐠 **Juvenile (Level 2)**<br>`29` commits | **Body:** `HTML (51%)`<br>**Fins:** `Python (32%)`<br>**Accents:** `C++ (17%)` | **Rosetail Petal-Fringe**<br>*Sleek Torpedo* | 186d ago |
<!-- FISH_TABLE_END -->

---

## 🌊 How It Works

Every repository has its own living procedural Betta fish:
1. **Repository Name as Genetic Seed**: The repo name deterministically shapes the fish's morphology, finnage (Veiltail, Crowntail, Spadetail, Doubletail, Rosetail), and scale patterns.
2. **Commit-Driven Growth (`betta_grow`)**: As you make commits, the fish evolves through 5 biological growth stages:
   - **Baby Fry** (1–11 commits): Compact body, chubby baby belly, short bud fins, wide curious eyes.
   - **Juvenile** (12–59 commits): Body lengthening, rays and fins fanning out.
   - **Mature Adult** (60–249 commits): Sprawling flowing fins, dorsal sails, and tail lobes.
   - **Master Elder** (250–999 commits): Majestic cascading waterfalls and peony petal ruffles.
   - **Mythic Leviathan** (1000+ commits): Titan proportions, needle ray spikes, and armored scales.
3. **Multi-Color GitHub Linguist Palettes**: Colors are derived from repository programming languages:
   - **Primary Language** $\rightarrow$ Body scales & shadow contours.
   - **Secondary Language** $\rightarrow$ Flowing caudal veil, dorsal sail, and anal curtain.
   - **Tertiary Language** $\rightarrow$ Ventral streamers, iris color, and iridescent scale glints.

---

## 🔄 Automatic GitHub Actions Updates & Anti-Crowding

This repository includes [`.github/workflows/aquarium.yml`](.github/workflows/aquarium.yml) which automatically manages your tank:
1. **Activity Deadline (`--deadline-days 60`)**: Repositories without a commit in the last 60 days automatically retire from the tank so it never gets stale.
2. **Tank Capacity Limit (`--max-fish 4`)**: Caps the maximum number of fish (e.g. 3 to 5) so the tank stays spacious and tranquil with zero overcrowding.
3. **Weekly Scheduled Sweep (`cron: '0 0 * * 0'`)**: Automatically runs weekly so old projects drop out when their deadline expires even if you haven't committed.
4. **Instant Push Updates**: Whenever you push code, the corresponding fish grows in real time.

You can customize the deadline in `.github/workflows/aquarium.yml`:
```yaml
python scripts/update_aquarium.py \
  --user "${{ github.repository_owner }}" \
  --deadline-days 60 \
  --max-fish 4 \
  --out assets/aquarium.gif
```

### Displaying On Your Personal Website

Add this snippet to your personal website, blog, or portfolio:

```html
<img 
  src="https://raw.githubusercontent.com/<YOUR-USERNAME>/git-pet/main/assets/aquarium.gif" 
  alt="Live Git Aquarium" 
  width="480" 
/>
```

---

## 💻 Local Building & CLI Usage

### Build
Requires any C99 compiler (`gcc`, `clang`, or `MSVC`):

```bash
make
# or directly:
gcc -O2 src/*.c -o git-betta -lm
```

### Run
```bash
# Render a single repository pet
./git-betta --repo "connor/git-pet" --commits 50 --languages "C:85,Shell:15" --animated --out pet.gif

# Render a growth time-lapse animation (1 to 1000 commits)
./git-betta --repo "connor/git-pet" --commits 1000 --languages "C:70,Python:25,Shell:5" --grow --out growth.gif

# Render a multi-fish aquarium simulation
./git-betta --aquarium --cross --fish-count 3 \
  --seeds "connor/git-pet,sapphire-drift,flame-crest" \
  --fish-commits "10,80,450" \
  --fish-languages "C:85,Shell:15;Sapphire:100;Flame:100" \
  --out aquarium.gif
```
