#!/usr/bin/env python3

import json
import argparse

DB_ROOT_KEY = "ROOT"


def db_add(dbs: list, patterns: list, value: str):
    if len(patterns) == 1:
        for db in dbs:
            if not isinstance(db, dict):
                continue
            db[patterns[0]] = value
        return
    next_dbs = []
    for db in dbs:
        if not isinstance(db, dict):
            continue
        for key in db.keys():
            if patterns[0] in key:
                next_dbs.append(db[key])
    db_add(next_dbs, patterns[1:], value)
        

def main():
    parser = argparse.ArgumentParser(description="add new section to db")
    parser.add_argument("-i", "--input", type=str, required=True, help="Input file")
    parser.add_argument("-o", "--output", type=str, required=True, help="Output file")
    parser.add_argument("-v", "--value", type=str, required=True, help="Default value")
    parser.add_argument("patterns", nargs="+", help="List of patterns")

    args = parser.parse_args()

    print(f"add {args.patterns} = {args.value} from {args.input} to {args.output}")
    with open(args.input, "r") as fin:
        db = json.load(fin)
        db_add([db], args.patterns, args.value)
        with open(args.output, "w") as fout:
            json.dump(db, fout, sort_keys=False, indent=4)

if __name__ == "__main__":
    main()
