# -*- coding: utf-8 -*-
"""
retry_upload.py - 重试上传失败的 18 个文件（限流恢复后）
"""
import base64
import json
import os
import subprocess
import sys
import time
import urllib.request
import urllib.parse

REPO = "Grhhh-like/XiaoZhi-SmartHome"
BASE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")

RETRY = [
    "LICENSE",
    "README.md",
    "app/README.md",
    "docs/design.md",
    "docs/scenarios.md",
    "docs/thesis_outline.md",
    "firmware/node/README.md",
    "firmware/terminal/main/lcd_ui.h",
    "firmware/terminal/main/main.c",
    "gateway/gateway.py",
    "gateway/requirements.txt",
    "gateway/scenes.yaml",
    "hardware/README.md",
    "hardware/terminal_LCEDA.json",
    "nas_bridge/README.md",
    "nas_bridge/bridge.py",
    "scripts/gen_arch_svg.py",
    "scripts/gen_lceda_json.py",
]


def get_token():
    r = subprocess.run(["gh", "auth", "token"], capture_output=True, text=True)
    return r.stdout.strip()


def get_sha(token, path):
    url = f"https://api.github.com/repos/{REPO}/contents/{urllib.parse.quote(path)}"
    req = urllib.request.Request(url, headers={
        "Authorization": f"token {token}",
        "Accept": "application/vnd.github+json",
        "User-Agent": "XiaoZhi-Uploader",
    })
    for attempt in range(3):
        try:
            with urllib.request.urlopen(req, timeout=30) as resp:
                data = json.loads(resp.read().decode())
                return data.get("sha")
        except Exception as e:
            code = getattr(e, "code", None)
            if code in (403, 429, 502, 503):
                time.sleep(5 * (attempt + 1))
                continue
            return None
    return None


def upload(token, rel, b64):
    sha = get_sha(token, rel)
    body = {"message": f"add/update {rel}", "content": b64}
    if sha:
        body["sha"] = sha
    url = f"https://api.github.com/repos/{REPO}/contents/{urllib.parse.quote(rel)}"
    req = urllib.request.Request(url, method="PUT",
                                 data=json.dumps(body).encode(),
                                 headers={
                                     "Authorization": f"token {token}",
                                     "Accept": "application/vnd.github+json",
                                     "User-Agent": "XiaoZhi-Uploader",
                                 })
    for attempt in range(3):
        try:
            with urllib.request.urlopen(req, timeout=60) as resp:
                resp.read()
                return True, ""
        except Exception as e:
            code = getattr(e, "code", None)
            if code in (403, 429, 502, 503, 409):
                time.sleep(6 * (attempt + 1))
                continue
            return False, str(e)[:200]
    return False, "max retries"


def main():
    token = get_token()
    if not token:
        print("no gh token")
        return 1
    ok, fail = 0, []
    for rel in RETRY:
        full = os.path.join(BASE, rel.replace("/", os.sep))
        with open(full, "rb") as f:
            b64 = base64.b64encode(f.read()).decode()
        success, err = upload(token, rel, b64)
        if success:
            ok += 1
            print(f"[OK] {rel}")
        else:
            fail.append((rel, err))
            print(f"[FAIL] {rel}: {err}")
        time.sleep(1.5)
    print(f"\nDone. success={ok}, fail={len(fail)}")
    for rel, err in fail:
        print(f"  FAILED: {rel} -> {err}")
    return 1 if fail else 0


if __name__ == "__main__":
    sys.exit(main())
