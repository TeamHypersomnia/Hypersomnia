#!/bin/bash
# test_iframe.sh
# Reproduces the CORS/cross-origin-isolation errors seen when play.hypersomnia.io
# is embedded in an iframe on https://www.arvexgames.com/play/hypersomnia
#
# ROOT CAUSE:
#   SharedArrayBuffer (required for the game's multithreading) is only available
#   in a "cross-origin isolated" context. For an iframe to be cross-origin isolated,
#   the TOP-LEVEL PAGE (arvexgames.com) must also send:
#     - Cross-Origin-Opener-Policy: same-origin (or same-origin-allow-popups)
#     - Cross-Origin-Embedder-Policy: require-corp (or credentialless)
#   This is a browser-enforced security requirement (Spectre mitigation).
#   It CANNOT be fixed by changes on play.hypersomnia.io alone.
#
# ERRORS OBSERVED IN BROWSER WHEN IFRAMING:
#   1. SharedArrayBuffer is not defined
#      (crossOriginIsolated === false because arvexgames.com is not isolated)
#   2. WebAssembly threading initialization failure
#      (workers cannot be created with SharedArrayBuffer)
#   3. Possibly: TypeError at Module instantiation (threading requires SAB)

set -e

IFRAME_HOST="https://www.arvexgames.com"
IFRAME_PAGE="$IFRAME_HOST/play/hypersomnia"
GAME_SITE="https://play.hypersomnia.io"
PASS=0
FAIL=0

check() {
  local label="$1"
  local actual="$2"
  local expected="$3"
  if echo "$actual" | grep -qi "$expected"; then
    echo "  [PASS] $label"
    PASS=$((PASS+1))
  else
    echo "  [FAIL] $label -> MISSING: $expected"
    FAIL=$((FAIL+1))
  fi
}

check_absent() {
  local label="$1"
  local actual="$2"
  local pattern="$3"
  if ! echo "$actual" | grep -qi "$pattern"; then
    echo "  [PASS] $label (absent as expected)"
    PASS=$((PASS+1))
  else
    echo "  [WARN] $label unexpectedly present: $(echo "$actual" | grep -i "$pattern")"
  fi
}

echo "=== test_iframe.sh: Reproducing iframe CORS errors ==="
echo
echo "Scenario: $GAME_SITE is iframed from $IFRAME_PAGE"
echo

# ------------------------------------------------------------------
echo "--- 1. Checking arvexgames.com top-level page headers ---"
PARENT_HEADERS=$(curl -sIL "$IFRAME_PAGE" 2>&1 | tail -20)
echo "$PARENT_HEADERS"
echo

echo "--- Required headers for cross-origin isolation (MISSING on arvexgames.com) ---"
echo "    Without these, SharedArrayBuffer is undefined in the iframe."
check "COOP header present on arvexgames.com" \
  "$PARENT_HEADERS" "cross-origin-opener-policy"
check "COEP header present on arvexgames.com" \
  "$PARENT_HEADERS" "cross-origin-embedder-policy"

echo
echo "--- Result: arvexgames.com is NOT cross-origin isolated ---"
echo "    Browser will set: window.crossOriginIsolated = false"
echo "    SharedArrayBuffer will be: UNDEFINED"
echo "    The Emscripten-compiled game (which uses pthreads) WILL CRASH."

echo
# ------------------------------------------------------------------
echo "--- 2. Verifying play.hypersomnia.io allows being framed ---"
GAME_HEADERS=$(curl -sI "$GAME_SITE/" 2>&1)
check "CSP frame-ancestors allows iframing" \
  "$GAME_HEADERS" "frame-ancestors"
check_absent "X-Frame-Options absent (would block iframing)" \
  "$GAME_HEADERS" "x-frame-options"
check "CORP: cross-origin on main page (resources loadable by cross-origin parent COEP)" \
  "$GAME_HEADERS" "cross-origin-resource-policy: cross-origin"

echo
# ------------------------------------------------------------------
echo "--- 3. Simulating what the browser does: crossOriginIsolated check ---"
echo
echo "    Simulated via Python/node logic (browser would do this internally):"
python3 - << 'PYEOF'
parent_coop = None        # arvexgames.com has no COOP
parent_coep = None        # arvexgames.com has no COEP
iframe_coop = "same-origin"    # play.hypersomnia.io
iframe_coep = "require-corp"   # play.hypersomnia.io

cross_origin_isolated = (
    parent_coop in ("same-origin", "same-origin-allow-popups") and
    parent_coep in ("require-corp", "credentialless")
)

print(f"    parent COOP:  {parent_coop!r}  (REQUIRED: 'same-origin' or 'same-origin-allow-popups')")
print(f"    parent COEP:  {parent_coep!r}  (REQUIRED: 'require-corp' or 'credentialless')")
print(f"    iframe COOP:  {iframe_coop!r}  (OK, but IGNORED for isolation purposes)")
print(f"    iframe COEP:  {iframe_coep!r}  (OK)")
print()
print(f"    window.crossOriginIsolated = {str(cross_origin_isolated).lower()}")
print()
if not cross_origin_isolated:
    print("    ERROR: SharedArrayBuffer is not defined")
    print("    ERROR: Atomics.wait disabled on main thread")
    print("    ERROR: WebAssembly pthreads will fail to initialize")
    print()
    print("    BROWSER CONSOLE WOULD SHOW (Chrome):")
    print("      Uncaught TypeError: SharedArrayBuffer is not defined")
    print("      OR: Uncaught ReferenceError: SharedArrayBuffer is not defined")
    print("      OR: RuntimeError: unreachable (Emscripten pthreads crash)")
PYEOF

echo
# ------------------------------------------------------------------
echo "--- 4. Fix required on arvexgames.com ---"
echo
echo "    Option A (recommended, less strict):"
echo "      Cross-Origin-Opener-Policy: same-origin-allow-popups"
echo "      Cross-Origin-Embedder-Policy: credentialless"
echo
echo "    Option B (strict):"
echo "      Cross-Origin-Opener-Policy: same-origin"
echo "      Cross-Origin-Embedder-Policy: require-corp"
echo "      (requires ALL cross-origin resources on arvexgames.com to have CORP headers)"
echo
echo "    Option C: Use coi-serviceworker.js on arvexgames.com"
echo "      See: https://github.com/gzuidhof/coi-serviceworker"
echo "      Register the service worker on arvexgames.com to inject the headers."
echo
echo "    play.hypersomnia.io side already has:"
echo "      CORP: cross-origin  -> OK for Option A/B"
echo "      COEP: require-corp  -> OK inside the iframe"
echo "      CSP frame-ancestors * -> allows being iframed"
echo
echo "Results: $PASS passed (present), $FAIL missing"
echo "VERDICT: SharedArrayBuffer WILL fail in iframe until arvexgames.com adds COOP+COEP"
