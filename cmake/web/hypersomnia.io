server {
    listen 80;
    server_name hypersomnia.io www.hypersomnia.io;

    root /var/www/html;
    index Hypersomnia.html;

    location / {
        try_files $uri $uri/ =404;
    }

    # MIME types for WebAssembly (.wasm) files
    types {
        application/wasm     wasm;
        text/html            html;
        text/javascript      js;
    }

    # Add Cross-Origin-Opener-Policy and Cross-Origin-Embedder-Policy headers
	add_header Cross-Origin-Resource-Policy "same-site";
    add_header Cross-Origin-Opener-Policy "same-origin";
    add_header Cross-Origin-Embedder-Policy "require-corp";
}
