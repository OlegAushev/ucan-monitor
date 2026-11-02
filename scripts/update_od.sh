#!/bin/bash
#
# Takes the description of the object dictionary of the adpt-etk-inverter
# firmware, which its build extracts from the ELF, into ucan-monitor:
#
#   scripts/update_od.sh ~/dev/adpt-etk-inverter [rev-a-release]
#
# The description is checked first, by tools/od_decode.py --check of the
# emblib the firmware is built with; if it fails, nothing changes. Then it
# goes to assets/od/, where ucan-monitor reads it at startup, and to
# tests/od/ go the firmware's example of frames with their expected decoding
# and the summary that --check prints, which the tests compare with. The
# description is the same in every preset of the firmware; the preset only
# says which build to take it from.

set -euo pipefail
export LC_ALL=C.UTF-8

die() {
  echo "update_od.sh: $*" >&2
  exit 1
}

(($# == 1 || $# == 2)) || die "usage: update_od.sh <firmware> [preset]"
[[ -d $1 ]] || die "$1: no such directory"
firmware=$(cd "$1" && pwd)
preset=${2:-rev-a-release}
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

description=$firmware/build/$preset/src/app/inverter/adpt-etk-inverter.od.jsonl
tool=$firmware/external/emblib/tools/od_decode.py
examples=$firmware/src/app/inverter/comm/can/canopen
[[ -f $description ]] || die "$description: no such file, build the preset $preset first"
[[ -f $tool ]] || die "$tool: no such file"
for name in pdo_example.candump pdo_example.expected; do
  [[ -f $examples/$name ]] || die "$examples/$name: no such file"
done

summary=$(python3 "$tool" --check "$description") || die "$description fails the check"
summary=${summary#"$description: "}

mkdir -p "$root/assets/od" "$root/tests/od"
cp -- "$description" "$root/assets/od/adpt-etk-inverter.od.jsonl"
cp -- "$examples/pdo_example.candump" "$examples/pdo_example.expected" "$root/tests/od/"
printf '%s\n' "$summary" >"$root/tests/od/description.expected"

if commit=$(git -C "$firmware" describe --always --dirty 2>/dev/null); then
  echo "adpt-etk-inverter $commit, $preset: $summary"
else
  echo "$summary"
fi
