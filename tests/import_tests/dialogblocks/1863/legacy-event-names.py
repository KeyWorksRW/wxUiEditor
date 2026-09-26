"""Legacy event names that the wxUiEditor importers cannot translate.

wxWidgets 2.x called almost every event wxEVT_COMMAND_*.  wxWidgets 3 renamed them
and kept the old name as an alias, for instance

    #define wxEVT_COMMAND_TEXT_UPDATED wxEVT_TEXT

Designers of that era write the old name into their project files, because that was
the name at the time.  ImportXML::GetCorrectEventName translates by looking the name
up in map_old_events, which has 27 entries.  Anything not in that table reaches
get_Event() with the old name, is not found, and the handler is dropped without a
message.

This is measured rather than recalled.  On one side the #define aliases from the
wxWidgets headers say what the modern name is.  On the other, the <event name="wxEVT_*">
entries of the wxUiEditor generator definitions say which of those modern names the
designer would be able to bind.  The intersection, minus the 27 already translated,
is the list of handlers that are lost although they are perfectly expressible.

Usage:

    python3 legacy-event-names.py <wxWidgets-source-dir> <wxUiEditor-source-dir>

It reads include/**/*.h from the wxWidgets tree, and src/import/import_xml.cpp and
src/xml/*.xml from the wxUiEditor tree.  Nothing is written.
"""
import collections
import glob
import os
import re
import sys


def wx_aliases(wx_include_dir):
    """old name -> modern name, as wxWidgets declares it."""
    pairs = {}
    for root, _dirs, files in os.walk(wx_include_dir):
        for name in files:
            if not name.endswith(".h"):
                continue
            text = open(os.path.join(root, name), encoding="iso-8859-1").read()
            for old, new in re.findall(
                    r"^#define\s+(wxEVT_COMMAND_\w+)\s+(wxEVT_\w+)\s*$", text, re.M):
                # A real alias renames something.  If the two names are equal it is a
                # circular definition rather than a translation.
                if old != new:
                    pairs[old] = new
    return pairs


def translated_by_importer(src_dir):
    """The entries of map_old_events, read from the source rather than assumed."""
    text = open(os.path.join(src_dir, "import", "import_xml.cpp"),
                encoding="utf-8").read()
    block = text.split("map_old_events = frozen::make_map", 1)[1]
    block = block.split("});", 1)[0]
    return set(re.findall(r'\{\s*"(wxEVT_COMMAND_\w+)"', block))


def declared_by_generators(src_dir):
    """event name -> the generators that declare it.

    Resolving <inherits> is not needed here.  The question is whether any generator
    declares that name, that is whether the designer can bind it somewhere.  Which
    classes declare it and which do not is a separate question, covered by the report
    on events that cannot be bound at all.
    """
    per_event = collections.defaultdict(list)
    for path in sorted(glob.glob(os.path.join(src_dir, "xml", "*.xml"))):
        text = open(path, encoding="utf-8").read()
        for m in re.finditer(r'<event\s+name="(wxEVT_\w+)"', text):
            before = text[:m.start()]
            generators = re.findall(r'<gen\s+class="([^"]+)"', before)
            per_event[m.group(1)].append(generators[-1] if generators else "?")
    return per_event


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 1
    wx_root, wxui_root = sys.argv[1], sys.argv[2]
    include_dir = os.path.join(wx_root, "include")
    src_dir = os.path.join(wxui_root, "src")
    for path in (include_dir, os.path.join(src_dir, "xml")):
        if not os.path.isdir(path):
            print("not a directory:", path)
            return 2

    aliases = wx_aliases(include_dir)
    known = translated_by_importer(src_dir)
    declared = declared_by_generators(src_dir)

    missing = {old: new for old, new in aliases.items()
               if old not in known and new in declared}

    print("wxEVT_COMMAND_* aliases declared by wxWidgets : %d" % len(aliases))
    print("entries in map_old_events                     : %d" % len(known))
    print("missing, but a generator declares the new name: %d" % len(missing))
    print()

    # Grouped by family, which is how they read and how they would be added.
    families = collections.defaultdict(list)
    for old in sorted(missing):
        family = old[len("wxEVT_COMMAND_"):].split("_")[0]
        families[family].append(old)

    for family in sorted(families):
        entries = families[family]
        generators = sorted(set(g for e in entries for g in declared[missing[e]]))
        print("%-16s %2d  (%s)" % (family, len(entries), ", ".join(generators[:4])))
        for old in entries:
            print("    %-52s -> %s" % (old, missing[old]))
        print()
    return 0


sys.exit(main())
