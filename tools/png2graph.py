#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
png2graph.py — полуавтоматический перенос дорожной сети с топографической PNG-карты
в промежуточный GeoJSON/CSV по методике «Road Network JSON Format.md» §11.

Рабочий процесс (pipeline):
  1. calibrate : калибровка m_per_px по двум опорным городам + проверка третьей точки;
  2. trace     : ручная трассировка дорог кликами по PNG (перекрёстки = узлы, изгибы = waypoints);
  3. export    : пиксельные координаты -> метры, GeoJSON + CSV (для генерации network.json);
  4. validate  : обмер длины полилиний (length_m = длина polyline * m_per_px, НЕ евклидова)
                 и проверка §9.3 (length_m >= euclid).

Зависимости GUI: Pillow + tkinter. В headless-среде используйте --no-gui --paths paths.json.

Примеры:
  python3 png2graph.py calibrate map.png --p1 "Южный=123,987" --p2 "Нижнегорск=1560,345" --check "Шахты=1180,720"
  python3 png2graph.py trace map.png --session map_session.json --calib calibration.json
  python3 png2graph.py export map_session.json --out-json roads.geojson --out-csv roads.csv
  python3 png2graph.py validate roads.geojson
"""

import argparse
import json
import math
import sys

# Координаты городов из Road Network Design §2 (км, мир 20x20, X запад->восток, Y юг->север)
CITIES_KM = {
    "city_yuzhny":      ("Южный",       4.5, 3.0),
    "city_elnino":      ("Ельнино",     9.5, 3.5),
    "city_klyuchi":     ("Ключи",      10.5, 7.5),
    "city_zalesk":      ("Залесск",     5.5, 8.5),
    "city_borovoe":     ("Боровое",     6.0, 13.0),
    "city_bukhta":      ("Бухта",       8.0, 16.5),
    "city_priozersk":   ("Приозёрск",  11.0, 13.5),
    "city_shakhty":     ("Шахты",      15.0, 8.0),
    "city_gorny":       ("Горный",     17.5, 10.5),
    "city_nizhnegorsk": ("Нижнегорск", 16.0, 13.5),
    "city_almazny":     ("Алмазный",   17.5, 16.0),
    "city_kolco":       ("Кольцо",     13.5, 9.5),
}


def city_km(name):
    for cid, (ru, cx, cy) in CITIES_KM.items():
        if name in (ru, cid):
            return cx, cy
    raise SystemExit(f"Неизвестный город: {name}")


def cmd_calibrate(args):
    """§11.1: два дальних опорных объекта -> m_per_px; третья точка — контроль (невязка <= 2%)."""
    def parse_pt(s):
        nm, xy = s.split("=", 1)
        x, y = (float(v) for v in xy.split(","))
        return nm.strip(), x, y

    n1, ax, ay = parse_pt(args.p1)
    n2, bx, by = parse_pt(args.p2)
    k1, k2 = city_km(n1), city_km(n2)
    dist_km = math.hypot(k2[0] - k1[0], k2[1] - k1[1])
    dist_px = math.hypot(bx - ax, by - ay)
    if dist_px < 10:
        raise SystemExit("Опорные точки слишком близко — выберите дальние города.")
    m_per_px = dist_km * 1000.0 / dist_px
    print(f"m_per_px = {m_per_px:.4f} ({dist_km:.2f} км / {dist_px:.1f} px)")

    if args.check:
        n3, cx, cy = parse_pt(args.check)
        k3 = city_km(n3)
        d_pred = math.hypot(k3[0] - k1[0], k3[1] - k1[1])
        d_meas = math.hypot(cx - ax, cy - ay) * m_per_px / 1000.0
        err = abs(d_meas - d_pred) / max(1e-9, d_pred)
        flag = "OK" if err <= 0.02 else "НЕВЯЗКА > 2% — проверьте выбор точек!"
        print(f"Контроль {n3}: предсказано {d_pred:.3f} км, измерено {d_meas:.3f} км, "
              f"невязка {err*100:.2f}% [{flag}]")

    out = {"image": args.map, "m_per_px": m_per_px,
           "anchor1": {"name": n1, "px": [ax, ay]},
           "anchor2": {"name": n2, "px": [bx, by]}}
    with open(args.out, "w", encoding="utf-8") as f:
        json.dump(out, f, ensure_ascii=False, indent=2)
    print(f"Калибровка сохранена: {args.out}")


def save_session(session, path):
    with open(path, "w", encoding="utf-8") as f:
        json.dump(session, f, ensure_ascii=False, indent=2)
    print(f"Сессия сохранена: {path} "
          f"({len(session['nodes'])} узлов, {len(session['roads'])} дорог)")


def _load_calib(path):
    try:
        with open(path, encoding="utf-8") as f:
            return json.load(f)
    except FileNotFoundError:
        return None


def cmd_trace(args):
    if args.no_gui:
        # headless: [{"points": [[x,y],...], "class": "B"}, ...]
        with open(args.paths, encoding="utf-8") as f:
            paths = json.load(f)
        session = {"calibration": _load_calib(args.calib), "nodes": [], "roads": []}
        for road in paths:
            idx = []
            for x, y in road["points"]:
                found = next((i for i, nd in enumerate(session["nodes"])
                              if abs(nd["px"][0]-x) < 6 and abs(nd["px"][1]-y) < 6), None)
                if found is None:
                    session["nodes"].append({"px": [x, y], "type": "waypoint"})
                    found = len(session["nodes"]) - 1
                idx.append(found)
            session["roads"].append({"node_indices": idx,
                                     "road_class": road.get("class", "B")})
        save_session(session, args.session)
        return

    try:
        from PIL import Image, ImageTk  # noqa
        import tkinter as tk
    except ImportError:
        sys.exit("Для GUI нужны Pillow+tkinter. Используйте --no-gui --paths file.json")

    img = Image.open(args.map)
    root = tk.Tk()
    canvas = tk.Canvas(root, width=min(img.width, 1400), height=min(img.height, 900))
    canvas.pack()
    photo = ImageTk.PhotoImage(img)
    canvas.create_image(0, 0, anchor="nw", image=photo)

    session = {"calibration": _load_calib(args.calib), "nodes": [], "roads": []}
    current = []

    def redraw():
        canvas.delete("mark")
        allpts = [nd["px"] for nd in session["nodes"]] + current
        for r in session["roads"]:
            pts = [allpts[i] for i in r["node_indices"]]
            flat = sum([list(p) for p in pts], [])
            canvas.create_line(*flat, fill="blue", width=2, tags="mark")
        if len(current) > 1:
            flat = sum([list(p) for p in current], [])
            canvas.create_line(*flat, fill="green", width=2, tags="mark")
        for nd in session["nodes"]:
            x, y = nd["px"]
            canvas.create_oval(x-4, y-4, x+4, y+4, outline="red", tags="mark")

    def find_or_add(x, y):
        for i, nd in enumerate(session["nodes"]):
            if math.hypot(nd["px"][0]-x, nd["px"][1]-y) < 6:
                return i  # перекрёсток уже есть
        session["nodes"].append({"px": [x, y], "type": "waypoint"})
        return len(session["nodes"]) - 1

    def add_node(e):
        i = find_or_add(e.x, e.y)
        current.append(session["nodes"][i]["px"])
        redraw()

    def end_road(_e):
        if len(current) >= 2:
            idx = [next(i for i, nd in enumerate(session["nodes"]) if nd["px"] == p)
                   for p in current]
            session["roads"].append({"node_indices": idx,
                                     "road_class": args.default_class})
        current.clear()
        redraw()

    def undo(_e):
        if current:
            current.pop()
        elif session["roads"]:
            session["roads"].pop()
        redraw()

    canvas.bind("<Button-1>", add_node)
    canvas.bind("<Button-3>", end_road)
    root.bind("u", undo)
    root.bind("q", lambda e: (save_session(session, args.session), root.destroy()))
    print("ЛКМ — точка, ПКМ — завершить дорогу (класс '%s'), u — отмена, q — сохранить и выйти"
          % args.default_class)
    root.mainloop()


def px_to_world(px, py, calib):
    """Пиксели -> метры мира (X восток, Y север; Y PNG инвертируется через знак sy)."""
    a1 = calib["anchor1"]["px"]; a2 = calib["anchor2"]["px"]
    k1, k2 = city_km(calib["anchor1"]["name"]), city_km(calib["anchor2"]["name"])
    dx_px, dy_px = a2[0] - a1[0], a2[1] - a1[1]
    dx_km, dy_km = k2[0] - k1[0], k2[1] - k1[1]
    sx = dx_km / dx_px if abs(dx_px) > 1 else 0.0
    sy = dy_km / dy_px if abs(dy_px) > 1 else 0.0
    return round((k1[0] + (px - a1[0]) * sx) * 1000), round((k1[1] + (py - a1[1]) * sy) * 1000)


def cmd_export(args):
    with open(args.session, encoding="utf-8") as f:
        session = json.load(f)
    calib = session.get("calibration") or _load_calib(args.calib)
    if not calib:
        sys.exit("Нет калибровки (--calib).")

    nodes_out = []
    for i, nd in enumerate(session["nodes"]):
        xm, ym = px_to_world(nd["px"][0], nd["px"][1], calib)
        nodes_out.append({"id": f"rnode_wp_{i:03d}", "x_m": xm, "y_m": ym,
                          "type": nd.get("type", "waypoint"),
                          "tile": f"x{max(0,min(19,xm//1000)):02d}_y{max(0,min(19,ym//1000)):02d}"})

    features = []
    for r, road in enumerate(session["roads"]):
        seq = road["node_indices"]
        length_m = 0.0
        for a, b in zip(seq, seq[1:]):
            na, nb = nodes_out[a], nodes_out[b]
            length_m += math.hypot(nb["x_m"] - na["x_m"], nb["y_m"] - na["y_m"])
        coords = [[nodes_out[i]["x_m"], nodes_out[i]["y_m"]] for i in seq]
        cls = road["road_class"]
        features.append({
            "type": "Feature",
            "properties": {"id": f"redge_{cls.lower()}_{r:03d}", "road_class": cls,
                           "length_m": round(length_m), "node_seq": seq},
            "geometry": {"type": "LineString", "coordinates": coords}})

    geojson = {"type": "FeatureCollection", "nodes": nodes_out, "features": features}
    with open(args.out_json, "w", encoding="utf-8") as f:
        json.dump(geojson, f, ensure_ascii=False, indent=2)
    print(f"GeoJSON: {args.out_json} ({len(nodes_out)} узлов, {len(features)} рёбер)")

    if args.out_csv:
        with open(args.out_csv, "w", encoding="utf-8") as f:
            f.write("edge_id,road_class,length_m,node_seq\n")
            for ft in features:
                p = ft["properties"]
                f.write(f"{p['id']},{p['road_class']},{p['length_m']},"
                        f"{'|'.join(map(str,p['node_seq']))}\n")
        print(f"CSV: {args.out_csv}")


def cmd_validate(args):
    """§9.3/§11.5: length_m >= euclid(from,to); предупреждение при большой кривизне."""
    with open(args.geojson, encoding="utf-8") as f:
        gj = json.load(f)
    bad = 0
    for ft in gj["features"]:
        c = ft["geometry"]["coordinates"]
        euc = math.hypot(c[-1][0]-c[0][0], c[-1][1]-c[0][1])
        lm = ft["properties"]["length_m"]
        pid = ft["properties"]["id"]
        if lm < euc:
            print(f"[ОШИБКА] {pid}: length_m={lm} < euclid={euc:.0f}")
            bad += 1
        elif euc > 0 and lm / euc > 1.6:
            print(f"[внимание] {pid}: длина в {lm/euc:.2f}x больше евклидовой (серпантин?)")
    print("OK" if bad == 0 else f"{bad} ошибок")
    sys.exit(1 if bad else 0)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    c = sub.add_parser("calibrate"); c.add_argument("map")
    c.add_argument("--p1", required=True); c.add_argument("--p2", required=True)
    c.add_argument("--check"); c.add_argument("--out", default="calibration.json")
    c.set_defaults(fn=cmd_calibrate)

    t = sub.add_parser("trace"); t.add_argument("map")
    t.add_argument("--session", default="map_session.json")
    t.add_argument("--calib", default="calibration.json")
    t.add_argument("--default-class", default="B")
    t.add_argument("--no-gui", action="store_true"); t.add_argument("--paths")
    t.set_defaults(fn=cmd_trace)

    e = sub.add_parser("export"); e.add_argument("session")
    e.add_argument("--calib", default="calibration.json")
    e.add_argument("--out-json", default="roads.geojson")
    e.add_argument("--out-csv", default="roads.csv")
    e.set_defaults(fn=cmd_export)

    v = sub.add_parser("validate"); v.add_argument("geojson")
    v.set_defaults(fn=cmd_validate)

    args = ap.parse_args()
    args.fn(args)


if __name__ == "__main__":
    main()
