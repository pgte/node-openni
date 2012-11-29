var kinect       = require('./build/Release/openni.node');
var EventEmitter = require('events').EventEmitter;
var inherits     = require('util').inherits;
var assert       = require('assert');


module.exports = function(options) {
  var kContext = new kinect.Context();

  var keepRunning = setInterval(function() {}, 3600000);
  var running = true;

  var oldClose = kContext.close;
  kContext.close = function() {
    console.log('Closing...');
    clearInterval(keepRunning);
    oldClose.apply(kContext, arguments);
  }
  
  return kContext;
};