/*
 * When loaded as a Service Worker, intercept fetch requests and add
 * COOP/COEP/CORP headers to enable cross-origin isolation (SharedArrayBuffer).
 */
self.addEventListener("install", () => self.skipWaiting());
self.addEventListener("activate", (e) => e.waitUntil(self.clients.claim()));
self.addEventListener("fetch", function (e) {
  if (e.request.cache === "only-if-cached" && e.request.mode !== "same-origin") return;
  e.respondWith(fetch(e.request).then((r) => {
    if (r.status === 0) return r;
    const headers = new Headers(r.headers);
    if (e.request.mode === "navigate") {
      const url = new URL(e.request.url);
      if (url.pathname.endsWith('/discord_redirect.html')) {
        // OAuth redirect page must NOT have COOP so window.opener survives
        // the cross-origin navigation chain (game iframe -> Discord -> redirect)
        headers.delete("Cross-Origin-Opener-Policy");
      } else {
        headers.set("Cross-Origin-Opener-Policy", "same-origin");
      }
    }
    headers.set("Cross-Origin-Embedder-Policy", "credentialless");
    headers.set("Cross-Origin-Resource-Policy", "cross-origin");
    return new Response(r.body, { status: r.status, statusText: r.statusText, headers });
  }).catch((e) => console.error(e)));
});
