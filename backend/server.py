from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from datetime import datetime
import sqlite3
import os

app = FastAPI(title="IoT Security Log Server")

DB_FILE = "security.db"

# Database Initialization
def init_db():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    # Create logs table if it doesn't exist
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS security_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            event_type TEXT NOT NULL,
            distance_cm INTEGER NOT NULL,
            timestamp TEXT NOT NULL
        )
    ''')
    conn.commit()
    conn.close()

init_db()

# Define the data structure expected from Arduino
class AlertPayload(BaseModel):
    distance: int

@app.get("/")
def read_root():
    # Simple dashboard endpoint to view total alerts today via browser
    today_str = datetime.now().strftime("%Y-%m-%d")
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute(
        "SELECT COUNT(*) FROM security_logs WHERE event_type = 'BREACH' AND timestamp LIKE ?", 
        (f"{today_str}%",)
    )
    today_count = cursor.fetchone()[0]
    conn.close()
    
    return {
        "status": "Server Online",
        "database_file": DB_FILE,
        "total_breaches_today": today_count
    }

@app.post("/log-alert")
def log_alert(payload: AlertPayload):
    try:
        current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        # Insert breach event into SQLite database
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        cursor.execute(
            "INSERT INTO security_logs (event_type, distance_cm, timestamp) VALUES (?, ?, ?)",
            ("BREACH", payload.distance, current_time)
        )
        conn.commit()
        conn.close()
        
        print(f"[{current_time}] Alert logged successfully! Distance: {payload.distance} cm")
        return {"status": "success", "message": "Alert logged into SQLite database."}
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
