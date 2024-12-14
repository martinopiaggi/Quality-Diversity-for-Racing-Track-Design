"""Generate TORCS race setup for simulation and track export."""

__author__ = "Jacopo Sirianni"
__copyright__ = "Copyright 2015-2016, Jacopo Sirianni"
__license__ = "GPL"
__email__ = "jacopo.sirianni@mail.polimi.it"

import os

# Fixed bot configuration - these are always used for racing
RACING_BOTS = [
    ("simplix_V1_10", "0"),
    ("bt", "1"),
    ("tita", "4"),
    ("olethros", "2"),
    ("damned", "3")
]

# Track exporter bot
TRACK_EXPORTER = ("trackexporter", "1")

def generate_track_export_xml(path):
    """Generate XML for track export (1 lap, trackexporter only)"""
    xml = '''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE params SYSTEM "../libs/tgf/params.dtd">

<params name="Quick Race" type="param" mode="mw">
    <section name="Tracks">
        <section name="1">
            <attstr name="name" val="output"/>
            <attstr name="category" val="road"/>
        </section>
    </section>

    <section name="Races">
        <section name="1">
            <attstr name="name" val="Quick Race"/>
        </section>
    </section>

    <section name="Quick Race">
        <attnum name="laps" val="1"/>
        <attstr name="type" val="race"/>
        <attstr name="starting order" val="random"/>
    </section>

    <section name="Drivers">
        <section name="1">
            <attnum name="idx" val="1"/>
            <attstr name="module" val="trackexporter"/>
        </section>
    </section>
</params>'''
    
    with open(path, "w") as f:
        f.write(xml)

def generate_race_xml(path, num_laps=10):
    """Generate XML for race simulation with all standard bots"""
    # Generate drivers section with all racing bots
    drivers_section = ""
    for i, (bot_name, bot_idx) in enumerate(RACING_BOTS, 1):
        drivers_section += f'''
        <section name="{i}">
            <attnum name="idx" val="{bot_idx}"/>
            <attstr name="module" val="{bot_name}"/>
        </section>'''

    xml = f'''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE params SYSTEM "../libs/tgf/params.dtd">

<params name="Quick Race" type="param" mode="mw">
    <section name="Tracks">
        <section name="1">
            <attstr name="name" val="output"/>
            <attstr name="category" val="road"/>
        </section>
    </section>

    <section name="Races">
        <section name="1">
            <attstr name="name" val="Quick Race"/>
        </section>
    </section>

    <section name="Quick Race">
        <attnum name="laps" val="{num_laps}"/>
        <attstr name="type" val="race"/>
        <attstr name="starting order" val="random"/>
    </section>

    <section name="Drivers">{drivers_section}
    </section>
</params>'''
    
    with open(path, "w") as f:
        f.write(xml)