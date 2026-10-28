"""Embedded only in the HH3 comparison build, never imported on device."""
HH3_PHASES = ("Index", "Datei/Header", "Position", "Puffer", "Lesen",
              "Pruefsumme", "Zeichnen", "Anzeige", "GC", "RAM-Diagnose")
HH3_ACTIVE = False
HH3_TIMES = None
HH3_CURRENT = 7
HH3_LAST = 0
HH3_COUNTS = None
HH3_SAVED = {}


def hh3_clock():
    if hasattr(time, "ticks_ms"):
        return time.ticks_ms()
    return int(time.monotonic() * 1000)


def hh3_diff(now, before):
    return time.ticks_diff(now, before) if hasattr(time, "ticks_diff") else now - before


def hh3_switch(category):
    global HH3_CURRENT, HH3_LAST
    previous = HH3_CURRENT
    if HH3_ACTIVE:
        now = hh3_clock()
        HH3_TIMES[HH3_CURRENT] += hh3_diff(now, HH3_LAST)
        HH3_LAST = now
    HH3_CURRENT = category
    return previous


class HH3Index:
    def __init__(self, table):
        self.table = table

    def __getitem__(self, key):
        previous = hh3_switch(0)
        try:
            return self.table[key]
        finally:
            hh3_switch(previous)


class HH3GC:
    def __init__(self, module):
        self.module = module

    def collect(self):
        previous = hh3_switch(8)
        if HH3_ACTIVE:
            HH3_COUNTS[0] += 1
        try:
            return self.module.collect()
        finally:
            hh3_switch(previous)

    def mem_free(self):
        previous = hh3_switch(9)
        if HH3_ACTIVE:
            HH3_COUNTS[1] += 1
        try:
            return self.module.mem_free()
        finally:
            hh3_switch(previous)


def hh3_stage_call(label, function):
    category = {"Bildansicht vorbereiten": 7, "Bildverzeichnis": 0,
                "Datei oeffnen": 1, "Paketkennung pruefen": 1,
                "Bildposition suchen": 2, "Bildpuffer anlegen": 3,
                "Bilddaten lesen": 4, "Pruefsumme pruefen": 5,
                "Vektorskizze zeichnen": 6, "Rasterdaten pruefen": 6,
                "Palette vorbereiten": 6, "Bildobjekt erzeugen": 6,
                "Rasterbild zeichnen": 6, "Bildbeschriftung": 7,
                "Bildschirm aktualisieren": 7}.get(label, 7)
    hh3_switch(category)
    function(label)


def hh3_stage(label):
    hh3_stage_call(label, HH3_SAVED["media_stage"])


def hh3_legacy_stage(label):
    hh3_stage_call(label, HH3_SAVED["legacy_media_stage"])


def hh3_open(kind):
    previous = hh3_switch(1)
    try:
        return HH3_SAVED["open_store"](kind)
    finally:
        hh3_switch(previous)


def hh3_legacy_open(kind):
    previous = hh3_switch(1)
    try:
        return HH3_SAVED["legacy_open_store"](kind)
    finally:
        hh3_switch(previous)


def hh3_bind(enable):
    names = ("MEDIA_INDEX", "legacy_MEDIA_INDEX", "gc", "media_stage",
             "legacy_media_stage", "open_store", "legacy_open_store")
    if enable:
        for name in names:
            HH3_SAVED[name] = globals()[name]
        globals()["MEDIA_INDEX"] = HH3Index(HH3_SAVED["MEDIA_INDEX"])
        globals()["legacy_MEDIA_INDEX"] = HH3Index(HH3_SAVED["legacy_MEDIA_INDEX"])
        globals()["gc"] = HH3GC(HH3_SAVED["gc"])
        globals()["media_stage"] = hh3_stage
        globals()["legacy_media_stage"] = hh3_legacy_stage
        globals()["open_store"] = hh3_open
        globals()["legacy_open_store"] = hh3_legacy_open
    else:
        for name in names:
            if name in HH3_SAVED:
                globals()[name] = HH3_SAVED[name]
        HH3_SAVED.clear()


def hh3_trial(legacy, ident):
    global HH3_ACTIVE, HH3_TIMES, HH3_CURRENT, HH3_LAST, HH3_COUNTS
    table = HH3_SAVED["legacy_MEDIA_INDEX" if legacy else "MEDIA_INDEX"]
    table.last_key = None
    table.last_value = None
    HH3_TIMES = [0] * len(HH3_PHASES)
    HH3_COUNTS = [0, 0]
    HH3_CURRENT = 7
    started = hh3_clock()
    HH3_LAST = started
    HH3_ACTIVE = True
    try:
        function = legacy_render_media_page if legacy else render_media_page
        function(ident, 0, 1)
        hh3_switch(7)
        return (hh3_diff(hh3_clock(), started), tuple(HH3_TIMES), tuple(HH3_COUNTS))
    finally:
        HH3_ACTIVE = False


