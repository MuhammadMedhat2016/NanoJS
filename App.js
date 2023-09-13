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
"use strict";

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
  toString(encoding) {
    if (encoding == undefined) {
      encoding = "utf-8";
    }
    const ops = getEncodingOps(encoding);
    if (ops === undefined) throw new Error("No such encoding exists");

    return ops.decodeToString(this);
  }
}

FastBuffer.prototype.constructor = Buffer;
Buffer.prototype = FastBuffer.prototype;

const zero_fill_toggle = internalBinding("getZeroFillToggle")();

function createUnsafeBuffer(size) {
  zero_fill_toggle[0] = 0;
  try {
    return new FastBuffer(size);
  } finally {
    zero_fill_toggle[0] = 1;
  }
}

let allocPool,
  poolOffset,
  poolSize = 8 * 1024;

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
    write: (buf, string, offset, len) =>
      buffer.utf8Write(buf, string, offset, len),
    slice: (buf, start, end) => buf.utf8Slice(start, end),
    decodeToString: (buf) =>
      buffer.fromUtf8(buf, buf.byteOffset, buf.byteLength),
  },
  utf16le: {
    encoding: "utf16le",
    encodingVal: 1,
    byteLength: (string) => string.length * 2,
    write: (buf, string, offset, len) =>
      buffer.ucs2Write(buf, string, offset, len),
    slice: (buf, start, end) => buf.ucs2Slice(start, end),
    decodeToString: (buf) =>
      buffer.fromUtf16(buf, buf.byteOffset, buf.byteLength),
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
  if (length >= poolSize >>> 1) {
    const buff = createUnsafeBuffer(length);
    ops.write(buff, str, 0, length);
    return buff;
  }
  const poolMaxSize = poolSize - poolOffset;
  if (length > poolMaxSize) createPool();
  const buff = new FastBuffer(allocPool, poolOffset, length);
  const bytesWritten = ops.write(buff, str, poolOffset, length);
  poolOffset += bytesWritten;
  alignPool();
  return buff;
}

function __fromArrayBuffer(value, offset, length) {
  if (offset == undefined) {
    offset = 0;
  } else {
    offset = +offset;
    if (isNaN(offset)) offset = 0;
  }
  const maxLength = value.byteLength - offset;
  if (maxLength < 0) throw new Error("specified offest is out of boundires");

  if (length == undefined) {
    length = maxLength;
  } else {
    length = +length;
    if (length < 0) length = 0;
    else if (length > maxLength)
      throw new Error("specified length is out of boundires");
  }
  return new FastBuffer(value, offset, length);
}

function __fromObject(value) {
  if (value.length !== undefined || isAnyArrayBuffer(value.buffer)) {
    if (typeof value.length !== "number") return new FastBuffer();
    return __fromArrayLike(value);
  }
}
function __fromArrayLike(array) {
  if (array.length <= 0) return new FastBuffer();
  if (array.length >= poolSize >>> 1) {
    const buffer = createUnsafeBuffer(length);
    for (let i = 0; i < array.length; ++i) buffer[i] = array[i] & 255;
    return buffer;
  }
  if (array.length > poolSize - poolOffset) createPool();
  const buffer = new FastBuffer(allocPool, poolOffset, array.length);
  for (let i = 0; i < array.length; ++i) buffer[i] = array[i] & 255;
  return buffer;
}

function Buffer(arg, encodingOrOffest, length) {
  if (typeof arg === "number") {
    if (typeof encodingOrOffest === "string")
      throw new TypeError(
        "Invalid Argument the second argument has to be offset(number) if the first argument is a number"
      );
    Buffer.alloc(arg);
  }
  Buffer.from(arg, encodingOrOffest, length);
}

Buffer.from = function from(value, encodingOrOffest, length) {
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
      return from(valueOf, encodingOrOffest, length);
    }

    const ret = __fromObject(value);
    if (ret) return ret;

    if (typeof value[Symbol.toPrimitive] === "function") {
      const toPrimitive = value[Symbol.toPrimitive]("string");
      if (typeof toPrimitive === "string")
        from(toPrimitive, encodingOrOffest, length);
    }
  }
};
function __fill(buff, fillValue, encoding) {
  const offset = 0;
  const end = buff.byteLength;

  if (typeof fillValue === "string") {
    const ops = getEncodingOps(encoding);
    if (ops == undefined) throw new Error("Invalid encoding value");
    if (fillValue.length === 0) return FastBuffer(length);

    buffer.fill(buff, fillValue, 0, end, ops.encodingVal);
  } else if (typeof fillValue === "number") {
    buffer.fill(buff, fillValue, 0, end);
  }
}

Buffer.allocUnsafe = function (length) {
  return createUnsafeBuffer(length);
};

Buffer.alloc = function (length, fill, encoding) {
  length = +length;
  if (isNaN(length)) throw new TypeError("Invalid length argument");
  if (length > 0 && fill != undefined && fill !== 0) {
    const buffer = createUnsafeBuffer(length);
    __fill(buffer, fill, encoding);
    return buffer;
  }
  return new FastBuffer(length);
};
const fs = internalBinding("fs");
class Files {
  static read(path, options, callback) {
    let callbackFunc = null;
    let opts = null;
    if (typeof path !== "string") throw new TypeError("Invalid Path String");
    if (typeof options === "function") {
      callbackFunc = options;
      opts = {
        buffer: Buffer.alloc(16384),
        offset: 0,
        length: 16384,
        position: 0,
      };
    } else {
      if (callback == undefined)
        throw new Error("a callback function must be provided");
      callbackFunc = callback;
      opts = options;
      if (opts.buffer == undefined) opts.buffer = Buffer.alloc(16384);
      if (opts.offset == undefined) opts.offset = 0;
      if (opts.length == undefined) opts.length = 16384;
      if (opts.position == undefined) opts.position = 0;
    }
    fs.readFileAsync(path, opts, callbackFunc);
  }
}
const options = {
  buffer : Buffer.alloc(10),
  length : 1,
  offset : 0,
  position : 5,
};
Files.read("Hello.txt", options, (error, buffer, bytesRead) => {
  if (error) throw new Error("invalid operation");
  log(buffer.toString());
  log(bytesRead);
});
