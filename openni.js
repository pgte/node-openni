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

  kContext.__proto__.__proto__ = EventEmitter.prototype;

  // Set all joints by default and let the user override this if he wills

  kContext.setJoints([
    "head",
    "neck",
    "torso",
    "waist",
    "left_collar",
    "left_shoulder",
    "left_elbow",
    "left_wrist",
    "left_hand",
    "left_fingertip",
    "right_collar",
    "right_shoulder",
    "right_elbow",
    "right_wrist",
    "right_hand",
    "right_fingertip",
    "left_hip",
    "left_knee",
    "left_ankle",
    "left_foot",
    "right_hip",
    "right_knee",
    "right_ankle",
    "right_foot"
  ]);
  
  return kContext;
};