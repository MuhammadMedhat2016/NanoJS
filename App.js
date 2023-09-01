/**
 * App.js
 *
 * Prints the current version of Lemon.
 * You can use this file for testing Lemon by running './lemon App.js'
 *
 */

const fs = internalBinding("fs");

fs.writeFileAsync("Hello.txt", "Hello World from NEST.JS" ,(bytesRead) =>
  log(`Done Bytes Written are ${bytesRead}`)
);


setTimeOut(() => log("Hello after 1 sec"), 1000);
setTimeOut(() => log("Hello after 3 sec"), 3000);
setTimeOut(() => log("Hello after 0 sec"), 0);
setTimeOut(() => log("Hello after 5 sec"), 5000);
log("Hello")
