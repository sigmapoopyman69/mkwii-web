const CACHE_NAME = "mkwii-web-assets-v4";

// Do not download the whole application during service-worker installation.
// In particular, a large WASM file must not block activation.
self.addEventListener("install", event => {
  event.waitUntil(self.skipWaiting());
});

self.addEventListener("activate", event => {
  event.waitUntil(
    caches.keys()
      .then(keys => Promise.all(keys
        .filter(key => key.startsWith("mkwii-web-assets-") && key !== CACHE_NAME)
        .map(key => caches.delete(key))))
      .then(() => self.clients.claim())
  );
});

self.addEventListener("fetch", event => {
  const request = event.request;
  const url = new URL(request.url);
  if (url.origin !== self.location.origin || request.method !== "GET") return;

  const isAppAsset =
    url.pathname.includes("/dist/") ||
    url.pathname.endsWith("/index.html") ||
    url.pathname.endsWith("/manifest.webmanifest") ||
    url.pathname.endsWith("/");

  if (!isAppAsset) return;

  // Network-first: return the response as soon as it arrives. Cache the clone
  // in the background so large JS/WASM downloads cannot hold up page loading.
  event.respondWith((async () => {
    const cache = await caches.open(CACHE_NAME);
    try {
      const fresh = await fetch(request);
      if (fresh.ok) {
        event.waitUntil(cache.put(request, fresh.clone()).catch(() => {}));
      }
      return fresh;
    } catch (error) {
      const cached = await cache.match(request);
      if (cached) return cached;
      throw error;
    }
  })());
});
