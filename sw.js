const CACHE_NAME = "mkwii-web-assets-v3";
const APP_FILES = [
  "./",
  "./index.html",
  "./manifest.webmanifest",
  "./dist/mkwii-web.js",
  "./dist/mkwii-web.wasm"
];

self.addEventListener("install", event => {
  event.waitUntil(
    caches.open(CACHE_NAME)
      .then(cache => cache.addAll(APP_FILES))
      .then(() => self.skipWaiting())
  );
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

  // Prefer the latest deployed files while online; keep the cached copy as an
  // offline fallback. This prevents an old HTML/JS pair from surviving deploys.
  event.respondWith((async () => {
    const cache = await caches.open(CACHE_NAME);
    try {
      const fresh = await fetch(request);
      if (fresh.ok) await cache.put(request, fresh.clone());
      return fresh;
    } catch (error) {
      const cached = await cache.match(request);
      if (cached) return cached;
      throw error;
    }
  })());
});