def hh3_cancelled():
    for event in poll_events():
        if event.type == KEYEV_DOWN and event.key in (KEY_DEL, KEY_EXIT):
            return True
        if event.type == KEYEV_TOUCH_UP and event.y < HEADER_H and event.x < 70:
            return True
    return False


def hh3_ids():
    last = "rings"
    offset = -1
    p4 = None
    for ident, row in MEDIA_INDEX.items():
        if row[2] > offset:
            last, offset = ident, row[2]
        if row[1] == "P4" and p4 is None:
            p4 = ident
    return ("electron_tube", "rings", last, p4 or last)


def hh3_median(values):
    return sorted(values)[len(values) // 2]


def hh3_compare():
    global HH3_ACTIVE
    ids = hh3_ids()[:3]
    results = []
    before = memory_free()
    hh3_bind(True)
    try:
        for ident in ids:
            trials = [[], []]
            for repeat in range(3):
                # Alternate order to reduce bias from file-system warm-up.
                for mode in ((0, 1) if repeat % 2 == 0 else (1, 0)):
                    if hh3_cancelled():
                        show_text("HH3-Bildtest", "Abgebrochen. Keine Dateien veraendert.")
                        return
                    dclear(C_WHITE)
                    draw_header("HH3-Bildtest", True)
                    for line, text in enumerate((ident, "Alt" if mode == 0 else "HH3 optimiert",
                                                 "Durchlauf " + str(repeat + 1) + " / 3",
                                                 "DEL: nach diesem Bild abbrechen")):
                        dtext_opt(8, 60 + line * 22, FG, C_NONE, DTEXT_LEFT, DTEXT_TOP, text, -1)
                    dupdate()
                    trials[mode].append(hh3_trial(mode == 0, ident))
            results.append((ident, trials))
    except (OSError, ValueError, TypeError, AttributeError, MemoryError, KeyError, IndexError, OverflowError) as error:
        if mode == 0:
            globals()["MEDIA_STAGE"] = legacy_MEDIA_STAGE
        show_text("HH3-Testfehler", media_error(error, ident))
        return
    finally:
        HH3_ACTIVE = False
        hh3_bind(False)
        gc.collect()
    after = memory_free()
    for ident, trials in results:
        old, new = trials
        body = "Millisekunden: Alt / HH3\nErster Aufruf: " + str(old[0][0]) + " / " + str(new[0][0])
        body += "\nMedian gesamt: " + str(hh3_median([r[0] for r in old]))
        body += " / " + str(hh3_median([r[0] for r in new]))
        body += "\nGC-Aufrufe: " + str(old[0][2][0]) + " / " + str(new[0][2][0])
        body += "\nRAM-Abfragen: " + str(old[0][2][1]) + " / " + str(new[0][2][1])
        for i, label in enumerate(HH3_PHASES):
            body += "\n" + label + ": " + str(hh3_median([r[1][i] for r in old]))
            body += " / " + str(hh3_median([r[1][i] for r in new]))
        body += "\nRAM vorher/nachher: " + str(before) + " / " + str(after)
        body += "\n-1 = PC, kein Geraete-RAM.\nDiagnostischer Vergleich im selben Testbuild."
        show_text(ident, body)


def hh3_swaps():
    ids = hh3_ids()
    before = memory_free()
    try:
        for i in range(20):
            if hh3_cancelled():
                return
            render_media_page(ids[i % len(ids)], i, 20)
        gc.collect()
        body = "20 Bildwechsel abgeschlossen.\nRAM vorher: " + str(before)
        body += "\nRAM danach: " + str(memory_free())
        body += "\nFarben und vollstaendige Bilder visuell pruefen.\n-1 = PC, kein Geraete-RAM."
        show_text("HH3-Bildwechsel", body)
    except (OSError, ValueError, TypeError, AttributeError, MemoryError, KeyError, IndexError, OverflowError) as error:
        show_text("HH3-Testfehler", media_error(error, ids[i % len(ids)]))


def hh3_test():
    while True:
        selected = select_list("HH3-Bildtest", (("Ladewege vergleichen", 0),
                               ("20 Bildwechsel", 1), ("Zurueck", 2)), True)
        if selected is None or selected == 2:
            return
        try:
            if selected == 0:
                hh3_compare()
            else:
                hh3_swaps()
        except (OSError, ValueError, TypeError, AttributeError, MemoryError, KeyError, IndexError, OverflowError) as error:
            show_text("HH3-Testfehler", media_error(error, "Verzeichnis"))
