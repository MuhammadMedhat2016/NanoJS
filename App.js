/**
 * App.js
 *
 * Prints the current version of Lemon.
 * You can use this file for testing Lemon by running './lemon App.js'
 *
 */

const fs = internalBinding("fs");

class fileSystem {
  watchFile(file, options = { interval: 500, persistent: true }, listener) {
    let path;
    let optionsObj = options;
    let callback;
    
    if (typeof file === "string") path = file;
    
    else throw new Error("first argument must be a path to a string");

    if(typeof optionsObj === "function")
    {
        callback = optionsObj;
        optionsObj = { interval: 500, persistent: true };
    }
    else 
    {
        if(listener)
            callback = listener;
        else
            throw new Error("Too Few Arguments, a callback function must be specified");
    }
    function callbackWrapper()
    {
        let prev = fs.getStatsSync(path);
        function wrapper()
        {
            let current = fs.getStatsSync(path);
            callback(prev,current);
            prev = current;
        }
        return wrapper;
    }
    
    let wrapper = callbackWrapper();
    fs.watch(path, optionsObj, wrapper);

  }
}

let file = new fileSystem();

file.watchFile("Hello.txt", (prev, curr) => {
    log(prev);
    log(curr);
})

log("Hello");
