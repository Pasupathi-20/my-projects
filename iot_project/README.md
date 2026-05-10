# IoT Sensor Data Explainer
### AI-Powered Sensor Analysis using Python + OpenAI GPT

---

## What this project does

1. Reads a CSV file of IoT sensor readings (temperature, humidity, pressure, vibration)
2. Computes statistics (mean, min, max, std deviation) for each sensor channel
3. Detects anomalies using the 2-sigma rule (readings that deviate too far from normal)
4. Sends the data to OpenAI GPT to generate a plain-English report
5. Saves the report as JSON and displays it in a live web dashboard

---

## File Structure

```
iot_explainer.py       ← Main Python script (run this)
dashboard.html         ← Web dashboard (open in browser after running script)
iot_sensor_data.csv    ← Sample data (replace with your own)
report_output.json     ← Auto-generated when you run the script
requirements.txt       ← Python dependencies
```

---

## Setup

### 1. Install dependencies
```bash
pip install openai pandas python-dotenv
```

### 2. Set your OpenAI API key
```bash
# Option A: In your terminal
export OPENAI_API_KEY="sk-your-key-here"

# Option B: Create a .env file in this folder
echo 'OPENAI_API_KEY=sk-your-key-here' > .env
```
Get a key at: https://platform.openai.com/api-keys

### 3. Run the script
```bash
python iot_explainer.py
```
Or with your own CSV:
```bash
python iot_explainer.py my_factory_data.csv
```

### 4. View the dashboard
Open `dashboard.html` in your browser (Chrome/Firefox).
It reads `report_output.json` automatically.

---

## Your CSV Format

The script works with any CSV that has numeric columns.
Example structure:

```
timestamp,temperature_C,humidity_percent,pressure_hPa,vibration_g,device_id
2024-01-15 08:00:00,24.5,62.1,1013.2,0.02,SENSOR_01
...
```

- Column names can be anything
- Timestamps are optional
- String columns (like device_id) are automatically ignored

---

## Concepts used (for your resume)

| Concept | Where used |
|---|---|
| OpenAI API integration | `client.chat.completions.create()` |
| Prompt Engineering | System prompt + structured JSON output |
| Anomaly Detection | 2-sigma / z-score algorithm |
| Data Pipeline | CSV → Stats → Anomalies → GPT → JSON → Dashboard |
| REST API / JSON | `response_format={"type": "json_object"}` |
| Python file I/O | csv module, json.dump |
| Edge Computing concepts | Sensor data processing logic |

---

## What to say in interviews

> "I built an IoT sensor data pipeline in Python that reads CSV sensor data, 
> applies statistical anomaly detection using the z-score method, then sends 
> a structured summary to the OpenAI API with a carefully engineered system 
> prompt to generate a plain-English diagnostic report. The output is served 
> through a real-time web dashboard I built in HTML/JS."
