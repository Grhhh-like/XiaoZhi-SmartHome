# -*- coding: utf-8 -*-
"""
gateway.py - 小智·智家 控制中枢网关
小智·智家 XiaoZhi SmartHome

功能：
- MQTT 桥接（EMQX/Mosquitto）
- 场景引擎（YAML 配置）
- 设备状态聚合
- REST API（Flask）
"""
import argparse
import json
import logging
import time
import threading

import yaml
import paho.mqtt.client as mqtt
from flask import Flask, request, jsonify

log = logging.getLogger("gateway")

# ---------------- 状态存储 ----------------
device_states = {}          # name -> {"state":..., "value":..., "ts":...}
scenes = {}                 # name -> {actions:[...], confirm:str}

# ---------------- MQTT ----------------
class MqttBridge:
    def __init__(self, broker, port=1883, user=None, password=None):
        self.client = mqtt.Client()
        if user:
            self.client.username_pw_set(user, password)
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message
        self.broker = broker
        self.port = port

    def _on_connect(self, client, userdata, flags, rc):
        log.info("MQTT connected: %s", self.broker)
        # 订阅设备状态主题
        client.subscribe("smart_home/+/status")

    def _on_message(self, client, userdata, msg):
        topic = msg.topic
        try:
            payload = json.loads(msg.payload.decode())
        except Exception:
            log.warning("bad mqtt payload: %s", msg.payload)
            return
        # topic: smart_home/{device}/status
        parts = topic.split("/")
        if len(parts) >= 3:
            device = parts[1]
            device_states[device] = {
                "state": payload.get("state", "unknown"),
                "value": payload.get("value", 0),
                "ts": payload.get("ts", time.time()),
            }
            log.info("status update: %s -> %s", device, payload)

    def start(self):
        self.client.connect(self.broker, self.port, 60)
        self.client.loop_start()

    def publish_cmd(self, device, cmd, value=0):
        topic = f"smart_home/{device}/cmd"
        payload = {
            "cmd": cmd,
            "value": value,
            "request_id": f"gw{int(time.time()*1000)}",
            "ts": time.time(),
        }
        self.client.publish(topic, json.dumps(payload), qos=1)
        log.info("publish: %s -> %s", topic, payload)

# ---------------- 场景引擎 ----------------
def execute_scene(mqtt: MqttBridge, name: str):
    if name not in scenes:
        return {"ok": False, "msg": f"scene not found: {name}"}
    scene = scenes[name]
    for action in scene.get("actions", []):
        mqtt.publish_cmd(
            action["device"],
            action["cmd"],
            action.get("value", 0),
        )
    return {"ok": True, "msg": scene.get("confirm", f"{name} executed")}

# ---------------- REST API ----------------
def create_app(mqtt: MqttBridge):
    app = Flask(__name__)

    @app.get("/api/status")
    def api_status():
        return jsonify({"devices": device_states, "scenes": list(scenes.keys())})

    @app.post("/api/device/<name>/cmd")
    def api_device_cmd(name):
        body = request.get_json(force=True)
        mqtt.publish_cmd(name, body.get("cmd", "on"), body.get("value", 0))
        return jsonify({"ok": True, "device": name, "cmd": body.get("cmd")})

    @app.post("/api/scene/<name>")
    def api_scene(name):
        return jsonify(execute_scene(mqtt, name))

    @app.get("/api/ir/codes")
    def api_ir_codes():
        # 红外码库由 ir_codes/ 目录加载
        return jsonify({"codes": ir_code_list()})

    return app

def ir_code_list():
    import os, glob
    base = os.path.join(os.path.dirname(__file__), "ir_codes")
    files = [os.path.basename(f) for f in glob.glob(os.path.join(base, "*.json"))]
    return files

def load_scenes(path):
    with open(path, "r", encoding="utf-8") as f:
        data = yaml.safe_load(f)
    return data.get("scenes", {})

def main():
    logging.basicConfig(level=logging.INFO,
                        format="%(asctime)s [%(name)s] %(levelname)s %(message)s")

    parser = argparse.ArgumentParser(description="XiaoZhi SmartHome Gateway")
    parser.add_argument("--mqtt-broker", default="127.0.0.1")
    parser.add_argument("--mqtt-port", type=int, default=1883)
    parser.add_argument("--mqtt-user", default=None)
    parser.add_argument("--mqtt-password", default=None)
    parser.add_argument("--config", default="scenes.yaml")
    parser.add_argument("--http-port", type=int, default=5000)
    args = parser.parse_args()

    global scenes
    scenes = load_scenes(args.config)
    log.info("loaded %d scenes", len(scenes))

    bridge = MqttBridge(args.mqtt_broker, args.mqtt_port,
                        args.mqtt_user, args.mqtt_password)
    bridge.start()

    app = create_app(bridge)
    app.run(host="0.0.0.0", port=args.http_port)

if __name__ == "__main__":
    main()
