#!/usr/bin/env python3
"""
Serves a Hypersomnia web build for local testing: python3 webserve.py <build dir> <port>

- Cross-origin isolation headers (COOP/COEP), needed for the WASM threads (SharedArrayBuffer).
- Any path serves the game, so /game/<connect string> joins that server right away:
  the page's relative assets (Hypersomnia.js, assets/common.js...) resolve by their name wherever they are requested from.
"""
import http.server
import json
import os
import sys

ROOT = os.path.abspath(sys.argv[1])
PORT = int(sys.argv[2])

TYPES = {
	".html": "text/html",
	".js": "application/javascript",
	".wasm": "application/wasm",
	".data": "application/octet-stream",
	".css": "text/css",
	".png": "image/png",
	".svg": "image/svg+xml",
	".json": "application/json",
	".ico": "image/x-icon",
	".woff2": "font/woff2",
}


CLEAR_USER_DATA_PATH = "/clear-user-data"
IDBFS_MOUNT_POINTS_FROM_COMMON_JS = ["/user", "/hypersomnia_keys"]

CLEAR_USER_DATA_PAGE = """<!doctype html>
<meta charset="utf-8">
<title>Clearing user data</title>
<pre id="log">Clearing the game's user data of this origin...</pre>
<script>
const log = text => { console.log(text); document.getElementById("log").textContent += "\\n" + text; };

const delete_database = name => new Promise(resolve => {
	const request = indexedDB.deleteDatabase(name);
	request.onsuccess = () => { log("deleted " + name); resolve(); };
	request.onerror = () => { log("failed to delete " + name); resolve(); };
	request.onblocked = () => log(name + " is still open - close the game's tab");
});

(async () => {
	const list_databases = async () => {
		try {
			return indexedDB.databases ? (await indexedDB.databases()).map(database => database.name) : [];
		}
		catch (error) {
			log("could not list the databases (" + error + "), deleting the known ones");
			return [];
		}
	};

	const listed = await list_databases();

	for (const name of new Set([...%s, ...listed])) {
		await delete_database(name);
	}

	localStorage.clear();
	sessionStorage.clear();
	log("done - starting the game");
	location.replace("/");
})();
</script>
""" % json.dumps(IDBFS_MOUNT_POINTS_FROM_COMMON_JS)


class Handler(http.server.BaseHTTPRequestHandler):
	def send_clear_user_data_page(self):
		body = CLEAR_USER_DATA_PAGE.encode("utf-8")
		self.send_response(200)
		self.send_header("Content-Type", "text/html")
		self.send_header("Content-Length", str(len(body)))
		self.send_header("Cache-Control", "no-store")
		self.end_headers()
		self.wfile.write(body)

	def resolve(self):
		path = self.path.split("?", 1)[0]

		if "/assets/" in path:
			return os.path.join(ROOT, "assets", path.split("/assets/", 1)[1])

		name = os.path.basename(path)

		if name and os.path.isfile(os.path.join(ROOT, name)):
			return os.path.join(ROOT, name)

		return os.path.join(ROOT, "Hypersomnia.html")

	def do_GET(self):
		if self.path.split("?", 1)[0] == CLEAR_USER_DATA_PATH:
			self.send_clear_user_data_page()
			return

		file = self.resolve()

		# Nothing outside the build and its assets (a symlink), whatever ../ the path holds
		real = os.path.realpath(file)
		allowed = [os.path.realpath(ROOT), os.path.realpath(os.path.join(ROOT, "assets"))]

		if not any(real == base or real.startswith(base + os.sep) for base in allowed):
			self.send_error(404)
			return

		if not os.path.isfile(file):
			self.send_error(404)
			return

		with open(file, "rb") as f:
			body = f.read()

		self.send_response(200)
		self.send_header("Content-Type", TYPES.get(os.path.splitext(file)[1], "application/octet-stream"))
		self.send_header("Content-Length", str(len(body)))
		self.send_header("Cross-Origin-Opener-Policy", "same-origin")
		self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
		self.send_header("Cross-Origin-Resource-Policy", "same-origin")
		self.send_header("Cache-Control", "no-store")
		self.end_headers()
		self.wfile.write(body)

	def log_message(self, fmt, *args):
		sys.stderr.write("%s\n" % (fmt % args))


http.server.ThreadingHTTPServer(("127.0.0.1", PORT), Handler).serve_forever()
