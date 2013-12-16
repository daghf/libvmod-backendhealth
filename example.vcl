import backendhealth;

backend b1 {
    .host = "127.0.0.1";
    .port = "8081";
    .probe = { .initial = 0; }
}

backend b2 {
    .host = "127.0.0.1";
    .port = "8082";
    .probe = { .initial = 0; }
}


backend b3 {
    .host = "127.0.0.1";
    .port = "8080";
    .probe = { .initial = 0; }
}

director foo round-robin {
    { .backend = b1; }
    { .backend = b2; }
    { .backend = b3; }
}

acl localhost {
    "localhost";
}

sub vcl_recv {
    set req.backend = foo;

    if (req.url ~ "/backend_health" && client.ip ~ localhost) {
        error 777 "OK";
    }
}

sub vcl_error {
    if (obj.status == 777) {
        set obj.status = 200;
        
        # JSONP
        if (req.url ~ "\?callback=\w+") {
            set req.http.cb = regsub(req.url, ".*\?callback=(\w+).*", "\1");
            set obj.http.Content-Type = "application/javascript";
            synthetic req.http.cb + {"("} + backendhealth.json(false) + {")"};
        }

        # JSON
        else {
            set obj.http.Content-Type = "application/json";
            synthetic backendhealth.json(true);
        }
        
        return (deliver);
    }
}
