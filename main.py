from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import subprocess
import sys
import argparse
import time
import threading

class Request(BaseModel):
    party_id: int

app = FastAPI()


@app.post("/test/add")
def add(request: Request):
    
    cmd = ["./bin/fssmain", str(request.party_id), "test", "-n", "ss", "-m", "7"]
    
    try:
        # Execute the command and capture output
        result = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        output = result.decode("utf-8").strip()

        return {"party_id": request.party_id, "output": output}

    except subprocess.CalledProcessError as e:
        # Handle errors from the subprocess
        error_output = e.output.decode("utf-8").strip()
        raise HTTPException(status_code=500, detail=f"Command failed: {error_output}")
    except Exception as e:
        # Handle general errors
        raise HTTPException(status_code=500, detail=str(e))
