import os
import sys
import json
import statistics
import csv
from datetime import datetime
from groq import Groq
try:
    from dotenv import load_dotenv
    load_dotenv()
except ImportError:
    pass  

API_KEY = os.getenv("OPENAI_API_KEY")
if not API_KEY:
    print("ERROR: OPENAI_API_KEY not found.")
    print("Set it with: export OPENAI_API_KEY='sk-...'")
    sys.exit(1)

client = Groq(api_key=API_KEY)


# ── Step 1: Load CSV data ──────────────────────────────────────────────────────
def load_sensor_data(filepath: str) -> list[dict]:
    """
    Reads a CSV file and returns a list of row dicts.
    Each row is one sensor reading.
    """
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


# ── Step 2: Compute statistics ─────────────────────────────────────────────────
def compute_stats(rows: list[dict]) -> dict:
    """
    Computes min, max, mean, std deviation for each numeric column.
    Returns a dict of stats per column.
    """
    print("\n[2/4] Computing statistics...")

    # Identify numeric columns
    numeric_cols = [
        key for key in rows[0].keys()
        if isinstance(rows[0][key], float)
    ]

    stats = {}
    for col in numeric_cols:
        values = [row[col] for row in rows]
        mean = sum(values) / len(values)
        std = statistics.stdev(values) if len(values) > 1 else 0
        stats[col] = {
            "min": round(min(values), 2),
            "max": round(max(values), 2),
            "mean": round(mean, 2),
            "std_dev": round(std, 2),
            "readings": len(values)
        }
        print(f"    {col}: mean={mean:.2f}, std={std:.2f}, min={min(values):.2f}, max={max(values):.2f}")

    return stats


# ── Step 3: Detect anomalies ───────────────────────────────────────────────────
def detect_anomalies(rows: list[dict], stats: dict) -> list[dict]:
    """
    Simple anomaly detection using the 2-sigma rule:
    If a reading is more than 2 standard deviations from the mean, flag it.
    
    This is the same concept used in ML outlier detection!
    """
    print("\n[3/4] Detecting anomalies (2-sigma rule)...")

    anomalies = []
    for i, row in enumerate(rows):
        for col, col_stats in stats.items():
            value = row.get(col)
            if value is None:
                continue
            mean = col_stats["mean"]
            std = col_stats["std_dev"]
            if std == 0:
                continue
            z_score = abs((value - mean) / std)  
            if z_score > 2.0:
                anomaly = {
                    "row_index": i,
                    "timestamp": row.get("timestamp", f"Row {i}"),
                    "sensor": col,
                    "value": round(value, 2),
                    "expected_range": f"{round(mean - 2*std, 2)} – {round(mean + 2*std, 2)}",
                    "z_score": round(z_score, 2),
                    "severity": "HIGH" if z_score > 3.0 else "MEDIUM"
                }
                anomalies.append(anomaly)
                print(f"    ⚠ Anomaly at {anomaly['timestamp']}: {col}={value} (z={z_score:.1f})")

    if not anomalies:
        print("    ✓ No anomalies detected.")

    return anomalies


