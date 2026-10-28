#!/usr/bin/env python3
"""Exercise hyprstack in a private Hyprland 0.57 session (requires kitty)."""
import argparse
import json
import os
from pathlib import Path
import shutil
import socket
import subprocess
import tempfile
import time


def wait_for(fn, description, timeout=30):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        result = fn()
        if result:
            return result
        time.sleep(0.1)
    raise AssertionError(f"Timed out: {description}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--hyprland", type=Path, required=True)
    parser.add_argument("--plugin", type=Path, required=True)
    parser.add_argument("--log", type=Path, default=Path("build/smoke-0.57.0.log"))
    parser.add_argument("--parent-display", type=Path, help="Absolute Wayland socket for a nested session")
    args = parser.parse_args()
    binary, plugin = args.hyprland.resolve(), args.plugin.resolve()
    if not shutil.which("kitty"):
        parser.error("kitty is required")
    runtime = Path(tempfile.mkdtemp(prefix="hs-"))
    env = dict(os.environ, XDG_RUNTIME_DIR=str(runtime), XDG_CONFIG_HOME=str(runtime / "config"),
               XDG_CACHE_HOME=str(runtime / "cache"), XDG_DATA_HOME=str(runtime / "data"),
               HYPRLAND_HEADLESS_ONLY="1", HYPRLAND_NO_SD_VARS="1", HYPRLAND_NO_SD_TARGET="1")
    for key in ("WAYLAND_DISPLAY", "DISPLAY", "HYPRLAND_INSTANCE_SIGNATURE", "WAYLAND_SOCKET"):
        env.pop(key, None)
    if args.parent_display:
        env["WAYLAND_DISPLAY"] = str(args.parent_display.resolve())
    config = runtime / "hyprland.lua"
    config.write_text('''hl.monitor({ output = "", mode = "1280x720@60", position = "auto", scale = 1 })
hl.config({ animations = { enabled = false }, input = { follow_mouse = 0 },
    ecosystem = { enforce_permissions = false }, debug = { enable_stdout_logs = true } })
''')
    processes = []
    args.log.parent.mkdir(parents=True, exist_ok=True)
    try:
        with args.log.open("w") as log:
            server = subprocess.Popen([str(binary), "--config", str(config)], env=env,
                                      stdout=log, stderr=subprocess.STDOUT)
            processes.append(server)
            ipc = wait_for(lambda: next((runtime / "hypr").glob("*/.socket.sock"), None), "IPC socket")

            def request(command):
                with socket.socket(socket.AF_UNIX) as connection:
                    connection.settimeout(15)
                    connection.connect(str(ipc))
                    connection.sendall(command.encode())
                    chunks = []
                    while chunk := connection.recv(65536):
                        chunks.append(chunk)
                    return b"".join(chunks).decode().strip()

            def ok(command):
                result = request(command)
                assert result == "ok", (command, result)

            def stack():
                return json.loads(request("/hyprstack stack list"))

            def lua(code):
                result = request("/eval " + code)
                assert result == "ok", (code, result)

            assert json.loads(request("j/version"))["version"] == "0.57.0"
            if not json.loads(request("j/monitors")):
                ok("/output create headless HYPRSTACK-TEST")
            ok(f"/plugin load {plugin}")
            assert len(json.loads(request("j/plugin list"))) == 1
            assert not stack()["windows"]
            assert request("/hyprstack focus next").startswith("error:")
            display = wait_for(lambda: next((p for p in runtime.glob("wayland-*") if p.is_socket()), None), "Wayland socket")
            client_env = dict(env, WAYLAND_DISPLAY=display.name)
            for i in range(3):
                processes.append(subprocess.Popen(
                    ["kitty", "--config", "NONE", "--class", f"hyprstack-test-{i}",
                     "--title", f"hyprstack-test-{i}", "sleep", "300"],
                    env=client_env, stdout=log, stderr=subprocess.STDOUT))
                wait_for(lambda: len(stack()["windows"]) == i + 1, "test window")
            initial = stack()
            order = [w["address"] for w in initial["windows"]]
            current = initial["current"]
            index = order.index(current)
            ok("/hyprstack focus next")
            assert stack()["current"] == order[(index + 1) % 3]
            ok("/hyprstack focus last")
            assert stack()["current"] == current
            ok("/hyprstack focus prev")
            assert stack()["current"] == order[(index - 1) % 3]
            ok("/hyprstack focus next")
            assert stack()["current"] == current
            ok("/hyprstack swap next")
            order[index], order[(index + 1) % 3] = order[(index + 1) % 3], order[index]
            assert [w["address"] for w in stack()["windows"]] == order
            assert stack()["current"] == current
            ok("/hyprstack swap prev")
            assert [w["address"] for w in stack()["windows"]] == [w["address"] for w in initial["windows"]]
            assert json.loads(request("/hyprstack stack around"))["current"] == current

            lua('hl.dispatch(hl.dsp.window.move({workspace = "name:notes"}))')
            assert stack()["workspace"]["id"] == "notes"
            assert len(stack()["windows"]) == 1
            lua('hl.dispatch(hl.dsp.workspace.rename({workspace = "name:notes", name = "Renamed"}))')
            assert stack()["workspace"] == {"id": "notes", "name": "Renamed"}
            lua('hl.dispatch(hl.dsp.window.move({workspace = "special:notes"}))')
            if stack()["workspace"]["id"] != "special:notes":
                lua('hl.dispatch(hl.dsp.workspace.toggle_special("notes"))')
            assert stack()["workspace"]["id"] == "special:notes"
            assert stack()["current"] == current
            ok(f"/plugin unload {plugin}")
            assert not json.loads(request("j/plugin list"))
            ok(f"/plugin load {plugin}")
            assert stack()["workspace"]["id"] == "special:notes"
            assert stack()["current"] == current
            ok(f"/plugin unload {plugin}")
            print("PASS: load, queries, focus next/prev/last, swap, named/special workspaces, rename, unload/reload")
    finally:
        for process in reversed(processes):
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
        shutil.rmtree(runtime)
        print(f"Session log: {args.log}")


if __name__ == "__main__":
    main()
