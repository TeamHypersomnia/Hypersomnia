server {
    server_name play.hypersomnia.io www.play.hypersomnia.io;

    root /var/www/html;
    index Hypersomnia.html;

    location / {
        # Try to serve file directly, then directories, then fall back to Hypersomnia.html
        try_files $uri $uri/ /Hypersomnia.html;

        # Allow embedding in iframes from any origin
        add_header Content-Security-Policy "frame-ancestors *";

        # Add Cross-Origin-Opener-Policy and Cross-Origin-Embedder-Policy headers
        add_header Cross-Origin-Resource-Policy "same-site";
        add_header Cross-Origin-Opener-Policy "same-origin";
        add_header Cross-Origin-Embedder-Policy "require-corp";
    }

    location /assets/ {
        alias /var/www/html/assets/;
        autoindex on;  # This is optional; it allows directory listing if no index file is found

        # Add Cross-Origin-Opener-Policy and Cross-Origin-Embedder-Policy headers
        add_header Cross-Origin-Resource-Policy "same-site";
        add_header Cross-Origin-Opener-Policy "same-origin";
        add_header Cross-Origin-Embedder-Policy "require-corp";
    }

    # MIME types for WebAssembly (.wasm) files
    types {
        application/wasm     wasm;
        application/octet-stream data;
        text/html            html;
        text/javascript      js;
        text/css      css;
    }

    # Add Cross-Origin-Opener-Policy and Cross-Origin-Embedder-Policy headers
    add_header Cross-Origin-Resource-Policy "same-site";
    add_header Cross-Origin-Opener-Policy "same-origin";
    add_header Cross-Origin-Embedder-Policy "require-corp";

    gzip on;
    gzip_types application/wasm application/octet-stream;
    gzip_min_length 10240;
    gzip_comp_level 9;
    gzip_proxied no-cache no-store private expired auth;

    listen 443 ssl http2;
    ssl_certificate /etc/letsencrypt/live/play.hypersomnia.io/fullchain.pem; # managed by Certbot
    ssl_certificate_key /etc/letsencrypt/live/play.hypersomnia.io/privkey.pem; # managed by Certbot
    include /etc/letsencrypt/options-ssl-nginx.conf; # managed by Certbot
    ssl_dhparam /etc/letsencrypt/ssl-dhparams.pem; # managed by Certbot
}

# HTTP server block for redirecting to HTTPS
server {
    if ($host = www.play.hypersomnia.io) {
        return 301 https://$host$request_uri;
    } # managed by Certbot

    if ($host = play.hypersomnia.io) {
        return 301 https://$host$request_uri;
    } # managed by Certbot

    listen 80;
    server_name play.hypersomnia.io www.play.hypersomnia.io;
    return 404; # managed by Certbot
}
