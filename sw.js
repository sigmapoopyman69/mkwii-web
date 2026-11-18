const CACHE_NAME = "mkwii-web-assets-v6";

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
    url.pathname.endsWith("/storage.js") ||
    url.pathname.endsWith("/manifest.webmanifest") ||
    url.pathname.endsWith("/");

  if (!isAppAsset) return;

  const networkResponse = fetch(request);

  // Register cache work synchronously, but do not make page loading wait for it.
  event.waitUntil(
    networkResponse.then(async response => {
      if (response.ok) {
        const cache = await caches.open(CACHE_NAME);
        await cache.put(request, response.clone());
      }
    }).catch(() => {})
  );

  event.respondWith(
    networkResponse.catch(async () => {
      const cache = await caches.open(CACHE_NAME);
      const cached = await cache.match(request);
      if (cached) return cached;
      throw new Error("Network unavailable and no cached app response exists.");
    })
  );
});
