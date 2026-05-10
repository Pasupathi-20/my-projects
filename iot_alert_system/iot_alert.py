"""
Smart IoT Alert System
======================
Reads sensor CSV data, uses Groq AI to decide alert level
(CRITICAL / WARNING / NORMAL), and generates alert messages.

Requirements:
    pip install groq python-dotenv

Usage:
    python iot_alert.py                    # uses sensor_data.csv
    python iot_alert.py my_data.csv        # your own CSV
"""

import os
import sys
import json
import csv
from datetime import datetime
from groq import Groq

# ── Load API Key ───────────────────────────────────────────────────────────────
try:
    from dotenv import load_dotenv
    load_dotenv()
except ImportError:
    pass

API_KEY = os.getenv("GROQ_API_KEY")
if not API_KEY:
    print("ERROR: GROQ_API_KEY not found.")
    print("Set it with: set GROQ_API_KEY=gsk_your_key_here   (Windows)")
    print("         or: export GROQ_API_KEY=gsk_your_key_here (Mac/Linux)")
    sys.exit(1)

client = Groq(api_key=API_KEY)


# ── Step 1: Load CSV ───────────────────────────────────────────────────────────
def load_sensor_data(filepath: str) -> list[dict]:
    print(f"\n[1/4] Loading sensor data from: {filepath}")
    rows = []
    with open(filepath, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            for key, val in row.items():
                try:
                    row[key] = float(val)
                except (ValueError, TypeError):
                    pass
            rows.append(row)
    print(f"    Loaded {len(rows)} readings.")
    return rows


# ── Step 2: Get the latest reading ────────────────────────────────────────────
def get_latest_reading(rows: list[dict]) -> dict:
    """
    Gets the most recent sensor reading (last row).
    In a real IoT system this would be the live sensor value.
    """
    print("\n[2/4] Getting latest sensor reading...")
    latest = rows[-1]
    print(f"    Timestamp : {latest.get('timestamp', 'N/A')}")
    for key, val in latest.items():
        if key != "timestamp" and key != "device_id":
            print(f"    {key} : {val}")
    return latest


# ── Step 3: Ask Groq AI to decide alert level ─────────────────────────────────
def analyze_with_groq(reading: dict, all_rows: list[dict]) -> dict:
    """
    Sends the latest reading + thresholds to Groq.
    AI decides: CRITICAL / WARNING / NORMAL
    and writes an alert message.
    """
    print("\n[3/4] Sending to Groq AI for alert analysis...")

    # Calculate simple averages from history to give AI context
    numeric_keys = [k for k, v in reading.items() if isinstance(v, float)]
    averages = {}
    for key in numeric_keys:
        vals = [r[key] for r in all_rows if isinstance(r.get(key), float)]
        averages[key] = round(sum(vals) / len(vals), 2)

    # Build the prompt
    system_prompt = """You are an expert IoT monitoring system.
Your job is to analyze real-time sensor readings and decide the alert level.

Alert levels:
- CRITICAL: Immediate action required. Equipment may be damaged or unsafe.
- WARNING:  Something is abnormal. Needs attention soon.
- NORMAL:   All readings are within acceptable range.

Always respond ONLY in this exact JSON format (no extra text):
{
  "alert_level": "CRITICAL or WARNING or NORMAL",
  "alert_title": "Short title (max 8 words)",
  "alert_message": "Clear explanation of what is wrong and why it matters (2-3 sentences)",
  "affected_sensors": ["list", "of", "sensor", "names"],
  "recommended_action": "One clear action the technician should take immediately",
  "urgency_minutes": "How many minutes before action is needed (e.g. 5, 30, 60, 999 for not urgent)"
}"""

    user_prompt = f"""Analyze this IoT sensor reading:

LATEST READING (most recent):
{json.dumps({k: v for k, v in reading.items()}, indent=2)}

HISTORICAL AVERAGES (normal range context):
{json.dumps(averages, indent=2)}

Decide the alert level and generate the appropriate alert message."""

    response = client.chat.completions.create(
        model="llama-3.3-70b-versatile",
        temperature=0.2,           # Low temperature = consistent, factual decisions
        messages=[
            {"role": "system", "content": system_prompt},
            {"role": "user",   "content": user_prompt}
        ]
    )

    raw = response.choices[0].message.content
    print(f"    Groq analysis complete! (Tokens: {response.usage.total_tokens})")

    # Clean and parse JSON
    raw = raw.strip()
    if raw.startswith("```"):
        raw = raw.split("```")[1]
        if raw.startswith("json"):
            raw = raw[4:]
    return json.loads(raw.strip())


# ── Step 4: Display + Save Alert ──────────────────────────────────────────────
def display_and_save_alert(alert: dict, reading: dict):
    """
    Prints a formatted alert to the terminal
    and saves it to alerts_log.json
    """
    print("\n[4/4] Generating alert...\n")

    # Terminal colors (works on Windows 10+ and Mac/Linux)
    RESET  = "\033[0m"
    BOLD   = "\033[1m"
    RED    = "\033[91m"
    YELLOW = "\033[93m"
    GREEN  = "\033[92m"
    CYAN   = "\033[96m"
    WHITE  = "\033[97m"

    level = alert.get("alert_level", "NORMAL")

    # Pick color and symbol based on level
    if level == "CRITICAL":
        color  = RED
        symbol = "🚨"
        border = "=" * 55
    elif level == "WARNING":
        color  = YELLOW
        symbol = "⚠️ "
        border = "-" * 55
    else:
        color  = GREEN
        symbol = "✅"
        border = "-" * 55

    # Print the alert box
    print(color + border + RESET)
    print(color + BOLD + f"  {symbol}  ALERT LEVEL: {level}" + RESET)
    print(color + border + RESET)
    print(f"\n  {BOLD}Title   :{RESET} {alert.get('alert_title')}")
    print(f"  {BOLD}Time    :{RESET} {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"  {BOLD}Device  :{RESET} {reading.get('device_id', 'SENSOR_01')}")
    print(f"\n  {BOLD}Message :{RESET}")
    print(f"  {alert.get('alert_message')}")
    print(f"\n  {BOLD}Affected:{RESET} {', '.join(alert.get('affected_sensors', []))}")
    print(f"\n  {BOLD}Action  :{RESET} {CYAN}{alert.get('recommended_action')}{RESET}")
    print(f"\n  {BOLD}Urgency :{RESET} Act within {alert.get('urgency_minutes')} minutes")
    print(color + border + RESET)

    # Save to log file
    log_entry = {
        "timestamp": datetime.now().isoformat(),
        "device_id": reading.get("device_id", "SENSOR_01"),
        "latest_reading": reading,
        "alert": alert
    }

    log_path = "alerts_log.json"

    # Load existing log if it exists
    existing = []
    if os.path.exists(log_path):
        with open(log_path) as f:
            try:
                existing = json.load(f)
            except json.JSONDecodeError:
                existing = []

    existing.append(log_entry)

    with open(log_path, "w") as f:
        json.dump(existing, f, indent=2)

    print(f"\n  📁 Alert saved to: {log_path}")
    print(f"  Total alerts logged: {len(existing)}")


# ── Main ───────────────────────────────────────────────────────────────────────
def main():
    csv_path = sys.argv[1] if len(sys.argv) > 1 else "sensor_data.csv"

    print("=" * 55)
    print("   Smart IoT Alert System — Powered by Groq AI")
    print("=" * 55)

    rows    = load_sensor_data(csv_path)
    latest  = get_latest_reading(rows)
    alert   = analyze_with_groq(latest, rows)
    display_and_save_alert(alert, latest)

    print("\n✅ Done! Run again with different data to test more alerts.")


if __name__ == "__main__":
    main()
