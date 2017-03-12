(function () {
    class ProtoCompiler {
        constructor() {
        }

        compileToPython(protoFile) {
            return this._compile(protoFile, 'python', false);
        }

        compileToPythonAndDownload(protoFile) {
            return this._compile(protoFile, 'python', true);
        }

        compileToCpp(protoFile) {
            return this._compile(protoFile, 'cpp', false);
        }

        compileToCppAndDownload(protoFile) {
            return this._compile(protoFile, 'cpp', true);
        }

        compileToCSharp(protoFile) {
            return this._compile(protoFile, 'csharp', false);
        }

        compileToCSharpAndDownload(protoFile) {
            return this._compile(protoFile, 'csharp', true);
        }

        _compile(protoFile, language, download) {
            return new Promise((resolve, reject) => {
                let xhttp = new XMLHttpRequest();
                xhttp.timeout = 10000;
                xhttp.open('POST', window.yogi.hubUri + '/compile/' + language, true);
                xhttp.send(protoFile.content);

                xhttp.onreadystatechange = () => {
                    if (xhttp.readyState != xhttp.DONE) {
                        return;
                    }

                    if (xhttp.status === 201) {
                        try {
                            let files = JSON.parse(xhttp.responseText);

                            if (download) {
                                for (let filename in files) {
                                    window.yogi.downloadTextFile(filename, files[filename]);
                                }
                            }

                            resolve(files);
                        }
                        catch (err) {
                            reject(`Cannot parse JSON content returned from proto compiler: ${err}`);
                        }
                    }
                    else {
                        let err = xhttp.status === 0 ? 'The operation timed out' : xhttp.statusText;
                        reject(`Failed to compile the proto file: ${xhttp.responseText}`);
                    }
                };
            });
        }
    }

    window.yogi = window.yogi || {};
    window.yogi.ProtoCompiler = ProtoCompiler;
})();
