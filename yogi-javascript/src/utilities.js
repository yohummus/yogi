(function () {
    window.yogi = window.yogi || {};

    // run a function in the next JS engine tick
    window.yogi.nextTick = function (fn) {
        Promise.resolve().then(fn);
    };

    // HUB URI information
    let scripts = document.getElementsByTagName('script');
    for (let script of scripts) {
        let m = script.src.match(/^(https?):\/\/(.*?)(:(\d+))?\/.*yogi-hub(-all)?.min.js$/);
        if (m) {
            window.yogi.hubProtocol = m[1];
            window.yogi.hubHost = m[2];

            let port = m[4];
            if (typeof port === 'undefined') {
                port = {
                    'http': '80',
                    'https': '443'
                }[window.yogi.hubProtocol];
            }

            window.yogi.hubPort = parseInt(port);
            window.yogi.hubUri = window.yogi.hubProtocol + '://' + window.yogi.hubHost + ':' + window.yogi.hubPort;
            break;
        }
    }

    if (!window.yogi.hubUri) {
        console.error('Could not determine the URI of the YOGI-Hub webserver that the YOGI JS code was downloaded from.');
    }

    // downloading files
    window.yogi.downloadTextFile = function (filename, content) {
        let node = document.createElement('a');
        node.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(content));
        node.setAttribute('download', filename);
        node.style.display = 'none';
        document.body.appendChild(node);
        node.click();
        document.body.removeChild(node);
    };
})();
