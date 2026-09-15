# -*- coding: utf-8 -*-
"""
upload_to_github.py - 通过 GitHub Contents API 上传「小智·智家」仓库
- 使用 gh auth token 认证
- 处理已存在文件的 sha 更新
- urllib 直调，避免 git push 直连不通的问题
"""
import base64
import json
import os
import subprocess
import sys
import urllib.request
import urllib.parse

REPO = "Grhhh-like/XiaoZhi-SmartHome"
BASE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
SKIP_DIRS = {".git", ".vscode", "__pycache__", "build", "managed_components"}


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
    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            data = json.loads(resp.read().decode())
            return data.get("sha")
    except Exception:
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
    try:
        with urllib.request.urlopen(req, timeout=60) as resp:
            resp.read()
            return True, ""
    except Exception as e:
        return False, str(e)[:300]


def collect_files(root):
    files = []
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
        for fn in filenames:
            full = os.path.join(dirpath, fn)
            rel = os.path.relpath(full, root).replace("\\", "/")
            files.append((full, rel))
    return sorted(files, key=lambda x: x[1])


def main():
    token = get_token()
    if not token:
        print("no gh token")
        return 1
    print(f"token acquired ({token[:8]}...)")

    files = collect_files(BASE)
    print(f"total files: {len(files)}")

    ok, fail = 0, []
    for full, rel in files:
        with open(full, "rb") as f:
            b64 = base64.b64encode(f.read()).decode()
        success, err = upload(token, rel, b64)
        if success:
            ok += 1
            print(f"[OK] {rel}")
        else:
            fail.append((rel, err))
            print(f"[FAIL] {rel}: {err}")

    print(f"\nDone. success={ok}, fail={len(fail)}")
    for rel, err in fail:
        print(f"  FAILED: {rel} -> {err}")
    return 1 if fail else 0


if __name__ == "__main__":
    sys.exit(main())
