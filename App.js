/**
 * App.js
 *
 * Prints the current version of Lemon.
 * You can use this file for testing Lemon by running './lemon App.js'
 *
 * 
const fs = internalBinding("fs");

class fileSystem {
  watchFile(file, options = { interval: 500, persistent: true }, listener) {
    let path;
    let optionsObj = options;
    let callback;

    if (typeof file === "string") path = file;
    else throw new Error("first argument must be a path to a string");

    if (typeof optionsObj === "function") {
      callback = optionsObj;
      optionsObj = { interval: 500, persistent: true };
    } else {
      if (listener) callback = listener;
      else
        throw new Error(
          "Too Few Arguments, a callback function must be specified"
        );
    }
    function callbackWrapper() {
      let prev = fs.getStatsSync(path);
      function wrapper() {
        let current = fs.getStatsSync(path);
        callback(prev, current);
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
  const date = new Date(curr.ctimeMs);
  log(date.toISOString());
  fs.readFileAsync("Hello.txt", (err, data) => {
    if (err) {
      throw err;
    }
    log(data);
  });
});
 */
'use strict'

class EventEmitter {
  listener = Object.create(null);

  addListener(eventName, listener) {
    this.listener[eventName] = this.listener[eventName] || [];
    this.listener[eventName].push(listener);
    return this;
  }
  on(eventName, listener) {
    return this.addListener(eventName, listener);
  }

  once(eventName, listener) {
    this.listener[eventName] = this.listener[eventName] || [];
    const onceWrapper = () => {
      listener();
      this.off(eventName, onceWrapper);
    };
    this.listener[eventName].push(onceWrapper);
    return this;
  }

  removeListener(eventName, listener) {
    const events = this.listener[eventName];
    if (events) {
      events.forEach((callback, idx) => {
        if (callback.toString() === listener.toString()) events.splice(idx, 1);
      });
    }
    return this;
  }

  off(eventName, listener) {
    return this.removeListener(eventName, listener);
  }

  emit(eventName, ...args) {
    const events = this.listener[eventName];
    if (events) {
      events.forEach((callback) => {
        callback(...args);
      });
      return true;
    } else return false;
  }

  listenerCount(eventName) {
    return this.listener[eventName]?.length || 0;
  }

  rawListners(eventName) {
    return this.listener[eventName] ? [...this.listener[eventName]] : [];
  }
}

const buffer = internalBinding("Buffer");

class FastBuffer extends Uint8Array {
  constructor(bufferOrLength, offset, length) {
    super(bufferOrLength, offset, length);
  }
}

const zero_fill_toggle = internalBinding("getZeroFillToggle")();

function createUnsafeBuffer(size) {
  zero_fill_toggle[0] = 0;
  try {
    return new FastBuffer(size);
  } finally {
    zero_fill_toggle[0] = 1;
  }
}

let allocPool, poolOffset, poolSize = 8 * 1024;

function createPool() {
  allocPool = createUnsafeBuffer(poolSize).buffer;
  poolOffset = 0;
}

function alignPool() {
  if (poolOffset & 0x7) {
    poolOffset |= 0x7;
    poolOffset++;
  }
}

createPool();

function isAnyArrayBuffer(object) {
  return (
    object instanceof ArrayBuffer ||
    object instanceof SharedArrayBuffer ||
    false
  );
}
const encodingOps = {
  utf8: {
    encoding: "utf8",
    byteLength: buffer.byteLengthUtf8,
    encodingVal: 0,
    write: (buf, string, offset, len) => buffer.utf8Write(buf,string, offset, len),
    slice: (buf, start, end) => buf.utf8Slice(start, end),
  },
  utf16le: {
    encoding: "utf16le",
    encodingVal: 1,
    byteLength: (string) => string.length * 2,
    write: (buf, string, offset, len) => buf.ucs2Write(string, offset, len),
    slice: (buf, start, end) => buf.ucs2Slice(start, end),
  },
};

function getEncodingOps(encoding) {
  encoding += "";
  switch (encoding.length) {
    case 4:
      if (encoding === "utf8") return encodingOps.utf8;
      encoding = StringPrototypeToLowerCase(encoding);
      if (encoding === "utf8") return encodingOps.utf8;
      break;
    case 5:
      if (encoding === "utf-8") return encodingOps.utf8;
      encoding = StringPrototypeToLowerCase(encoding);
      if (encoding === "utf-8") return encodingOps.utf8;
      break;
    case 7:
      if (
        encoding === "utf16le" ||
        StringPrototypeToLowerCase(encoding) === "utf16le"
      )
        return encodingOps.utf16le;
      break;
    case 8:
      if (
        encoding === "utf-16le" ||
        StringPrototypeToLowerCase(encoding) === "utf-16le"
      )
        return encodingOps.utf16le;
      break;
  }
}
function __fromString(value, encoding) {
  let ops;
  if (typeof encoding !== "string" || encoding.length == 0) {
    if (value.length === 0) return FastBuffer();
    ops = encodingOps.utf8;
    encoding = null;
  } else {
    ops = getEncodingOps(encoding);
    if (ops === undefined) throw new Error("Invalid Encoding");
    if (value.length === 0) return FastBuffer();
  }
  return __fromStringFast(value, ops);
}
function __fromStringFast(str, ops) {
  const length = ops.byteLength(str);
  if (length >= poolSize >>> 1) return createFromString(str, ops.encodingVal);
  const poolMaxSize = poolSize - poolOffset;
  if (length > poolMaxSize) createPool();
  log(poolOffset)
  const buffer = new FastBuffer(allocPool, poolOffset, length);
  const bytesWritten = ops.write(buffer, str, 0, length);
  poolOffset += bytesWritten;
  alignPool();  
  return buffer;
}

function __fromArrayBuffer(value, offset, length) {}
function __fromObject(value) {}
class Buffer {
  constructor(arg, encodingOrOffest, length) {
    if (typeof arg === "number") {
      if (typeof encodingOrOffest === "string")
        throw new TypeError(
          "Invalid Argument the second argument has to be offset(number) if the first argument is a number"
        );
      Buffer.alloc(arg);
    }
    Buffer.from(arg, encodingOrOffest, length);
  }
  static from(value, encodingOrOffest, length) {
    if (typeof value === "string") return __fromString(value, encodingOrOffest);

    if (typeof value === "object" && value !== null) {
      if (isAnyArrayBuffer(value))
        __fromArrayBuffer(value, encodingOrOffest, length);

      const valueOf = value.valueOf && value.valueOf();
      if (
        valueOf != null &&
        valueOf !== value &&
        (typeof valueOf === "string" || typeof valueOf === "object")
      ) {
        from(valueOf, encodingOrOffest, length);
      }

      const ret = __fromObject(value);
      if (ret) return ret;

      if (typeof value[Symbol.toPrimitive] === "function") {
        const toPrimitive = value[Symbol.toPrimitive]("string");
        if (typeof toPrimitive === "string")
          from(toPrimitive, encodingOrOffest, length);
      }
    }
    throw new TypeError("Invalid arguments");
  }

  static alloc() {}
}



const buff2 = Buffer.from("Muhammad Medhat", "utf-8");

log(buff2)

const buff1 = Buffer.from("弛", "utf-8");

log(buff1)

const buff3 = new Uint8Array(allocPool, 18, 3);

log(buff3)
