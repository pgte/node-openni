var kinect       = require('./build/Release/openni.node');
var EventEmitter = require('events').EventEmitter;
var inherits     = require('util').inherits;
var assert       = require('assert');


module.exports = function(options) {
  var kContext = new kinect.Context();
  
  return kContext;
};