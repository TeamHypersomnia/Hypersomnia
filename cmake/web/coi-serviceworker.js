/*
 * Cross-Origin Isolation Service Worker
 *
 * Enables SharedArrayBuffer in cross-origin iframes by intercepting
 * fetch requests and adding COEP/CORP headers via a Service Worker.
 *
 * When loaded as a <script>, it checks if the page is already
 * cross-origin isolated. If not, it registers itself as a Service Worker
 * and reloads the page so the SW can add the required headers.
 *
 * When running as a Service Worker, it intercepts all fetch requests
 * and adds Cross-Origin-Embedder-Policy and Cross-Origin-Resource-Policy
 * headers to enable cross-origin isolation.
 */

if (typeof window === 'undefined') {
  // Running as a Service Worker
  self.addEventListener("install", () => self.skipWaiting());
  self.addEventListener("activate", (e) => e.waitUntil(self.clients.claim()));

  self.addEventListener("fetch", function (e) {
    if (e.request.cache === "only-if-cached" && e.request.mode !== "same-origin") {
      return;
    }

    e.respondWith(
      fetch(e.request).then((r) => {
        const headers = new Headers(r.headers);
        headers.set("Cross-Origin-Embedder-Policy", "credentialless");
        headers.set("Cross-Origin-Resource-Policy", "cross-origin");
        return new Response(r.body, {
          status: r.status,
          statusText: r.statusText,
          headers,
        });
      })
    );
  });
} else {
  // Running as a regular script in the window context
  (async function () {
    if (window.crossOriginIsolated !== false) return;

    const registration = await navigator.serviceWorker
      .register(window.document.currentScript.src)
      .catch((e) =>
        console.log("COOP/COEP Service Worker failed to register:", e)
      );

    if (registration) {
      console.log("COOP/COEP Service Worker registered", registration.scope);

      registration.addEventListener("updatefound", () => {
        const worker = registration.installing;
        worker.addEventListener("statechange", () => {
          if (worker.state === "activated") {
            console.log(
              "Reloading page to enable cross-origin isolation via Service Worker."
            );
            window.location.reload();
          }
        });
      });

      if (registration.active && !navigator.serviceWorker.controller) {
        console.log(
          "Reloading page to enable cross-origin isolation via Service Worker."
        );
        window.location.reload();
      }
    }
  })();
}
