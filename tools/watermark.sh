#!/bin/sh
# Reproducible source hash manifest, deliberately not a self-referential hash.
set -eu
export LC_ALL=C
cd "$(dirname "$0")/.."
find self q mercury cryptol saw c11 vulkan src -type f -print 2>/dev/null | sort | while IFS= read -r path; do
  sha256sum "$path"
done > WATERMARK.md.tmp
{
  printf '%s\n' '# AVIAN source watermark' '' 'SHA-256 source manifest (paths relative to this package):' '' '```text'
  cat WATERMARK.md.tmp
  printf '%s\n' '```'
} > WATERMARK.md
rm WATERMARK.md.tmp
