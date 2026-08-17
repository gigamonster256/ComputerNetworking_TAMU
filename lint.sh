#!/usr/bin/env bash
#
# Lint C/C++ sources with clang-tidy.
#
# Requires `clang-tidy` and `fd` on PATH (e.g. `nix develop`).
# Additional arguments are passed to fd to scope the search,
# e.g. `./lint.sh libtcp libsbcp` or `./lint.sh -e cpp MP2/src`.

set -euo pipefail

INCLUDES=(-Ilibtcp/include -Ilibsbcp/include -Ilibudp/include -Ilibtftp/include -Ilibhttp/include)

mapfile -t FILES < <(fd "$@" -t f -e cpp -e c -E chatgpt -E .direnv -E result)

status=0
for f in "${FILES[@]}"; do
  clang-tidy --quiet --warnings-as-errors='*' --header-filter='.*' "$f" -- -std=c++17 "${INCLUDES[@]}" || status=1
done
exit $status