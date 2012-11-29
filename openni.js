var kinect       = require('./build/Release/openni.node');
var EventEmitter = require('events').EventEmitter;

module.exports = function(options) {
  var kContext = new kinect.Context();

  var keepRunning = setInterval(function() {}, 3600000);
  var running = true;

  var oldClose = kContext.close;
  kContext.close = function() {
    console.log('\nClosing...\n');
    clearInterval(keepRunning);
    oldClose.apply(kContext, arguments);
  }

  kContext.__proto__ = EventEmitter.prototype;
  
  return kContext;
};