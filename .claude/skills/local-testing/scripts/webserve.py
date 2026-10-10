#!/usr/bin/env python3
"""
Serves a Hypersomnia web build for local testing: python3 webserve.py <build dir> <port>

- Cross-origin isolation headers (COOP/COEP), needed for the WASM threads (SharedArrayBuffer).
- Any path serves the game, so /game/<connect string> joins that server right away:
  the page's relative assets (Hypersomnia.js, assets/common.js...) resolve by their name wherever they are requested from.
"""
import http.server
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


class Handler(http.server.BaseHTTPRequestHandler):
	def resolve(self):
		path = self.path.split("?", 1)[0]

		if "/assets/" in path:
			return os.path.join(ROOT, "assets", path.split("/assets/", 1)[1])

		name = os.path.basename(path)

		if name and os.path.isfile(os.path.join(ROOT, name)):
			return os.path.join(ROOT, name)

		return os.path.join(ROOT, "Hypersomnia.html")

	def do_GET(self):
		file = self.resolve()

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
