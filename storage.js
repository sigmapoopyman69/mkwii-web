/* WiiCompiled Web v0.4 browser storage adapter.
 * All records stay in this browser's origin-local IndexedDB. Nothing is uploaded.
 */
(() => {
  "use strict";
  const DB_NAME = "wiicompiled-web-v04";
  const DB_VERSION = 1;
  const FILE_STORE = "files";
  const KV_STORE = "kv";
  const MAX_MOUNT_BYTES = 128 * 1024 * 1024;
  let databasePromise;

  function openDatabase() {
    if (databasePromise) return databasePromise;
    databasePromise = new Promise((resolve, reject) => {
      if (!("indexedDB" in globalThis)) {
        reject(new Error("IndexedDB is unavailable."));
        return;
      }
      const request = indexedDB.open(DB_NAME, DB_VERSION);
      request.onupgradeneeded = () => {
        const db = request.result;
        if (!db.objectStoreNames.contains(FILE_STORE)) {
          db.createObjectStore(FILE_STORE, { keyPath: "id" });
        }
        if (!db.objectStoreNames.contains(KV_STORE)) {
          db.createObjectStore(KV_STORE, { keyPath: "key" });
        }
      };
      request.onsuccess = () => resolve(request.result);
      request.onerror = () => reject(request.error || new Error("Could not open IndexedDB."));
      request.onblocked = () => reject(new Error("Database upgrade is blocked by another open tab."));
    });
    return databasePromise;
  }

  function requestResult(request) {
    return new Promise((resolve, reject) => {
      request.onsuccess = () => resolve(request.result);
      request.onerror = () => reject(request.error || new Error("IndexedDB request failed."));
    });
  }

  async function transaction(storeName, mode, operation) {
    const db = await openDatabase();
    return new Promise((resolve, reject) => {
      const tx = db.transaction(storeName, mode);
      const store = tx.objectStore(storeName);
      let result;
      try { result = operation(store); }
      catch (error) { reject(error); return; }
      tx.oncomplete = () => resolve(result && result.result);
      tx.onerror = () => reject(tx.error || new Error("IndexedDB transaction failed."));
      tx.onabort = () => reject(tx.error || new Error("IndexedDB transaction was aborted."));
    });
  }

  async function saveSetting(key, value) {
    await transaction(KV_STORE, "readwrite", store => store.put({ key, value }));
    return true;
  }

  async function loadSetting(key) {
    const record = await transaction(KV_STORE, "readonly", store => store.get(key));
    return record ? record.value : null;
  }

  async function saveFile(file) {
    if (!(file instanceof Blob)) throw new TypeError("A browser File or Blob is required.");
    const id = globalThis.crypto && crypto.randomUUID
      ? crypto.randomUUID()
      : "file-" + Date.now() + "-" + Math.random().toString(16).slice(2);
    const record = {
      id,
      name: String(file.name || "selected-file").replace(/[\\/]/g, "_"),
      type: file.type || "application/octet-stream",
      size: file.size,
      lastModified: Number(file.lastModified || Date.now()),
      blob: file
    };
    await transaction(FILE_STORE, "readwrite", store => store.put(record));
    return { id, name: record.name, type: record.type, size: record.size };
  }

  async function listFiles() {
    const records = await transaction(FILE_STORE, "readonly", store => store.getAll());
    return (records || []).map(({ id, name, type, size, lastModified }) =>
      ({ id, name, type, size, lastModified }));
  }

  async function getFile(id) {
    const record = await transaction(FILE_STORE, "readonly", store => store.get(id));
    if (!record) throw new Error("The selected file is no longer in browser storage.");
    return record;
  }

  async function deleteFile(id) {
    await transaction(FILE_STORE, "readwrite", store => store.delete(id));
    return true;
  }

  async function mountFile(id, module, destination) {
    if (!module || !module.FS || typeof module.FS.writeFile !== "function") {
      throw new Error("The WebAssembly module does not expose Emscripten FS.");
    }
    const record = await getFile(id);
    if (record.size > MAX_MOUNT_BYTES) {
      throw new Error("This prototype limits in-memory WASM mounts to 128 MiB. The file remains safely stored in IndexedDB.");
    }
    const bytes = new Uint8Array(await record.blob.arrayBuffer());
    const path = destination || ("/user/" + record.name);
    try { module.FS.mkdir("/user"); } catch (_) {}
    module.FS.writeFile(path, bytes);
    return { path, size: bytes.byteLength };
  }

  async function estimateStorage() {
    if (navigator.storage && navigator.storage.estimate) {
      return await navigator.storage.estimate();
    }
    return null;
  }

  globalThis.WiiWebStorage = Object.freeze({
    openDatabase, saveSetting, loadSetting, saveFile, listFiles,
    getFile, deleteFile, mountFile, estimateStorage,
    maxMountBytes: MAX_MOUNT_BYTES
  });
})();
