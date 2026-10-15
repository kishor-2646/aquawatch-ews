# AquaWatch Early Warning System (EWS)
## Sensor Calibration & Disease Classification Guide

---

## Part 1: How Disease Outbreak Risk is Determined (Viva & Guide Defense)

When your project guide or evaluation committee asks:
> *"How can simple physical sensors (turbidity, temperature, pH) predict biological disease outbreaks like Cholera, Typhoid, or Hepatitis A without taking a microbiological culture?"*

Here is the exact scientific and architectural defense based on **WHO (World Health Organization)** and **CDC (Centers for Disease Control)** water safety protocols:

### 1. Two-Tier AI & Analytical Architecture
AquaWatch uses a two-tier approach:
* **Tier 1: Machine Learning Classification (Random Forest)**
  - Trained on historical multi-parameter water health indices.
  - Predicts macro water potability: `Safe` vs `High Risk - Outbreak Warning`.
* **Tier 2: Pathogen-Specific Environmental Vulnerability Modeling (Rule-Based Heuristic Index)**
  - Waterborne bacterial and viral pathogens cannot multiply or survive equally in all physical water conditions.
  - While an IoT sensor cannot sequence DNA, it measures the **critical environmental indicators** that allow waterborne pathogens to proliferate and bypass human immune defense.

---

### 2. Parameter-by-Parameter Biological Correlation

#### A. Turbidity (Primary Pathogen Carrier & Shield)
* **Biological Mechanism:** Turbidity measures suspended colloidal particles (silt, clay, organic matter, microbial flocs).
* **Pathogen Shielding (WHO Standard):** Pathogenic microbes (*Vibrio cholerae*, *Salmonella enterica*, *Shigella*, enteroviruses) adsorb onto suspended particles. High turbidity physically protects microbes from solar ultraviolet (UV) radiation disinfection and chlorine neutralization.
* **WHO Guideline:** Drinking water turbidity should ideally be **< 1 NTU**; values **> 5 NTU** indicate significant biological contamination risk from surface runoff or sewage infiltration.

#### B. Water Temperature (Microbial Growth Kinetics)
* **Biological Mechanism:** Microbial metabolic and reproduction rates are governed by the Arrhenius temperature relationship.
* **Bacterial Growth:** Most enteric bacteria (*Vibrio cholerae*, *Salmonella*, *E. coli*) are mesophilic, growing rapidly in warm water (20°C – 37°C). Warm ambient water in tropical areas like Assam (26°C – 32°C) accelerates pathogen colony counts exponentially.
* **Viral Persistence:** Enteric viruses like *Hepatitis A* are non-enveloped and remain infectious longer in cool or temperate water, but still require suspended matter to avoid UV degradation.

#### C. pH (Cellular Integrity & Survival Window)
* **Biological Mechanism:** Microorganisms require an optimal neutral pH range (6.5 – 8.5) to maintain transmembrane proton gradients and enzymatic activity.
* **Indicative Factor:** Rapid pH drops often signal organic acidic decomposition or acidic runoff; high pH signals alkaline contamination. Neutral to slightly alkaline water (7.0 – 8.0) is the optimal survival zone for *V. cholerae*.

---

### 3. Pathogen Profiles & Scoring Matrix

| Disease | Causative Agent | Type | Environmental Triggers | Mathematical Weighting |
| :--- | :--- | :--- | :--- | :--- |
| **Cholera** | *Vibrio cholerae* | Bacterium | Warm water (>24°C), suspended particulate matter (attaches to chitin/algae flocs), high turbidity from flood runoff. | $Risk = 100 \times TurbIndex \times (0.5 + 0.5 \times TempIndex)$ |
| **Typhoid** | *Salmonella enterica* serovar Typhi | Bacterium | Fecal contamination via runoff; persists in warm-to-ambient turbid water. | $Risk = 100 \times TurbIndex \times (0.6 + 0.4 \times TempIndex)$ |
| **Dysentery** | *Shigella dysenteriae* / *Entamoeba* | Bacterium / Amoeba | Extremely low infective dose (<10 organisms); high correlation with runoff turbidity and particulate matter. | $Risk = 100 \times TurbIndex \times (0.6 + 0.4 \times TempIndex)$ |
| **Gastroenteritis** | *E. coli* / *Campylobacter* | Bacteria | Rapid bacterial multiplication above 25°C in contaminated suspended solids. | $Risk = 100 \times TurbIndex \times (0.5 + 0.5 \times TempIndex)$ |
| **Hepatitis A** | *Hepatitis A Virus* (HAV) | Non-enveloped Virus | Highly resistant viral capsid; survives extended periods in cool/ambient turbid water where particles shield against sunlight. | $Risk = 100 \times TurbIndex \times (0.6 + 0.4 \times (1 - TempIndex))$ |

---

## Part 2: Step-by-Step Turbidity Calibration (4 Water Levels)

Because your hardware uses an optical analog sensor, the raw ADC output drops as water opacity increases (light beam gets blocked).

### Step 1: Prepare the 4 Water Samples
Prepare 4 transparent glasses with identical volume (~200 mL):
1. **Sample 1 (Clean):** Pure drinking water or RO filtered water.
2. **Sample 2 (Less Dirty / Slight Haziness):** 200 mL clean water + 2–3 drops of milk or a tiny pinch of soil.
3. **Sample 3 (Moderate Dirty):** 200 mL clean water + 1/4 teaspoon fine soil/clay, stirred.
4. **Sample 4 (More Dirty / Muddy):** 200 mL clean water + 1 full teaspoon garden soil or muddy water.

### Step 2: Record Serial Monitor ADC Values
1. Ensure the probe switch on the blue adapter board is set to **A** (Analog).
2. Clean and dry the optical prongs with a tissue before testing.
3. Dip the sensor into each glass, gently stir, wait **20 seconds** for the ADC reading to settle, and note down the `Turbidity Raw` value from Arduino Serial Monitor:

| Water Sample | Target NTU | Typical Raw ADC Range | Your Measured Raw Value |
| :--- | :--- | :--- | :--- |
| **1. Clean** | 0.0 – 2.0 NTU | ~1580 – 1640 | `_________` |
| **2. Less Dirty** | 10.0 – 15.0 NTU | ~1200 – 1400 | `_________` |
| **3. Moderate Dirty** | 30.0 – 45.0 NTU | ~600 – 900 | `_________` |
| **4. More Dirty** | 80.0 – 100.0 NTU | ~100 – 300 | `_________` |

### Step 3: Insert Your Values into `main.py`
Open `main.py` around line 146:
```python
TURBIDITY_CAL_POINTS = [
    (raw_more_dirty,     100.0),
    (raw_moderate_dirty,  35.0),
    (raw_less_dirty,      12.0),
    (raw_clean,            0.5),
]
```
The backend automatically performs smooth piecewise linear interpolation (`numpy.interp`) between these points!

---

## Part 3: Live Verification
You can test what NTU your current table yields at any time directly in your browser without restarting:
```
https://aquawatch-ews.onrender.com/api/calibration/turbidity?raw=1250
```
It returns:
```json
{
  "raw": 1250.0,
  "ntu": 25.32,
  "table": [[104.0, 100.0], [1640.0, 0.0]]
}
```
