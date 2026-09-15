# -*- coding: utf-8 -*-
"""
delete_obsolete.py - 删除 GitHub 仓库中已废弃的文件
（红外/433/ESP-NOW 相关模块，本地已删除，远端同步清理）
"""
import json
import subprocess
import sys
import urllib.request
import urllib.parse

REPO = "Grhhh-like/XiaoZhi-SmartHome"

OBSOLETE = [
    "firmware/terminal/main/ir_remote.c",
    "firmware/terminal/main/ir_remote.h",
    "firmware/terminal/main/rf433.c",
    "firmware/terminal/main/rf433.h",
    "firmware/terminal/main/espnow_mesh.c",
    "firmware/terminal/main/espnow_mesh.h",
    "firmware/node/espnow_node/espnow_node.c",
    "firmware/node/espnow_node/espnow_node.h",
    "firmware/node/espnow_node/CMakeLists.txt",
    "gateway/ir_codes/gree_ac.json",
    "gateway/ir_codes/README.md",
]


def get_token():
    r = subprocess.run(["gh", "auth", "token"], capture_output=True, text=True)
    return r.stdout.strip()


def delete_file(token, path):
    url = f"https://api.github.com/repos/{REPO}/contents/{urllib.parse.quote(path)}"
    # 先取 sha
    req = urllib.request.Request(url, headers={
        "Authorization": f"token {token}",
        "Accept": "application/vnd.github+json",
        "User-Agent": "XiaoZhi-Uploader",
    })
    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            data = json.loads(resp.read().decode())
        sha = data.get("sha")
    except Exception as e:
        print(f"[SKIP] {path}: 不存在或读取失败 {str(e)[:120]}")
        return True

    body = {"message": f"remove obsolete {path}", "sha": sha}
    req = urllib.request.Request(url, method="DELETE",
                                 data=json.dumps(body).encode(),
                                 headers={
                                     "Authorization": f"token {token}",
                                     "Accept": "application/vnd.github+json",
                                     "User-Agent": "XiaoZhi-Uploader",
                                 })
    try:
        with urllib.request.urlopen(req, timeout=60) as resp:
            resp.read()
        print(f"[DEL] {path}")
        return True
    except Exception as e:
        print(f"[FAIL] {path}: {str(e)[:200]}")
        return False


def main():
    token = get_token()
    if not token:
        print("no gh token")
        return 1
    ok = 0
    for p in OBSOLETE:
        if delete_file(token, p):
            ok += 1
    print(f"Done. deleted/ok={ok}/{len(OBSOLETE)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
