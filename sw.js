// WiiCompiled Web diagnostic service worker v7.
// Deliberately does not intercept fetches: all page assets use normal browser networking.
// This isolates whole-tab loading problems from Cache Storage and response interception.
self.addEventListener("install", event => {
  event.waitUntil(self.skipWaiting());
});
self.addEventListener("activate", event => {
  event.waitUntil(
    caches.keys()
      .then(keys => Promise.all(
        keys.filter(key => key.startsWith("mkwii-web-assets-")).map(key => caches.delete(key))
      ))
      .then(() => self.clients.claim())
  );
});
