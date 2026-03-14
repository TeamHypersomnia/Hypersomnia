#!/bin/bash
# test_web.sh
# Tests that play.hypersomnia.io is correctly configured for direct (non-iframe) access.
# Cross-origin isolation requires COOP: same-origin + COEP: require-corp.
# These enable SharedArrayBuffer (needed for multithreading).

set -e

SITE="https://play.hypersomnia.io"
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
    echo "  [FAIL] $label"
    echo "         Expected: $expected"
    echo "         Got:      $actual"
    FAIL=$((FAIL+1))
  fi
}

echo "=== test_web.sh: Checking play.hypersomnia.io headers ==="
echo

echo "--- Main page ($SITE/) ---"
HEADERS=$(curl -sI "$SITE/")
echo "$HEADERS"
echo

echo "--- Header checks ---"
check "COOP: same-origin (required for SharedArrayBuffer)" \
  "$(echo "$HEADERS" | grep -i cross-origin-opener)" \
  "same-origin"

check "COEP: credentialless (required for SharedArrayBuffer, compatible with iframe embedding)" \
  "$(echo "$HEADERS" | grep -i cross-origin-embedder)" \
  "credentialless"

check "CORP: cross-origin (allows cross-origin embedding of resources)" \
  "$(echo "$HEADERS" | grep -i cross-origin-resource)" \
  "cross-origin"

check "CSP frame-ancestors: allows iframing from any origin" \
  "$(echo "$HEADERS" | grep -i content-security-policy)" \
  "frame-ancestors"

echo
echo "--- JS file headers ---"
JS_HEADERS=$(curl -sI "$SITE/Hypersomnia.js")
check "CORP on Hypersomnia.js (required for cross-origin load)" \
  "$(echo "$JS_HEADERS" | grep -i cross-origin-resource)" \
  "cross-origin"

echo
echo "--- WASM file headers ---"
WASM_HEADERS=$(curl -sI "$SITE/Hypersomnia.wasm")
check "CORP on Hypersomnia.wasm" \
  "$(echo "$WASM_HEADERS" | grep -i cross-origin-resource)" \
  "cross-origin"

echo
check "Service-Worker-Allowed: / on /assets/ (allows SW registered from /assets/ to control /)" \
  "$(curl -sI "$SITE/assets/coi-serviceworker.js" | grep -i service-worker-allowed)" \
  "/"

echo
echo "Results: $PASS passed, $FAIL failed"