# ── Step 4: Ask GPT to explain everything ─────────────────────────────────────
def explain_with_gpt(stats: dict, anomalies: list[dict], row_count: int) -> str:
    """
    Sends the computed stats and anomalies to GPT.
    GPT generates a plain-English report a non-technical person can understand.
    
    Key concepts used here:
    - System prompt: sets GPT's role and behavior
    - User prompt: gives GPT the actual data to analyze
    - Temperature: lower = more factual/consistent (good for reports)
    """
    print("\n[4/4] Sending data to GPT for explanation...")

    stats_text = ""
    for col, s in stats.items():
        stats_text += (
            f"- {col}: average={s['mean']}, min={s['min']}, "
            f"max={s['max']}, std_dev={s['std_dev']}\n"
        )

    anomaly_text = ""
    if anomalies:
        for a in anomalies:
            anomaly_text += (
                f"- At {a['timestamp']}: {a['sensor']} was {a['value']} "
                f"(expected range: {a['expected_range']}, severity: {a['severity']})\n"
            )
    else:
        anomaly_text = "No anomalies were detected. All readings were within normal range."

    system_prompt = """You are an expert IoT data analyst and technical writer.
Your job is to analyze sensor data statistics and anomalies, then write
a clear, plain-English report that a factory manager or non-technical
stakeholder can easily understand.

Always structure your response as valid JSON with these exact keys:
{
  "summary": "2-3 sentence overall summary of the data",
  "health_status": "NORMAL | WARNING | CRITICAL",
  "health_reason": "one sentence explaining the health status",
  "anomaly_explanation": "plain English explanation of each anomaly and what it might mean physically",
  "recommendations": ["list", "of", "3-5", "action items"],
  "insights": "any interesting patterns or trends observed in the data"
}

Be specific, practical, and avoid jargon. Focus on what the data means
for real-world equipment or environment conditions."""

    user_prompt = f"""Please analyze this IoT sensor dataset:

DATASET OVERVIEW:
- Total readings: {row_count}
- Sensors monitored: {', '.join(stats.keys())}

STATISTICS SUMMARY:
{stats_text}

ANOMALIES DETECTED:
{anomaly_text}

Generate a complete report in the JSON format specified."""

    response = client.chat.completions.create(
        model="llama-3.3-70b-versatile",          
        temperature=0.3,              
        response_format={"type": "json_object"},  
        messages=[
            {"role": "system", "content": system_prompt},
            {"role": "user",   "content": user_prompt}
        ]
    )

    raw = response.choices[0].message.content
    print("    GPT analysis complete!")
    print(f"    Tokens used: {response.usage.total_tokens}")
    return raw


# ── Step 5: Save results to JSON ───────────────────────────────────────────────
def save_results(stats: dict, anomalies: list[dict], gpt_report: str,
                 rows: list[dict], output_path: str):
    """
    Saves everything to a JSON file.
    The web dashboard (dashboard.html) reads this file to render the UI.
    """
    report = json.loads(gpt_report)

    result = {
        "generated_at": datetime.now().isoformat(),
        "total_readings": len(rows),
        "stats": stats,
        "anomalies": anomalies,
        "gpt_report": report,
        # Include raw data for charts in the dashboard
        "raw_data": rows[:50]  # limit to 50 rows for the dashboard
    }

    with open(output_path, "w") as f:
        json.dump(result, f, indent=2)

    print(f"\n✅ Results saved to: {output_path}")
    print("   Open dashboard.html in your browser to view the report.")


# ── Main entry point ───────────────────────────────────────────────────────────
def main():
    
    csv_path = sys.argv[1] if len(sys.argv) > 1 else "iot_sensor_data.csv"
    output_path = "report_output.json"

    print("=" * 55)
    print("  IoT Sensor Data Explainer — Powered by OpenAI GPT")
    print("=" * 55)

   
    rows      = load_sensor_data(csv_path)
    stats     = compute_stats(rows)
    anomalies = detect_anomalies(rows, stats)
    gpt_raw   = explain_with_gpt(stats, anomalies, len(rows))
    save_results(stats, anomalies, gpt_raw, rows, output_path)

    report = json.loads(gpt_raw)
    print("\n" + "=" * 55)
    print("  GPT REPORT")
    print("=" * 55)
    print(f"\nStatus  : {report.get('health_status')}")
    print(f"Summary : {report.get('summary')}")
    print(f"\nAnomalies: {report.get('anomaly_explanation')}")
    print(f"\nInsights: {report.get('insights')}")
    print("\nRecommendations:")
    for i, rec in enumerate(report.get("recommendations", []), 1):
        print(f"  {i}. {rec}")


if __name__ == "__main__":
    main()
