#!/usr/bin/env python3
"""
Serves a directory over HTTP slowly (an external arena files provider on a bad connection): /arenas/<map>/<file>.
Usage: slow_http.py <dir with arenas/> <port> <bytes per second>
"""
import http.server, os, sys, time, socketserver

root, port, rate = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])

class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        path = os.path.normpath(os.path.join(root, self.path.split('?')[0].lstrip('/')))

        if not path.startswith(os.path.abspath(root)) or not os.path.isfile(path):
            self.send_response(404)
            self.end_headers()
            return

        data = open(path, 'rb').read()
        self.send_response(200)
        self.send_header('Content-Length', str(len(data)))
        # a game page that is cross-origin isolated fetches it from another origin
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Cross-Origin-Resource-Policy', 'cross-origin')
        self.end_headers()

        chunk = max(1, rate // 10)

        for i in range(0, len(data), chunk):
            self.wfile.write(data[i:i + chunk])
            self.wfile.flush()
            time.sleep(0.1)

    def log_message(self, *args):
        pass

class Server(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True

Server(('127.0.0.1', port), Handler).serve_forever()
