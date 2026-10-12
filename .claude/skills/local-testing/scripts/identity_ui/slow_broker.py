#!/usr/bin/env python3
# A proxy to the local broker (13500) answering after DELAY seconds - a slow hypersomnia.io. Usage: slow_broker.py [port] [delay]
import sys, time, urllib.request
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 13599
DELAY = float(sys.argv[2]) if len(sys.argv) > 2 else 4.0

class H(BaseHTTPRequestHandler):
	def forward(self, method):
		time.sleep(DELAY)
		n = int(self.headers.get('Content-Length') or 0)
		body = self.rfile.read(n) if n else None
		req = urllib.request.Request('http://127.0.0.1:13500' + self.path, data=body, method=method)
		for k in ('Content-Type',):
			if self.headers.get(k):
				req.add_header(k, self.headers[k])
		try:
			r = urllib.request.urlopen(req, timeout=10)
			code, data = r.status, r.read()
		except urllib.error.HTTPError as e:
			code, data = e.code, e.read()
		self.send_response(code)
		self.send_header('Content-Length', str(len(data)))
		self.end_headers()
		self.wfile.write(data)

	def do_GET(self):
		self.forward('GET')

	def do_POST(self):
		self.forward('POST')

	def log_message(self, *a):
		pass

ThreadingHTTPServer(('127.0.0.1', PORT), H).serve_forever()
